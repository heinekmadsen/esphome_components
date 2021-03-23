#pragma once

#include "../genvex.h"
#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace genvex {

class GenvexClimate : public climate::Climate, public Component {
 public:
  GenvexClimate(Genvex *genvex) : genvex_(genvex) {}
  void setup() override;
  void dump_config() override;

  void set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; }

 protected:
  Genvex *genvex_;
  /// Override control to change settings of the climate device.
  void control(const climate::ClimateCall &call) override;
  /// Return the traits of this controller.
  climate::ClimateTraits traits() override;

  /// The sensor used for getting the current temperature
  sensor::Sensor *sensor_{nullptr};
};

}  // namespace genvex
}  // namespace esphome
