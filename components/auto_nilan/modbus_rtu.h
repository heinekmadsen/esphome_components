#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "esphome/components/uart/uart.h"
#include "esphome/core/gpio.h"

namespace esphome {
namespace auto_nilan {

struct ModbusTiming {
  uint32_t pre_tx_us{20};
  uint32_t post_tx_us{60};
  uint32_t inter_frame_us{4000};
  uint32_t rx_gap_us{250000};
  uint32_t rx_timeout_ms{600};
};

struct ModbusStats {
  uint32_t transactions{0};
  uint32_t retries{0};
  uint32_t failures{0};
  uint32_t timeouts{0};
  uint32_t crc_errors{0};
  uint32_t resyncs{0};
  uint32_t exceptions{0};
  uint8_t last_exception{0};
};

uint16_t modbus_crc16(const uint8_t *frame, size_t len);

void modbus_setup_gpio(GPIOPin *pin);
void modbus_compute_timing(uint32_t baud, ModbusTiming &out);

bool modbus_read_registers(uart::UARTDevice &uart, GPIOPin *flow, uint8_t address, uint8_t function_code,
                           uint16_t reg, uint16_t count, std::vector<uint16_t> &out, const ModbusTiming &timing,
                           ModbusStats &stats);

bool modbus_write_register(uart::UARTDevice &uart, GPIOPin *flow, uint8_t address, uint16_t reg, uint16_t value,
                           const ModbusTiming &timing, ModbusStats &stats);

// FC 0x10 write multiple registers. The CTS602 accepts single-register FC06
// requests but does not apply them, so writes must use this function.
bool modbus_write_registers(uart::UARTDevice &uart, GPIOPin *flow, uint8_t address, uint16_t reg,
                            const uint16_t *values, uint16_t count, const ModbusTiming &timing,
                            ModbusStats &stats);

}  // namespace auto_nilan
}  // namespace esphome
