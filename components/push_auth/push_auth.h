#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/sensor/sensor.h"

#ifdef USE_ESP32
#include "esphome/components/web_server_idf/web_server_idf.h"
#include <esp_http_server.h>
#endif

#include <string>

namespace esphome {
namespace push_auth {

class PushAuth : public Component {
 public:
  void set_token(const std::string &token) { this->token_ = token; }
  void set_header(const std::string &header) { this->header_ = header; }
  void set_alt_header(const std::string &alt_header) { this->alt_header_ = alt_header; }

#ifdef USE_ESP32
  bool validate_request(web_server_idf::AsyncWebServerRequest *request);
  bool validate_raw(httpd_req_t *req);
#endif
  bool validate_token(const std::string &credential);

  void set_last_result_sensor(text_sensor::TextSensor *sensor) { this->last_result_sensor_ = sensor; }
  void set_last_attempt_sensor(text_sensor::TextSensor *sensor) { this->last_attempt_sensor_ = sensor; }
  void set_failure_count_sensor(sensor::Sensor *sensor) { this->failure_count_sensor_ = sensor; }

  void dump_config() override;

 protected:
  void record_attempt_(const std::string &result);
  static bool secure_equals_(const std::string &a, const std::string &b);
  static std::string extract_bearer_(const std::string &value);
  std::string token_;
  std::string header_{"Authorization"};
  std::string alt_header_;
  text_sensor::TextSensor *last_result_sensor_{nullptr};
  text_sensor::TextSensor *last_attempt_sensor_{nullptr};
  sensor::Sensor *failure_count_sensor_{nullptr};
  uint32_t failure_count_{0};
};

}  // namespace push_auth
}  // namespace esphome
