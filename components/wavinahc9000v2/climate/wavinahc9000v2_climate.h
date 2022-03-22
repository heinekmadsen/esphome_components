#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/modbus_controller/modbus_controller.h"

namespace esphome {
namespace wavinahc9000v2 {
class Wavinahc9000v2Climate : public climate::Climate, public Component {
public:

  Wavinahc9000v2Climate() {}

  void setup() override;
  void dump_config() override;

  void set_current_temp_sensor(sensor::Sensor *sensor) {
    this->current_temp_sensor_ = sensor;
  }

  void set_temp_setpoint_number(number::Number *number) {
    this->temp_setpoint_number_ = number;
  }

  void set_mode_switch(switch_::Switch *switch_) {
    this->mode_switch_ = switch_;
  }


protected:
  /// Override control to change settings of the climate device.
  void control(const climate::ClimateCall& call) override;

  /// Return the traits of this controller.
  climate::ClimateTraits traits() override;

  /// The sensor used for getting the current temperature
  sensor::Sensor *current_temp_sensor_{ nullptr };

  /// The number component used for getting the temperature setpoint
  number::Number *temp_setpoint_number_{ nullptr };

  /// The select component used for getting the operation mode
  switch_::Switch *mode_switch_{ nullptr };

private:
};
} // namespace wavinahc9000v2
} // namespace esphome
