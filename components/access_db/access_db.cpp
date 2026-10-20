#include "access_db.h"

#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"

#include <ArduinoJson.h>
#include <esp_littlefs.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <memory>
#include <vector>

namespace esphome {
namespace access_db {

static const char *const TAG = "access_db";
static constexpr uint32_t ACCESS_DB_MAGIC = 0x42444341;  // 'ACDB' little-endian
static constexpr size_t STREAM_CHUNK_SIZE = 4096;

AccessDb *global_access_db = nullptr;  // NOLINT

struct __attribute__((packed)) AccessDbFileHeader {
  uint32_t magic;
  uint32_t version;
  uint32_t entry_count;
  uint32_t index_offset;
};

struct __attribute__((packed)) IndexEntry {
  uint32_t rfid;
  uint32_t record_offset;
};

static bool write_u16(FILE *f, uint16_t value) { return fwrite(&value, sizeof(value), 1, f) == 1; }

static bool write_string(FILE *f, const std::string &value) {
  if (value.size() > UINT16_MAX) {
    return false;
  }
  if (!write_u16(f, static_cast<uint16_t>(value.size()))) {
    return false;
  }
  return value.empty() || fwrite(value.data(), 1, value.size(), f) == value.size();
}

static bool read_u16(FILE *f, uint16_t &value) { return fread(&value, sizeof(value), 1, f) == 1; }

static bool read_string(FILE *f, std::string &value) {
  uint16_t len = 0;
  if (!read_u16(f, len)) {
    return false;
  }
  value.resize(len);
  if (len == 0) {
    return true;
  }
  return fread(&value[0], 1, len, f) == len;
}

float AccessDb::get_setup_priority() const { return setup_priority::WIFI - 1.0f; }

void AccessDb::setup() {
  global_access_db = this;
  this->db_path_ = this->mount_path_ + "/access.db";

  if (!this->mount_filesystem_()) {
    this->mark_failed();
    return;
  }

  if (this->load_header_()) {
    this->publish_healthy_(true);
    this->publish_status_("ready");
    if (this->version_sensor_ != nullptr) {
      this->version_sensor_->publish_state(this->header_.version);
    }
    if (this->entry_count_sensor_ != nullptr) {
      this->entry_count_sensor_->publish_state(this->header_.entry_count);
    }
  } else {
    this->publish_healthy_(false);
    this->publish_status_("idle");
  }

  auto *base = web_server_base::global_web_server_base;
  if (base == nullptr) {
    ESP_LOGE(TAG, "WebServerBase not found");
    this->mark_failed();
    return;
  }

  base->add_handler_without_auth(this);

  httpd_uri_t put_uri = {
      .uri = this->path_.c_str(),
      .method = HTTP_PUT,
      .handler = [](httpd_req_t *req) -> esp_err_t {
        if (global_access_db == nullptr) {
          httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, nullptr);
          return ESP_FAIL;
        }
        global_access_db->handle_raw_request_(req);
        return ESP_OK;
      },
      .user_ctx = nullptr,
  };

  auto *server = base->get_server();
  if (server != nullptr && server->get_server() != nullptr) {
    httpd_register_uri_handler(server->get_server(), &put_uri);
  }
}

void AccessDb::dump_config() {
  ESP_LOGCONFIG(TAG, "Access DB:");
  ESP_LOGCONFIG(TAG, "  Path: %s", this->path_.c_str());
  ESP_LOGCONFIG(TAG, "  Mount: %s", this->mount_path_.c_str());
  ESP_LOGCONFIG(TAG, "  Partition: %s", this->partition_label_.c_str());
  ESP_LOGCONFIG(TAG, "  Max body: %u bytes", static_cast<unsigned>(this->max_body_size_));
}

bool AccessDb::mount_filesystem_() {
  esp_vfs_littlefs_conf_t conf = {
      .base_path = this->mount_path_.c_str(),
      .partition_label = this->partition_label_.c_str(),
      .format_if_mount_failed = true,
      .dont_mount = false,
  };
  const esp_err_t ret = esp_vfs_littlefs_register(&conf);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "LittleFS mount failed: %s", esp_err_to_name(ret));
    this->fs_mounted_ = false;
    return false;
  }
  this->fs_mounted_ = true;
  return true;
}

