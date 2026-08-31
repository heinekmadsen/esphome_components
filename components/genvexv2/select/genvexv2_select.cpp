#include "genvexv2_select.h"
#include "esphome/core/log.h"

namespace esphome {
namespace genvexv2 {

static const char *TAG = "genvexv2.select";

using modbus_controller::ModbusCommandItem;

void Genvexv2Select::parse_and_publish(std::span<const uint8_t> data) {
  float received_value = modbus_controller::payload_to_float(data, *this, this->offset);
  ESP_LOGD(TAG, "Genvexv2 Select index: %f", received_value);

  const auto &options = this->traits.get_options();

  if (received_value >= 0 && received_value < options.size()) {
    const auto index = static_cast<size_t>(received_value);
    ESP_LOGD(TAG, "Select new state : %s", options[index]);
    this->publish_state(index);
  }
}

void Genvexv2Select::control(size_t index) {
  const uint16_t speed = static_cast<uint16_t>(index);
  ESP_LOGD(TAG, "Genvexv2 Select state: %s - WRITING INDEX: %zu (speed: %u)", this->option_at(index), index, speed);

  auto write_cmd =
      ModbusCommandItem::create_write_single_command(this->modbus_controller_, this->write_address(), speed);

  // publish new value once the device ack's the write
  write_cmd.on_data_func = [this, index](modbus::EntityType register_type, uint16_t start_address,
                                         std::span<const uint8_t> data) {
    this->modbus_controller_->on_write_register_response(register_type, start_address, data);
    this->publish_state(index);
  };
  this->modbus_controller_->queue_command(std::move(write_cmd));
}

} // namespace genvexv2
} // namespace esphome