#pragma once

#include "../wavinAhc9000.h"
#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace wavinAhc9000 {

class WavinAhc9000Climate : public climate::Climate, public Component {
 public:
  WavinAhc9000Climate(WavinAhc9000 *wavin) : wavin_(wavin) {}
  void setup() override;
  void dump_config() override;

  void set_channel(int channel) { channel_ = channel; }
  void set_battery_level_sensor(sensor::Sensor *battery_level_sensor) { battery_level_sensor_ = battery_level_sensor; }
  void set_current_temp_sensor(sensor::Sensor *current_temp_sensor) { current_temp_sensor_ = current_temp_sensor; }

 protected:
  WavinAhc9000 *wavin_;
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  int channel_;
  sensor::Sensor *battery_level_sensor_{nullptr};
  sensor::Sensor *current_temp_sensor_{nullptr};
};

}  // namespace wavinAhc9000
}  // namespace esphome
