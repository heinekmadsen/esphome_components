#pragma once

#include "../wavinAhc9000.h"
#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace wavinAhc9000 {

class WavinAhc9000Climate : public climate::Climate, public Component {
 public:
  WavinAhc9000Climate(WavinAhc9000 *wavinAhc9000) : wavinAhc9000_(wavinAhc9000) {}
  void setup() override;
  void dump_config() override;

  void set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; }

 protected:
  WavinAhc9000 *wavinAhc9000_;
  /// Override control to change settings of the climate device.
  void control(const climate::ClimateCall &call) override;
  /// Return the traits of this controller.
  climate::ClimateTraits traits() override;

  /// The sensor used for getting the current temperature
  sensor::Sensor *sensor_{nullptr};
};

}  // namespace wavinAhc9000
}  // namespace esphome
