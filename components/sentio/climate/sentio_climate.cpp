#include "sentio_climate.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace sentio {
static const char *TAG = "sentio.climate";

void SentioClimate::setup() {
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

  mode_select_->add_on_state_callback([this](float state) {
    // ESP_LOGD(TAG, "FAN SPEED SENSOR CALLBACK: %f", state);
    sentio_mode_to_climatemode(state);
    publish_state();
  });

  current_temperature = current_temp_sensor_->state;
  target_temperature  = temp_setpoint_number_->state;
  sentio_mode_to_climatemode(mode_select_->state); 
}

void SentioClimate::control(const climate::ClimateCall& call) {
  if (call.get_target_temperature().has_value())
  {
    this->target_temperature = *call.get_target_temperature();
    float target = target_temperature;
    ESP_LOGD(TAG, "Target temperature changed to: %f", target);
    temp_setpoint_number_->set(target);
  }
}

climate::ClimateTraits SentioClimate::traits() {
  auto traits = climate::ClimateTraits();

  traits.set_supports_current_temperature(true);
  traits.set_visual_temperature_step(0.5);
  traits.set_visual_min_temperature(5);
  traits.set_visual_max_temperature(30);

  return traits;
}

void SentioClimate::dump_config() {
  LOG_CLIMATE("", "Sentio Climate", this);
}

void SentioClimate::sentio_mode_to_climatemode(const int state)
{
  switch (state) {
  case 1:
    this->mode = climate::CLIMATE_MODE_OFF;
    break;
  case 2:
    this->mode = climate::CLIMATE_MODE_HEAT;
    break;
  case 3:
    this->mode = climate::CLIMATE_MODE_COOL;
    break;
  default: 
    this->mode = climate::CLIMATE_MODE_OFF;
    break;
  }
}

} // namespace sentio
} // namespace esphome
