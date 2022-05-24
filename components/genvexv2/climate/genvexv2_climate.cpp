#include "genvexv2_climate.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace genvexv2 {
static const char *TAG = "genvexv2.climate";

void Genvexv2Climate::setup() {
  current_temp_sensor_->add_on_state_callback([this](float state) {
    ESP_LOGD(TAG, "CURRENT TEMP SENSOR CALLBACK: %f", state);
    current_temperature = state;
    publish_state();
  });
  temp_setpoint_number_->add_on_state_callback([this](float state) {
    ESP_LOGD(TAG, "TEMP SETPOINT SENSOR CALLBACK: %f", state);
    target_temperature = state;
    publish_state();
  });
  fan_speed_number_->add_on_state_callback([this](float state) {
    ESP_LOGD(TAG, "FAN SPEED SENSOR CALLBACK: %f", state);
    genvexv2fanspeed_to_fanmode(state);
    publish_state();
  });

  current_temperature = current_temp_sensor_->state;
  target_temperature  = temp_setpoint_number_->state;
  genvexv2fanspeed_to_fanmode(fan_speed_number_->state);
}

void Genvexv2Climate::control(const climate::ClimateCall& call) {
  if (call.get_target_temperature().has_value())
  {
    this->target_temperature = *call.get_target_temperature();
    float target = target_temperature;
    ESP_LOGD(TAG, "Target temperature changed to: %f", target);
    temp_setpoint_number_->make_call().set_value(target).perform();//set(target);
  }

  if (call.get_mode().has_value())
  {
    ESP_LOGD("TAG", "Mode changed from Home Assistant");
    auto new_mode = *call.get_mode();
    mode = new_mode;
    switch (mode) {
      case climate::CLIMATE_MODE_OFF: 
      {
        ESP_LOGD("TAG", "Mode changed to OFF");
        this->fan_mode = climate::CLIMATE_FAN_OFF;
        // The only valid fan mode that is not custom is "OFF"
        auto new_fan_mode = *call.get_fan_mode();
        custom_fan_mode.reset();

        ESP_LOGD(TAG, "Custom Fan mode set to: 0");
        fan_speed_number_->make_call().set_value(0).perform();//set(0);
        break;
      }
      case climate::CLIMATE_MODE_AUTO: 
      {
        ESP_LOGD("TAG", "Mode changed to AUTO");
        this->custom_fan_mode = esphome::to_string("2");
        fan_mode.reset();
        auto optional_genvexv2_fan_mode = parse_number<float>("2");
        if(optional_genvexv2_fan_mode.has_value())
        {
          auto genvexv2_fan_mode = optional_genvexv2_fan_mode.value();
          ESP_LOGD(TAG, "Custom Fan mode set to: %i", static_cast<int>(genvexv2_fan_mode));
          fan_speed_number_->make_call().set_value(genvexv2_fan_mode).perform();;//set(genvexv2_fan_mode);
        }
        break;
      }
      default:
        break;
    }
  }

  if (call.get_fan_mode().has_value())
  {
    ESP_LOGD("TAG", "Fan mode changed to OFF from Home Assistant");
    // The only valid fan mode that is not custom is "OFF"
    auto new_fan_mode = *call.get_fan_mode();
    custom_fan_mode.reset();

    ESP_LOGD(TAG, "Fan mode set to: 0");
    fan_speed_number_->make_call().set_value(0).perform();//set(0);
  }

  if (call.get_custom_fan_mode().has_value())
  {
    ESP_LOGD("TAG", "Fan mode changed to custom fan mode 1-4 from Home Assistant");
    auto new_custom_fan_mode = *call.get_custom_fan_mode();
    custom_fan_mode = new_custom_fan_mode;
    fan_mode.reset();
    auto optional_genvexv2_fan_mode = parse_number<float>(new_custom_fan_mode.c_str());
    if(optional_genvexv2_fan_mode.has_value())
    {
      auto genvexv2_fan_mode = optional_genvexv2_fan_mode.value();
      ESP_LOGD(TAG, "Custom Fan mode set to: %i", static_cast<int>(genvexv2_fan_mode));
      fan_speed_number_->make_call().set_value(genvexv2_fan_mode).perform();//set(genvexv2_fan_mode);
    }
  }
  this->publish_state();
}

climate::ClimateTraits Genvexv2Climate::traits() {
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
    climate::ClimateMode::CLIMATE_MODE_OFF,
    climate::ClimateMode::CLIMATE_MODE_AUTO,
   });

  traits.set_supports_current_temperature(true);
  traits.set_visual_temperature_step(0.1);
  traits.set_visual_min_temperature(5);
  traits.set_visual_max_temperature(30);

  return traits;
}

void Genvexv2Climate::dump_config() {
  LOG_CLIMATE("", "Genvexv2 Climate", this);
}

void Genvexv2Climate::genvexv2fanspeed_to_fanmode(const int state)
{
  ESP_LOGD("TAG", "In genvexv2fanspeed_to_fanmode");
  climate::ClimateFanMode return_value;
  ESP_LOGD("TAG", "State is %i", state);
  this->custom_fan_mode.reset();
  this->fan_mode.reset();

  switch (state) {
  case 1:
    ESP_LOGD("TAG", "Case 1");
    this->mode = climate::CLIMATE_MODE_AUTO;
    this->custom_fan_mode = esphome::to_string(state);
    break;
  case 2:
    ESP_LOGD("TAG", "Case 2");
    this->mode = climate::CLIMATE_MODE_AUTO;
    this->custom_fan_mode = esphome::to_string(state);
    break;
  case 3:
    ESP_LOGD("TAG", "Case 3");
    this->mode = climate::CLIMATE_MODE_AUTO;
    this->custom_fan_mode = esphome::to_string(state);
    break;
  case 4:
    ESP_LOGD("TAG", "Case 4");
    this->mode = climate::CLIMATE_MODE_AUTO;
    this->custom_fan_mode = esphome::to_string(state);
    break;
  case 0:
    ESP_LOGD("TAG", "Case 0");
    this->fan_mode = climate::CLIMATE_FAN_OFF; 
    this->mode = climate::CLIMATE_MODE_OFF;
    break;
  default: 
    ESP_LOGD("TAG", "Case default");
    this->fan_mode = climate::CLIMATE_FAN_OFF; 
    this->mode = climate::CLIMATE_MODE_OFF;
    break;
  }
}

} // namespace genvexv2
} // namespace esphome
