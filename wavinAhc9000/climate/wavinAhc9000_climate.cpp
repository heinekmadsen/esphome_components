#include "wavinAhc9000_climate.h"
#include "esphome/core/log.h"

namespace esphome {
namespace wavinAhc9000 {

static const char *TAG = "wavinAhc9000.climate";

void WavinAhc9000Climate::setup() {
  wavin_->add_temp_callback(channel_, [this](float state) {
    if (current_temperature != state) {
      current_temperature = state;
      publish_state();
    }
  });
  wavin_->add_target_temp_callback(channel_, [this](float state) {
    if (target_temperature != state) {
      target_temperature = state;
      publish_state();
    }
  });
  wavin_->add_output_callback(channel_, [this](bool is_on) {
    if (is_on && (mode != climate::CLIMATE_MODE_HEAT)) {
      mode = climate::CLIMATE_MODE_HEAT;
      publish_state();
    } else if (!is_on && (mode != climate::CLIMATE_MODE_OFF)) {
      mode = climate::CLIMATE_MODE_OFF;
      publish_state();
    }
  });
  if (battery_level_sensor_ != nullptr) {
    wavin_->add_bat_level_callback(channel_, [this](int level) {
      battery_level_sensor_->publish_state(level);
    });
  }
  if (current_temp_sensor_ != nullptr) {
    wavin_->add_temp_callback(channel_, [this](float state) {
      current_temp_sensor_->publish_state(state);
    });
  }  
}

void WavinAhc9000Climate::control(const climate::ClimateCall &call) {
  if (call.get_target_temperature().has_value()) {
    target_temperature = *call.get_target_temperature();
    float target = target_temperature;
    ESP_LOGD(TAG, "Target temperature for channel %d changed to: %f", channel_ + 1, target);
    wavin_->set_target_temp(channel_, target);
    target_temperature = target;
    publish_state();
  }
}

climate::ClimateTraits WavinAhc9000Climate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
  return traits;
}

void WavinAhc9000Climate::dump_config() {
  LOG_CLIMATE("", "WavinAhc9000 Climate", this);
}

}  // namespace wavinAhc9000
}  // namespace esphome
