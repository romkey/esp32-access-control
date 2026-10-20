#include "access_syslog.h"

#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/core/log.h"

#include <ArduinoJson.h>
#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

namespace esphome {
namespace access_syslog {

static const char *const TAG = "access_syslog";
static constexpr size_t UNLOCK_BODY_MAX = 512;

AccessSyslog *global_access_syslog = nullptr;  // NOLINT

float AccessSyslog::get_setup_priority() const { return setup_priority::WIFI - 1.0f; }

void AccessSyslog::setup() {
  global_access_syslog = this;
  this->ensure_socket_();

  if (this->unlock_path_.empty() || this->auth_ == nullptr) {
    return;
  }

  auto *base = web_server_base::global_web_server_base;
  if (base == nullptr) {
    ESP_LOGW(TAG, "WebServerBase not found; unlock endpoint disabled");
    return;
  }

  base->add_handler_without_auth(this);

  httpd_uri_t put_uri = {
      .uri = this->unlock_path_.c_str(),
      .method = HTTP_PUT,
      .handler = [](httpd_req_t *req) -> esp_err_t {
        if (global_access_syslog == nullptr) {
          httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, nullptr);
          return ESP_FAIL;
        }
        global_access_syslog->handle_raw_request_(req);
        return ESP_OK;
      },
      .user_ctx = nullptr,
  };

