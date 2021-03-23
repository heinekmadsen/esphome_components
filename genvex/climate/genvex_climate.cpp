#include "genvex_climate.h"
#include "esphome/core/log.h"

namespace esphome {
namespace genvex {

static const char *TAG = "genvex.climate";

void GenvexClimate::setup() {
  sensor_->add_on_state_callback([this](float state) {
    current_temperature = state;
    publish_state();
  });
  genvex_->add_target_temp_callback([this](float state) {
    target_temperature = state;
    publish_state();
  });
  genvex_->add_fan_speed_callback([this](int state) {
    climate::ClimateFanMode fmode;
    switch (state) {
      case 2: fmode = climate::CLIMATE_FAN_LOW; break;
      case 3: fmode = climate::CLIMATE_FAN_MEDIUM; break;
      case 4: fmode = climate::CLIMATE_FAN_HIGH; break;
      default: fmode = climate::CLIMATE_FAN_OFF; break;
    }
    fan_mode = fmode;
    publish_state();
  });
  current_temperature = sensor_->state;
  target_temperature = sensor_->state;
  mode = climate::CLIMATE_MODE_HEAT;
  fan_mode = climate::CLIMATE_FAN_OFF;
}

void GenvexClimate::control(const climate::ClimateCall &call) {
  if (call.get_target_temperature().has_value())
  {
	  this->target_temperature = *call.get_target_temperature();
		float target = target_temperature;
		//fan_mode = *call.get_fan_mode();
	  ESP_LOGD(TAG, "Target temperature changed to: %", target);
	  Genvex::writeTargetTemperature(target);
  }
  
  if (call.get_target_temperature_low().has_value())
    ;
  this->publish_state();
}

climate::ClimateTraits GenvexClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
  traits.set_supports_fan_mode_off(true);
  traits.set_supports_fan_mode_low(true);
  traits.set_supports_fan_mode_medium(true);
  traits.set_supports_fan_mode_high(true);
  return traits;
}

void GenvexClimate::dump_config() {
  LOG_CLIMATE("", "Genvex Climate", this);
}

}  // namespace genvex
}  // namespace esphome
