#include "nilan_climate.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace nilan {
static const char *TAG = "nilan.climate";

void NilanClimate::setup() {
  current_temp_sensor_->add_on_state_callback([this](float state) {
        // ESP_LOGD(TAG, "CURRENT TEMP SENSOR CALLBACK: %f", state);
        current_temperature = state;
        publish_state();
      });
  temp_setpoint_sensor_->add_on_state_callback([this](float state) {
        // ESP_LOGD(TAG, "TEMP SETPOINT SENSOR CALLBACK: %f", state);
        target_temperature = state;
        publish_state();
      });
  mode_sensor_->add_on_state_callback([this](std::string state) {
        // ESP_LOGD(TAG, "OPERATION MODE CALLBACK: %s", state.c_str());
        nilanmodetext_to_climatemode(state);
        publish_state();
      });
  fan_speed_sensor_->add_on_state_callback([this](float state) {
        // ESP_LOGD(TAG, "FAN SPEED SENSOR CALLBACK: %f", state);
        nilanfanspeed_to_fanmode(state);
        publish_state();
      });

  current_temperature = current_temp_sensor_->state;
  target_temperature  = temp_setpoint_sensor_->state;
  nilanmodetext_to_climatemode(mode_sensor_->state);
  nilanfanspeed_to_fanmode(fan_speed_sensor_->state); // Will update either fan_mode or custom_fan_mode
}

void NilanClimate::control(const climate::ClimateCall& call) {
  if (call.get_target_temperature().has_value())
  {
    this->target_temperature = *call.get_target_temperature();
    float target = target_temperature;
    ESP_LOGD(TAG, "Target temperature changed to: %f", target);
    nilan_->writeTargetTemperature(target);
  }

  if (call.get_mode().has_value())
  {
    auto new_mode = *call.get_mode();
    mode = new_mode;
    int operation_mode = climatemode_to_nilanoperationmode(new_mode);

    ESP_LOGD(TAG, "Operation mode changed to: %d", operation_mode);

    if (operation_mode > 0) {
      nilan_->writeRunset(1);
      nilan_->writeOperationMode(operation_mode);
    }
    else {
      nilan_->writeRunset(0);
    }
  }

  if (call.get_fan_mode().has_value())
  {
    int  nilan_fan_mode;
    auto new_fan_mode = *call.get_fan_mode();
    fan_mode = new_fan_mode;
    custom_fan_mode.reset();

    switch (new_fan_mode) {
      case climate::CLIMATE_FAN_OFF: 
        nilan_fan_mode = 0; 
        break;
      default: 
        nilan_fan_mode = 2; 
        break;
    }
    ESP_LOGD(TAG, "Fan mode set to: %i", nilan_fan_mode);
    nilan_->writeFanMode(nilan_fan_mode);
  }
  if (call.get_custom_fan_mode().has_value())
  {
    auto new_custom_fan_mode = *call.get_custom_fan_mode();
    custom_fan_mode = new_custom_fan_mode;
    fan_mode.reset();
    auto optional_nilan_fan_mode = esphome::parse_int(new_custom_fan_mode);
    if(optional_nilan_fan_mode.has_value())
    {
      auto nilan_fan_mode = optional_nilan_fan_mode.value();
      ESP_LOGD(TAG, "Custom Fan mode set to: %i", nilan_fan_mode);
      nilan_->writeFanMode(nilan_fan_mode);
    }
  }
  this->publish_state();
}

climate::ClimateTraits NilanClimate::traits() {
  auto traits = climate::ClimateTraits();

  traits.set_supported_custom_fan_modes({
    "1",
    "2",
    "3",
    "4"
  });

  traits.set_supported_fan_modes({ 
    climate::ClimateFanMode::CLIMATE_FAN_OFF 
  });

  traits.set_supported_modes({
    climate::ClimateMode::CLIMATE_MODE_COOL,
    climate::ClimateMode::CLIMATE_MODE_HEAT,
    climate::ClimateMode::CLIMATE_MODE_HEAT_COOL
   });

  traits.set_supports_current_temperature(true);
  traits.set_visual_temperature_step(1);
  traits.set_visual_min_temperature(5);
  traits.set_visual_max_temperature(30);

  return traits;
}

void NilanClimate::dump_config() {
  LOG_CLIMATE("", "Nilan Climate", this);
}
} // namespace nilan
} // namespace esphome
