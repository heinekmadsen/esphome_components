#include "genvexv2_select.h"
#include "esphome/core/log.h"

namespace esphome {
namespace genvexv2 {

static const char *TAG = "genvexv2.select";

using modbus_controller::ModbusCommandItem;
using modbus_controller::ModbusRegisterType;

void Genvexv2Select::parse_and_publish(const std::vector<uint8_t> &data) {
  union {
    float float_value;
    uint32_t raw;
  } raw_to_float;

  float received_value = payload_to_float(data, *this);
  ESP_LOGD(TAG, "Genvexv2 Select index: %f", received_value);

  const auto &options = traits.get_options();

  if (received_value < options.size()) {
    const char *select_value = options[static_cast<size_t>(received_value)];
    ESP_LOGD(TAG, "Select new state : %s", select_value);
    this->publish_state(select_value);
  }
}

void Genvexv2Select::control(const std::string &value) {
  ESP_LOGD(TAG, "Genvexv2 Select state: %s", value.c_str());

  const auto &options = traits.get_options();

  for (size_t i = 0; i < options.size(); ++i) {
    if (value == options[i]) {
      ESP_LOGD(TAG, "WRITING INDEX: %zu", i);
      uint16_t speed = static_cast<uint16_t>(i);
      ESP_LOGD(TAG, "UINT16_t speed: %u", speed);
      auto write_cmd = ModbusCommandItem::create_write_single_command(modbus_controller_, this->start_address, speed);

      // publish new value
      write_cmd.on_data_func =
        [this, write_cmd, value](ModbusRegisterType register_type, uint16_t start_address, const std::vector<uint8_t> &data) {
          modbus_controller_->on_write_register_response(write_cmd.register_type, start_address, data);
          this->publish_state(value.c_str());
        };
      modbus_controller_->queue_command(write_cmd);
    }
  }
}

} // namespace genvexv2
} // namespace esphome
