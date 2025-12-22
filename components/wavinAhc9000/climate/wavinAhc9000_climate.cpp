#include "wavinAhc9000_climate.h"
#include "esphome/core/log.h"

namespace esphome {
namespace wavinAhc9000 {

static const char *TAG = "wavinAhc9000.climate";

void WavinAhc9000Climate::setup() {
  // Register this channel as being in use
  wavin_->register_channel(channel_);
  
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
  if (call.get_mode().has_value()) {
    // Mode changes are handled by the hardware/output callbacks
    // We just acknowledge the mode change here
    this->mode = *call.get_mode();
  }
  if (call.get_target_temperature().has_value()) {
    this->target_temperature = *call.get_target_temperature();
    ESP_LOGD(TAG, "Target temperature for channel %d changed to: %.1f", channel_ + 1, this->target_temperature);
    wavin_->set_target_temp(channel_, this->target_temperature);
  }
  this->publish_state();
}

climate::ClimateTraits WavinAhc9000Climate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
  traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT});
  traits.set_visual_min_temperature(5.0);
  traits.set_visual_max_temperature(40.0);
  traits.set_visual_temperature_step(0.5);
  return traits;
}

void WavinAhc9000Climate::dump_config() {
  LOG_CLIMATE("", "WavinAhc9000 Climate", this);
}

}  // namespace wavinAhc9000
}  // namespace esphome