bool AccessDb::canHandle(web_server_idf::AsyncWebServerRequest *request) const {
  if (request->method() != HTTP_POST) {
    return false;
  }
  char url_buf[web_server_idf::AsyncWebServerRequest::URL_BUF_SIZE];
  return request->url_to(url_buf) == this->path_;
}

void AccessDb::handleRequest(web_server_idf::AsyncWebServerRequest *request) { this->handle_raw_request_(*request); }

void AccessDb::handle_raw_request_(httpd_req_t *req) {
  if (this->auth_ == nullptr || !this->auth_->validate_raw(req)) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"invalid_token\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  if (this->busy_) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"busy\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  const size_t content_len = req->content_len;
  if (content_len == 0 || content_len > this->max_body_size_) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"payload_too_large\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  this->busy_ = true;
  this->publish_status_("receiving");

  const std::string json_tmp = this->mount_path_ + "/upload.json.tmp";
  const std::string db_tmp = this->mount_path_ + "/access.db.tmp";

  if (!this->stream_body_to_file_(req, content_len, json_tmp)) {
    this->busy_ = false;
    this->publish_status_("error", "stream_failed");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"stream_failed\"}", HTTPD_RESP_USE_STRLEN);
    remove(json_tmp.c_str());
    return;
  }

  this->publish_status_("building");
  std::string response_json;
  if (!this->build_index_from_json_file_(json_tmp, response_json)) {
    this->busy_ = false;
    remove(json_tmp.c_str());
    remove(db_tmp.c_str());
    this->publish_status_("error", "parse_failed");
    this->publish_healthy_(this->index_loaded_);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"parse_failed\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  remove(json_tmp.c_str());
  remove(this->db_path_.c_str());
  if (rename(db_tmp.c_str(), this->db_path_.c_str()) != 0) {
    this->busy_ = false;
    this->publish_status_("error", "rename_failed");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"rename_failed\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  this->index_loaded_ = this->load_header_();
  this->busy_ = false;
  this->publish_healthy_(this->index_loaded_);
  this->publish_sync_time_();
  this->publish_status_("ready");

  if (this->version_sensor_ != nullptr) {
    this->version_sensor_->publish_state(this->header_.version);
  }
  if (this->entry_count_sensor_ != nullptr) {
    this->entry_count_sensor_->publish_state(this->header_.entry_count);
  }

  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, response_json.c_str(), HTTPD_RESP_USE_STRLEN);
}

bool AccessDb::stream_body_to_file_(httpd_req_t *req, size_t content_len, const std::string &dest_path) {
  FILE *file = fopen(dest_path.c_str(), "wb");
  if (file == nullptr) {
    ESP_LOGE(TAG, "Failed to open %s for writing", dest_path.c_str());
    return false;
  }

  size_t remaining = content_len;
  size_t bytes_since_yield = 0;
  auto buffer = std::make_unique<char[]>(STREAM_CHUNK_SIZE);

  while (remaining > 0) {
    const size_t to_read = std::min(remaining, STREAM_CHUNK_SIZE);
    const int recv_len = httpd_req_recv(req, buffer.get(), to_read);
    if (recv_len <= 0) {
      fclose(file);
      remove(dest_path.c_str());
      return false;
    }
    if (fwrite(buffer.get(), 1, recv_len, file) != static_cast<size_t>(recv_len)) {
      fclose(file);
      remove(dest_path.c_str());
      return false;
    }
    remaining -= recv_len;
    bytes_since_yield += recv_len;
    if (bytes_since_yield >= 16384) {
      vTaskDelay(1);
      bytes_since_yield = 0;
    }
  }

  fclose(file);
  return true;
}

