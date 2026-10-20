#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

#include <string>

namespace esphome {
namespace access_site {

class AccessSite : public Component {
 public:
  void set_name(const std::string &name) { this->name_ = name; }
  const std::string &get_name() const { return this->name_; }

  void set_name_sensor(text_sensor::TextSensor *sensor) { this->name_sensor_ = sensor; }

  void setup() override;
  void dump_config() override;

 protected:
  std::string name_;
  text_sensor::TextSensor *name_sensor_{nullptr};
};

}  // namespace access_site
}  // namespace esphome
