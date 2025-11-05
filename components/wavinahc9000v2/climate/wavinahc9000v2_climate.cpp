#include "wavinahc9000v2_climate.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace wavinahc9000v2 {
static const char *TAG = "wavinahc9000v2.climate";

void Wavinahc9000v2Climate::setup() {
  current_temp_sensor_->add_on_state_callback([this](float state) {
    // ESP_LOGD(TAG, "CURRENT TEMP SENSOR CALLBACK: %f", state);
    current_temperature = state;
    publish_state();
  });
  temp_setpoint_number_->add_on_state_callback([this](float state) {
    // ESP_LOGD(TAG, "TEMP SETPOINT SENSOR CALLBACK: %f", state);
    target_temperature = state;
    publish_state();
  });
  mode_switch_->add_on_state_callback([this](bool state) {
    ESP_LOGD(TAG, "OPERATION MODE CALLBACK: %s", ONOFF(state));
    if (state) {
      this->mode = climate::CLIMATE_MODE_OFF;
    }
    else if (!state) {
      this->mode = climate::CLIMATE_MODE_AUTO;
    }
    publish_state();
  });
  hvac_action_->add_on_state_callback([this](bool state) {
    ESP_LOGD(TAG, "Current action is : %s", ONOFF(state));
    if (state) {
      this->action = climate::CLIMATE_ACTION_HEATING;
    }
    else if (!state) {
      this->action = climate::CLIMATE_ACTION_IDLE;
    }
    publish_state();
  });

  current_temperature = current_temp_sensor_->state;
  target_temperature  = temp_setpoint_number_->state;
}

void Wavinahc9000v2Climate::control(const climate::ClimateCall& call) {
  if (call.get_target_temperature().has_value())
  {
    this->target_temperature = *call.get_target_temperature();
    float target = ((roundf(target_temperature * 2.0) / 2));
    ESP_LOGV(TAG, "Rounded to nearest half: %f", target);
    ESP_LOGD(TAG, "Target temperature changed to: %f", target);
    temp_setpoint_number_->make_call().set_value(target).perform();//set(target);
  }

  if (call.get_mode().has_value())
  {
    ESP_LOGD(TAG, "MODE CHANGED");
    auto new_mode = *call.get_mode();
    mode = new_mode;

    if(mode == climate::CLIMATE_MODE_AUTO)
    {
      ESP_LOGD(TAG, "Turning off thermostat standby mode");
      mode_switch_->turn_off();
    }
    else if(mode == climate::CLIMATE_MODE_OFF)
    {
      ESP_LOGD(TAG, "Turning on thermostat standby mode");
      mode_switch_->turn_on();
    }
  }
  this->publish_state();
}

climate::ClimateTraits Wavinahc9000v2Climate::traits() {
  auto traits = climate::ClimateTraits();

  traits.set_supported_modes({
    climate::ClimateMode::CLIMATE_MODE_OFF,
    climate::ClimateMode::CLIMATE_MODE_AUTO,
  });

  traits.set_supports_action(true);

  traits.set_supports_current_temperature(true);
  traits.set_visual_current_temperature_step(0.1);
  traits.set_visual_target_temperature_step(0.5);
  traits.set_visual_min_temperature(6);
  traits.set_visual_max_temperature(40);

  return traits;
}

void Wavinahc9000v2Climate::dump_config() {
  LOG_CLIMATE("", "Wavinahc9000v2 Climate", this);
}

} // namespace wavinahc9000v2
} // namespace esphome