bool AccessDb::build_index_from_json_file_(const std::string &json_path, std::string &response_json) {
  FILE *json_file = fopen(json_path.c_str(), "rb");
  if (json_file == nullptr) {
    return false;
  }

  fseek(json_file, 0, SEEK_END);
  const long file_size = ftell(json_file);
  fseek(json_file, 0, SEEK_SET);
  if (file_size <= 0 || static_cast<size_t>(file_size) > this->max_body_size_) {
    fclose(json_file);
    return false;
  }

  std::string json(static_cast<size_t>(file_size), '\0');
  if (fread(&json[0], 1, file_size, json_file) != static_cast<size_t>(file_size)) {
    fclose(json_file);
    return false;
  }
  fclose(json_file);

  JsonDocument doc;
  const DeserializationError err = deserializeJson(doc, json);
  json.clear();
  json.shrink_to_fit();
  if (err) {
    ESP_LOGE(TAG, "JSON parse failed: %s", err.c_str());
    return false;
  }

  if (!doc["version"].is<uint32_t>() && !doc["version"].is<int>()) {
    ESP_LOGE(TAG, "Missing version field");
    return false;
  }

  const uint32_t version = doc["version"].as<uint32_t>();
  JsonObject cards = doc["cards"].as<JsonObject>();
  if (cards.isNull()) {
    ESP_LOGE(TAG, "Missing cards object");
    return false;
  }

  struct PendingEntry {
    uint32_t rfid;
    std::string uid;
    std::string name;
    std::string nickname;
  };

  std::vector<PendingEntry> entries;
  entries.reserve(cards.size());

  for (JsonPair kv : cards) {
    PendingEntry entry;
    entry.rfid = parse_rfid_key_(kv.key().c_str());
    JsonObject card = kv.value().as<JsonObject>();
    if (card.isNull()) {
      continue;
    }
    if (card["uid"].is<const char *>()) {
      entry.uid = card["uid"].as<const char *>();
    }
    if (card["name"].is<const char *>()) {
      entry.name = card["name"].as<const char *>();
    }
    if (card["nickname"].is<const char *>()) {
      entry.nickname = card["nickname"].as<const char *>();
    }
    entries.push_back(std::move(entry));
  }

  std::sort(entries.begin(), entries.end(),
            [](const PendingEntry &a, const PendingEntry &b) { return a.rfid < b.rfid; });

  const std::string db_tmp = this->mount_path_ + "/access.db.tmp";
  remove(db_tmp.c_str());

  FILE *db_file = fopen(db_tmp.c_str(), "wb");
  if (db_file == nullptr) {
    return false;
  }

  if (fseek(db_file, sizeof(AccessDbFileHeader), SEEK_SET) != 0) {
    fclose(db_file);
    return false;
  }

  std::vector<IndexEntry> index;
  index.reserve(entries.size());

  for (const auto &entry : entries) {
    IndexEntry idx{};
    idx.rfid = entry.rfid;
    idx.record_offset = static_cast<uint32_t>(ftell(db_file));
    index.push_back(idx);

    if (!write_string(db_file, entry.uid) || !write_string(db_file, entry.name) ||
        !write_string(db_file, entry.nickname)) {
      fclose(db_file);
      return false;
    }
  }

  AccessDbFileHeader header{};
  header.magic = ACCESS_DB_MAGIC;
  header.version = version;
  header.entry_count = static_cast<uint32_t>(entries.size());
  header.index_offset = static_cast<uint32_t>(ftell(db_file));

  for (const auto &idx : index) {
    if (fwrite(&idx, sizeof(idx), 1, db_file) != 1) {
      fclose(db_file);
      return false;
    }
  }

  if (fseek(db_file, 0, SEEK_SET) != 0 || fwrite(&header, sizeof(header), 1, db_file) != 1) {
    fclose(db_file);
    return false;
  }

  fclose(db_file);

  char buf[96];
  snprintf(buf, sizeof(buf), "{\"ok\":true,\"version\":%u,\"entries\":%u}", version,
           static_cast<unsigned>(entries.size()));
  response_json = buf;
  return true;
}

bool AccessDb::load_header_() {
  FILE *file = fopen(this->db_path_.c_str(), "rb");
  if (file == nullptr) {
    this->index_loaded_ = false;
    return false;
  }

  AccessDbFileHeader header{};
  if (fread(&header, sizeof(header), 1, file) != 1) {
    fclose(file);
    this->index_loaded_ = false;
    return false;
  }
  fclose(file);

  if (header.magic != ACCESS_DB_MAGIC) {
    this->index_loaded_ = false;
    return false;
  }

  this->header_.magic = header.magic;
  this->header_.version = header.version;
  this->header_.entry_count = header.entry_count;
  this->header_.index_offset = header.index_offset;
  this->index_loaded_ = true;
  return true;
}

