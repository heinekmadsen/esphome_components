#include "wavinAhc9000_climate.h"
#include "esphome/core/log.h"

namespace esphome {
namespace wavinAhc9000 {

static const char *TAG = "wavinAhc9000.climate";

void WavinAhc9000Climate::setup() {
  wavin_->add_temp_callback(channel_, [this](float state) {
    current_temperature = state;
    publish_state();
  });
  wavin_->add_target_temp_callback(channel_, [this](float state) {
    target_temperature = state;
    publish_state();
  });
}

void WavinAhc9000Climate::control(const climate::ClimateCall &call) {
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
