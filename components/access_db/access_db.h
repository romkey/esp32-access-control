#pragma once

#include "esphome/core/component.h"
#include "esphome/components/push_auth/push_auth.h"
#include "esphome/components/web_server_idf/web_server_idf.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include <esp_http_server.h>

#include <cstdint>
#include <cstdio>
#include <string>

namespace esphome {
namespace access_db {

struct AccessDbLookupResult {
  bool found{false};
  std::string uid;
  std::string name;
  std::string nickname;
};

class AccessDb : public web_server_idf::AsyncWebHandler, public Component {
 public:
  AccessDb() = default;

  void set_auth(push_auth::PushAuth *auth) { this->auth_ = auth; }
  void set_path(const std::string &path) { this->path_ = path; }
  void set_max_body_size(size_t size) { this->max_body_size_ = size; }
  void set_partition_label(const std::string &label) { this->partition_label_ = label; }
  void set_mount_path(const std::string &path) { this->mount_path_ = path; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;

  AccessDbLookupResult lookup(const std::string &rfid_code);
  AccessDbLookupResult lookup_by_uid(const std::string &uid);
  const AccessDbLookupResult &get_last_match() const { return this->last_match_; }

  void set_version_sensor(sensor::Sensor *sensor) { this->version_sensor_ = sensor; }
  void set_entry_count_sensor(sensor::Sensor *sensor) { this->entry_count_sensor_ = sensor; }
  void set_status_text_sensor(text_sensor::TextSensor *sensor) { this->status_text_sensor_ = sensor; }
  void set_last_sync_text_sensor(text_sensor::TextSensor *sensor) { this->last_sync_text_sensor_ = sensor; }
  void set_last_error_text_sensor(text_sensor::TextSensor *sensor) { this->last_error_text_sensor_ = sensor; }
  void set_healthy_binary_sensor(binary_sensor::BinarySensor *sensor) { this->healthy_binary_sensor_ = sensor; }

  bool canHandle(web_server_idf::AsyncWebServerRequest *request) const override;
  void handleRequest(web_server_idf::AsyncWebServerRequest *request) override;
  void handleBody(web_server_idf::AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index,
                  size_t total) override;
  void handle_raw_request_(httpd_req_t *req);
  bool isRequestHandlerTrivial() const override { return false; }

 protected:
  bool mount_filesystem_();
  void publish_status_(const std::string &status, const std::string &error = "");
  void publish_healthy_(bool healthy);
  void publish_sync_time_();
  bool build_index_from_json_file_(const std::string &json_path, std::string &response_json);
  bool load_header_();
  AccessDbLookupResult lookup_loaded_(uint32_t rfid) const;
  bool read_record_at_(FILE *file, uint32_t record_offset, AccessDbLookupResult &result) const;
  static uint32_t parse_rfid_key_(const std::string &rfid_code);

  push_auth::PushAuth *auth_{nullptr};
  std::string path_{"/access-db"};
  std::string mount_path_{"/storage"};
  std::string partition_label_{"storage"};
  size_t max_body_size_{131072};
  bool fs_mounted_{false};
  bool busy_{false};
  enum class BodyState { NONE, RECEIVING, BAD_TOKEN, BUSY, BAD_SIZE, STREAM_FAILED };
  BodyState body_state_{BodyState::NONE};
  FILE *body_file_{nullptr};

  struct LoadedHeader {
    uint32_t magic{0};
    uint32_t version{0};
    uint32_t entry_count{0};
    uint32_t index_offset{0};
  };
  LoadedHeader header_{};
  bool index_loaded_{false};
  std::string db_path_;
  AccessDbLookupResult last_match_{};

  sensor::Sensor *version_sensor_{nullptr};
  sensor::Sensor *entry_count_sensor_{nullptr};
  text_sensor::TextSensor *status_text_sensor_{nullptr};
  text_sensor::TextSensor *last_sync_text_sensor_{nullptr};
  text_sensor::TextSensor *last_error_text_sensor_{nullptr};
  binary_sensor::BinarySensor *healthy_binary_sensor_{nullptr};
};

extern AccessDb *global_access_db;  // NOLINT

}  // namespace access_db
}  // namespace esphome