uint32_t AccessDb::parse_rfid_key_(const std::string &rfid_code) {
  char *end = nullptr;
  const unsigned long value = strtoul(rfid_code.c_str(), &end, 10);
  if (end == rfid_code.c_str()) {
    return 0;
  }
  return static_cast<uint32_t>(value);
}

AccessDbLookupResult AccessDb::lookup(const std::string &rfid_code) {
  if (!this->index_loaded_) {
    return {};
  }
  AccessDbLookupResult result = this->lookup_loaded_(parse_rfid_key_(rfid_code));
  if (result.found) {
    this->last_match_ = result;
  }
  return result;
}

AccessDbLookupResult AccessDb::lookup_by_uid(const std::string &uid) {
  AccessDbLookupResult result;
  if (!this->index_loaded_ || uid.empty()) {
    return result;
  }

  FILE *file = fopen(this->db_path_.c_str(), "rb");
  if (file == nullptr) {
    return result;
  }

  for (uint32_t i = 0; i < this->header_.entry_count; i++) {
    const long index_offset = static_cast<long>(this->header_.index_offset + i * sizeof(IndexEntry));
    if (fseek(file, index_offset, SEEK_SET) != 0) {
      continue;
    }

    IndexEntry entry{};
    if (fread(&entry, sizeof(entry), 1, file) != 1) {
      continue;
    }

    AccessDbLookupResult candidate;
    if (!this->read_record_at_(file, entry.record_offset, candidate)) {
      continue;
    }

    if (candidate.uid == uid) {
      candidate.found = true;
      this->last_match_ = candidate;
      fclose(file);
      return candidate;
    }
  }

  fclose(file);
  return result;
}

bool AccessDb::read_record_at_(FILE *file, uint32_t record_offset, AccessDbLookupResult &result) const {
  if (fseek(file, static_cast<long>(record_offset), SEEK_SET) != 0) {
    return false;
  }
  return read_string(file, result.uid) && read_string(file, result.name) && read_string(file, result.nickname);
}

AccessDbLookupResult AccessDb::lookup_loaded_(uint32_t rfid) const {
  AccessDbLookupResult result;

  FILE *file = fopen(this->db_path_.c_str(), "rb");
  if (file == nullptr) {
    return result;
  }

  if (this->header_.entry_count == 0) {
    fclose(file);
    return result;
  }

  size_t left = 0;
  size_t right = this->header_.entry_count - 1;

  while (left <= right) {
    const size_t mid = left + ((right - left) / 2);
    const long offset = static_cast<long>(this->header_.index_offset + mid * sizeof(IndexEntry));

    if (fseek(file, offset, SEEK_SET) != 0) {
      fclose(file);
      return result;
    }

    IndexEntry entry{};
    if (fread(&entry, sizeof(entry), 1, file) != 1) {
      fclose(file);
      return result;
    }

    if (entry.rfid == rfid) {
      if (!this->read_record_at_(file, entry.record_offset, result)) {
        fclose(file);
        return {};
      }
      result.found = true;
      fclose(file);
      return result;
    }

    if (entry.rfid < rfid) {
      left = mid + 1;
    } else {
      if (mid == 0) {
        break;
      }
      right = mid - 1;
    }
  }

  fclose(file);
  return result;
}

void AccessDb::publish_status_(const std::string &status, const std::string &error) {
  if (this->status_text_sensor_ != nullptr) {
    this->status_text_sensor_->publish_state(status);
  }
  if (this->last_error_text_sensor_ != nullptr) {
    this->last_error_text_sensor_->publish_state(error);
  }
}

void AccessDb::publish_healthy_(bool healthy) {
  if (this->healthy_binary_sensor_ != nullptr) {
    this->healthy_binary_sensor_->publish_state(healthy);
  }
}

void AccessDb::publish_sync_time_() {
  if (this->last_sync_text_sensor_ == nullptr) {
    return;
  }
  char buf[32];
  time_t now = time(nullptr);
  struct tm timeinfo {};
  if (localtime_r(&now, &timeinfo) != nullptr) {
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeinfo);
    this->last_sync_text_sensor_->publish_state(buf);
  }
}

}  // namespace access_db
}  // namespace esphome
