#include "push_auth.h"
#include "esphome/core/log.h"

#include <ctime>

namespace esphome {
namespace push_auth {

static const char *const TAG = "push_auth";

bool PushAuth::secure_equals_(const std::string &a, const std::string &b) {
  if (a.size() != b.size()) {
    return false;
  }
  uint8_t result = 0;
  for (size_t i = 0; i < a.size(); i++) {
    result |= static_cast<uint8_t>(a[i] ^ b[i]);
  }
  return result == 0;
}

std::string PushAuth::extract_bearer_(const std::string &value) {
  const std::string prefix = "Bearer ";
  if (value.rfind(prefix, 0) == 0) {
    return value.substr(prefix.size());
  }
  return value;
}

bool PushAuth::validate_token(const std::string &credential) {
  if (this->token_.empty() || credential.empty()) {
    this->record_attempt_("missing_token");
    return false;
  }
  const std::string token = extract_bearer_(credential);
  if (secure_equals_(token, this->token_)) {
    this->record_attempt_("ok");
    return true;
  }
  this->record_attempt_("invalid_token");
  this->failure_count_++;
  if (this->failure_count_sensor_ != nullptr) {
    this->failure_count_sensor_->publish_state(this->failure_count_);
  }
  return false;
}

#ifdef USE_ESP32
bool PushAuth::validate_request(web_server_idf::AsyncWebServerRequest *request) {
  if (request->hasHeader(this->header_.c_str())) {
    return this->validate_token(request->get_header(this->header_.c_str()).value_or(""));
  }
  if (!this->alt_header_.empty() && request->hasHeader(this->alt_header_.c_str())) {
    return this->validate_token(request->get_header(this->alt_header_.c_str()).value_or(""));
  }
  return this->validate_token("");
}

bool PushAuth::validate_raw(httpd_req_t *req) {
  auto read_header = [&](const std::string &name) -> std::string {
    const size_t len = httpd_req_get_hdr_value_len(req, name.c_str());
    if (len == 0) {
      return "";
    }
    std::string value(len + 1, '\0');
    if (httpd_req_get_hdr_value_str(req, name.c_str(), value.data(), value.size()) != ESP_OK) {
      return "";
    }
    value.resize(len);
    return value;
  };

  std::string credential = read_header(this->header_);
  if (credential.empty() && !this->alt_header_.empty()) {
    credential = read_header(this->alt_header_);
  }
  return this->validate_token(credential);
}

#endif

void PushAuth::record_attempt_(const std::string &result) {
  if (this->last_result_sensor_ != nullptr) {
    this->last_result_sensor_->publish_state(result);
  }
  if (this->last_attempt_sensor_ != nullptr) {
    char buf[32];
    time_t now = time(nullptr);
    struct tm timeinfo {};
    if (localtime_r(&now, &timeinfo) != nullptr) {
      strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeinfo);
      this->last_attempt_sensor_->publish_state(buf);
    } else {
      this->last_attempt_sensor_->publish_state("unknown");
    }
  }
}

void PushAuth::dump_config() {
  ESP_LOGCONFIG(TAG, "Push Auth:");
  ESP_LOGCONFIG(TAG, "  Header: %s", this->header_.c_str());
  if (!this->alt_header_.empty()) {
    ESP_LOGCONFIG(TAG, "  Alt Header: %s", this->alt_header_.c_str());
  }
  ESP_LOGCONFIG(TAG, "  Token: [set]");
}

}  // namespace push_auth
}  // namespace esphome
