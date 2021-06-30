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
	  case 1: fmode = climate::CLIMATE_FAN_LOW; break;
      case 2: fmode = climate::CLIMATE_FAN_MEDIUM; break;
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
	  ESP_LOGD(TAG, "Target temperature changed to: %f", target);
	  genvex_->writeTargetTemperature(target);
  }
  
  if (call.get_fan_mode().has_value())
  {
	  int mode;
	  this->fan_mode = *call.get_fan_mode();
	  switch (fan_mode.value()) {
		  case climate::CLIMATE_FAN_LOW: mode = 1; break;
		  case climate::CLIMATE_FAN_MEDIUM: mode = 2; break;
		  case climate::CLIMATE_FAN_HIGH: mode = 4; break;
		  case climate::CLIMATE_FAN_OFF: mode = 0; break;
		  default: mode = 2; break;
	  }
	  ESP_LOGD(TAG, "Fan mode set to: %i", mode);
	  genvex_->writeFanMode(mode);
  }
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
