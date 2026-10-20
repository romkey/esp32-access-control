#include "access_site.h"
#include "esphome/core/log.h"

namespace esphome {
namespace access_site {

static const char *const TAG = "access_site";

void AccessSite::setup() {
  if (this->name_sensor_ != nullptr) {
    this->name_sensor_->publish_state(this->name_);
  }
}

void AccessSite::dump_config() {
  ESP_LOGCONFIG(TAG, "Access Site:");
  ESP_LOGCONFIG(TAG, "  Name: %s", this->name_.c_str());
}

}  // namespace access_site
}  // namespace esphome
