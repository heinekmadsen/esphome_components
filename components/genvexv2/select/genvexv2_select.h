#pragma once

#include "esphome/core/component.h"
#include "esphome/components/select/select.h"
#include "esphome/components/modbus_controller/modbus_controller.h"

namespace esphome {
namespace genvexv2 {

using modbus_controller::ModbusController;
using modbus_controller::SensorItem;
using modbus_controller::SensorValueType;
using modbus_controller::ModbusRegisterType;

class Genvexv2Select : public select::Select, public Component, public SensorItem {
public:
  Genvexv2Select(uint16_t start_address, uint8_t offset, uint32_t bitmask, SensorValueType value_type, 
              int register_count, uint8_t skip_updates, bool force_new_range)
      : select::Select(), Component(), SensorItem() {
    this->register_type = ModbusRegisterType::HOLDING;
    this->start_address = start_address;
    this->offset = offset;
    this->bitmask = bitmask;
    this->sensor_value_type = value_type;
    this->register_count = register_count;
    this->skip_updates = skip_updates;
    this->force_new_range = force_new_range;
  };

  void parse_and_publish(const std::vector<uint8_t> &data) override;
  void set_parent(ModbusController *modbus_controller) { this->modbus_controller_ = modbus_controller; }

protected:
  modbus_controller::ModbusController *modbus_controller_;

  void control(const std::string &value) override;
};
} // namespace genvexv2
} // namespace esphome