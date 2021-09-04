#pragma once

#include "../nilan.h"
#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace nilan {
class NilanClimate : public climate::Climate, public Component {
public:

  NilanClimate(Nilan *nilan) : nilan_(nilan) {}

  void setup() override;
  void dump_config() override;

  void set_current_temp_sensor(sensor::Sensor *sensor) {
    this->current_temp_sensor_ = sensor;
  }

  void set_temp_setpoint_sensor(sensor::Sensor *sensor) {
    this->temp_setpoint_sensor_ = sensor;
  }

  void set_fan_speed_sensor(sensor::Sensor *sensor) {
    this->fan_speed_sensor_ = sensor;
  }

  void set_mode_sensor(text_sensor::TextSensor *sensor) {
    this->mode_sensor_ = sensor;
  }

protected:

  Nilan *nilan_;

  /// Override control to change settings of the climate device.
  void                   control(const climate::ClimateCall& call) override;

  /// Return the traits of this controller.
  climate::ClimateTraits traits() override;

  /// The sensor used for getting the current temperature
  sensor::Sensor *current_temp_sensor_{ nullptr };

  /// The sensor used for getting the temperature setpoint
  sensor::Sensor *temp_setpoint_sensor_{ nullptr };

  /// The sensor used for getting fan speed
  sensor::Sensor *fan_speed_sensor_{ nullptr };

  /// The text sensor used for getting the operation mode
  text_sensor::TextSensor *mode_sensor_{ nullptr };

private:

  void nilanfanspeed_to_fanmode(const int state)
  {
    climate::ClimateFanMode return_value;

    switch (state) {
    case 1:
    case 2:
    case 3:
    case 4:
      this->custom_fan_mode = esphome::to_string(state);
      break;
    case 0:
    default: 
      this->fan_mode = climate::CLIMATE_FAN_OFF; 
      break;
    }
  }

  int climatemode_to_nilanoperationmode(const climate::ClimateMode mode)
  {
    int return_value;

    switch (mode) {
    case climate::CLIMATE_MODE_OFF: return_value = 0; break;

    case climate::CLIMATE_MODE_HEAT: return_value = 1; break;

    case climate::CLIMATE_MODE_COOL: return_value = 2; break;

    case climate::CLIMATE_MODE_HEAT_COOL: return_value = 3; break;

    default: return_value = 4; break;
    }

    return return_value;
  }

  void nilanmodetext_to_climatemode(const std::string& nilan_mode)
  {
    if (nilan_mode == "Off") {
      this->mode = climate::CLIMATE_MODE_OFF;
    }
    else if (nilan_mode == "Heat") {
      this->mode = climate::CLIMATE_MODE_HEAT;
    }
    else if (nilan_mode == "Cool") {
      this->mode = climate::CLIMATE_MODE_COOL;
    }
    else {
      this->mode = climate::CLIMATE_MODE_HEAT_COOL;
    }
  }
};
} // namespace nilan
} // namespace esphome