  auto *server = base->get_server();
  if (server != nullptr && server->get_server() != nullptr) {
    httpd_register_uri_handler(server->get_server(), &put_uri);
  }
}

void AccessSyslog::dump_config() {
  ESP_LOGCONFIG(TAG, "Access Syslog:");
  ESP_LOGCONFIG(TAG, "  Server: %s:%u", this->server_.c_str(), this->port_);
  ESP_LOGCONFIG(TAG, "  Hostname: %s", this->hostname_.c_str());
  ESP_LOGCONFIG(TAG, "  Facility: %u", this->facility_);
  if (this->site_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Site: %s", this->site_->get_name().c_str());
  }
  if (!this->unlock_path_.empty()) {
    ESP_LOGCONFIG(TAG, "  Unlock path: %s", this->unlock_path_.c_str());
  }
}

std::string AccessSyslog::site_label_() const {
  if (this->site_ != nullptr && !this->site_->get_name().empty()) {
    return this->site_->get_name();
  }
  return this->hostname_;
}

std::string AccessSyslog::resolve_person_name_(const access_db::AccessDbLookupResult *match) const {
  if (match != nullptr && match->found && !match->name.empty()) {
    return match->name;
  }
  if (this->access_db_ != nullptr) {
    const auto &last = this->access_db_->get_last_match();
    if (last.found && !last.name.empty()) {
      return last.name;
    }
  }
  return "unknown";
}

bool AccessSyslog::ensure_socket_() {
  if (this->socket_ >= 0) {
    return true;
  }

  this->socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (this->socket_ < 0) {
    ESP_LOGE(TAG, "Failed to create syslog socket: errno %d", errno);
    return false;
  }

  struct timeval timeout {};
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;
  setsockopt(this->socket_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

  return true;
}

void AccessSyslog::close_socket_() {
  if (this->socket_ >= 0) {
    close(this->socket_);
    this->socket_ = -1;
  }
}

void AccessSyslog::send_message(const std::string &message) {
  if (message.empty()) {
    return;
  }

  if (!this->ensure_socket_()) {
    return;
  }

  struct sockaddr_in dest {};
  dest.sin_family = AF_INET;
  dest.sin_port = htons(this->port_);
  if (inet_pton(AF_INET, this->server_.c_str(), &dest.sin_addr) != 1) {
    ESP_LOGE(TAG, "Invalid syslog server address: %s", this->server_.c_str());
    return;
  }

  char timestamp[20];
  struct timeval tv {};
  gettimeofday(&tv, nullptr);
  struct tm timeinfo {};
  localtime_r(&tv.tv_sec, &timeinfo);
  strftime(timestamp, sizeof(timestamp), "%b %e %H:%M:%S", &timeinfo);

  const int pri = this->facility_ * 8 + 6;
  char packet[512];
  const int len = snprintf(packet, sizeof(packet), "<%d>%s %s access-control: %s", pri, timestamp,
                           this->hostname_.c_str(), message.c_str());
  if (len <= 0 || len >= static_cast<int>(sizeof(packet))) {
    ESP_LOGW(TAG, "Syslog message too long");
    return;
  }

  const ssize_t sent = sendto(this->socket_, packet, len, 0, reinterpret_cast<struct sockaddr *>(&dest), sizeof(dest));
  if (sent < 0) {
    ESP_LOGW(TAG, "Syslog send failed: errno %d", errno);
    this->close_socket_();
    return;
  }

  ESP_LOGD(TAG, "Syslog: %s", message.c_str());
}

void AccessSyslog::send_door_opened_for_person_(const std::string &person_name) {
  const std::string site = this->site_label_();
  this->send_message(person_name + " has opened " + site + " door");
}

void AccessSyslog::send_unlocked_by_person_(const std::string &person_name) {
  const std::string site = this->site_label_();
  this->send_message(site + " door unlocked by " + person_name);
}

void AccessSyslog::send_online() { this->send_message(this->site_label_() + " access control is online"); }

void AccessSyslog::send_door_opened() { this->send_door_opened_for_person_(this->resolve_person_name_()); }

void AccessSyslog::send_unlocked_by() { this->send_unlocked_by_person_(this->resolve_person_name_()); }

void AccessSyslog::send_unlocked_by_uid(const std::string &uid) {
  if (this->access_db_ == nullptr) {
    this->send_unlocked_by_person_("unknown");
    return;
  }
  const auto match = this->access_db_->lookup_by_uid(uid);
  this->send_unlocked_by_person_(this->resolve_person_name_(&match));
}

void AccessSyslog::send_access_denied(const std::string &detail) {
  const std::string site = this->site_label_();
  if (detail.empty()) {
    this->send_message("access denied at " + site);
  } else {
    this->send_message("access denied at " + site + " (" + detail + ")");
  }
}

bool AccessSyslog::canHandle(web_server_idf::AsyncWebServerRequest *request) const {
  if (this->unlock_path_.empty() || this->auth_ == nullptr) {
    return false;
  }
  if (request->method() != HTTP_POST) {
    return false;
  }
  char url_buf[web_server_idf::AsyncWebServerRequest::URL_BUF_SIZE];
  return request->url_to(url_buf) == this->unlock_path_;
}

// ESPHome's web server reads a POST body that isn't a form before it calls
// handleRequest() and hands it over in chunks here (see AccessDb::handleBody).
// Clients must send a non-form Content-Type such as application/json.
void AccessSyslog::handleBody(web_server_idf::AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index,
                              size_t total) {
  if (index == 0) {
    this->body_.clear();
    this->body_ok_ = total > 0 && total <= UNLOCK_BODY_MAX;
    if (this->body_ok_) {
      this->body_.reserve(total);
    }
  }
  if (this->body_ok_) {
    this->body_.append(reinterpret_cast<const char *>(data), len);
  }
}

void AccessSyslog::handleRequest(web_server_idf::AsyncWebServerRequest *request) {
  this->handle_raw_request_(*request);
}

void AccessSyslog::handle_raw_request_(httpd_req_t *req) {
  if (this->auth_ == nullptr || !this->auth_->validate_raw(req)) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"invalid_token\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  if (this->access_db_ == nullptr) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"access_db_unavailable\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  std::string body;
  body.swap(this->body_);
  const bool body_ok = this->body_ok_;
  this->body_ok_ = false;
  if (!body_ok || body.empty()) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"invalid_body\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, body) != DeserializationError::Ok) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"parse_failed\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  const char *uid = doc["uid"].as<const char *>();
  if (uid == nullptr || uid[0] == '\0') {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"missing_uid\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  const auto match = this->access_db_->lookup_by_uid(uid);
  if (!match.found) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"unknown_uid\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  this->send_unlocked_by_person_(match.name);

  char response[160];
  snprintf(response, sizeof(response), "{\"ok\":true,\"uid\":\"%s\",\"name\":\"%s\"}", match.uid.c_str(),
           match.name.c_str());
  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

}  // namespace access_syslog
}  // namespace esphome
