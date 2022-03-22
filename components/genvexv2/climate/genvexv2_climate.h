#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"

namespace esphome {
namespace genvexv2 {
class Genvexv2Climate : public climate::Climate, public Component {
public:

  Genvexv2Climate() {}

  void setup() override;
  void dump_config() override;

  void set_current_temp_sensor(sensor::Sensor *sensor) {
    this->current_temp_sensor_ = sensor;
  }

  void set_temp_setpoint_number(number::Number *number) {
    this->temp_setpoint_number_ = number;
  }

  void set_fan_speed_number(number::Number *number) {
    this->fan_speed_number_ = number;
  }

  //void set_mode_select(select::Select *select) {
  //  this->mode_select_ = select;
  //}

protected:
  /// Override control to change settings of the climate device.
  void control(const climate::ClimateCall& call) override;

  /// Return the traits of this controller.
  climate::ClimateTraits traits() override;

  /// The sensor used for getting the current temperature
  sensor::Sensor *current_temp_sensor_{ nullptr };

  /// The number component used for getting the temperature setpoint
  number::Number *temp_setpoint_number_{ nullptr };

  /// The number component used for getting fan speed
  number::Number *fan_speed_number_{ nullptr };

  /// The select component used for getting the operation mode
  //select::Select *mode_select_{ nullptr };

private:

  void genvexv2fanspeed_to_fanmode(const int state);
  int climatemode_to_genvexv2operationmode(const climate::ClimateMode mode);
  void genvexv2modetext_to_climatemode(const std::string& genvexv2_mode);
};
} // namespace genvexv2
} // namespace esphome