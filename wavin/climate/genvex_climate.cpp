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
  current_temperature = sensor_->state;
  target_temperature = sensor_->state;
  mode = climate::CLIMATE_MODE_HEAT;
  fan_mode = climate::CLIMATE_FAN_OFF;
}

void GenvexClimate::control(const climate::ClimateCall &call) {
  if (call.get_target_temperature_low().has_value())
    ;
  //this->publish_state();
}

climate::ClimateTraits GenvexClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
  traits.set_supports_fan_mode_on(true);
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
