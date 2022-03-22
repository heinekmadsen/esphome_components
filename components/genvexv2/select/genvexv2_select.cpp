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

  auto options = traits.get_options();

  if(received_value < options.size()) {
    auto select_value = options[received_value];
    ESP_LOGD(TAG, "Select new state : %s", select_value.c_str());
    this->publish_state(select_value);
  }
}

void Genvexv2Select::control(const std::string &value) {
  ESP_LOGD(TAG, "Genvexv2 Select state: %s", value.c_str());

  auto options = traits.get_options();

  for(auto i = 0; i<options.size(); ++i) {
    if(options[i] == value) {
      ESP_LOGD(TAG, "WRITING INDEX: %d", i);
      std::vector<uint16_t> data = modbus_controller::float_to_payload(i, modbus_controller::SensorValueType::U_WORD);
      uint16_t speed = i;
      ESP_LOGD(TAG, "UINT16_t speed: %i", speed);
      //auto write_cmd = ModbusCommandItem::create_write_multiple_command(modbus_controller_, this->start_address + this->offset, this->register_count, data);
      auto write_cmd = ModbusCommandItem::create_write_single_command(modbus_controller_, this->start_address, speed);

      // publish new value
      write_cmd.on_data_func = 
      [this, write_cmd, value](ModbusRegisterType register_type, uint16_t start_address, const std::vector<uint8_t> &data) {
        // gets called when the write command is ack'd from the device
        modbus_controller_->on_write_register_response(write_cmd.register_type, start_address, data);
        this->publish_state(value);
      };
      modbus_controller_->queue_command(write_cmd);
    }
  }
}

} // namespace genvexv2
} // namespace esphome