#include "modbus_rtu.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace auto_nilan {

static const char *const TAG = "auto_nilan.modbus";

uint16_t modbus_crc16(const uint8_t *frame, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= frame[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x0001) {
        crc = (uint16_t) ((crc >> 1) ^ 0xA001);
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

void modbus_setup_gpio(GPIOPin *pin) {
  if (pin == nullptr)
    return;
  pin->setup();
  pin->digital_write(false);
}

void modbus_compute_timing(uint32_t baud, ModbusTiming &out) {
  if (baud == 0)
    baud = 19200;
  const uint32_t bits_per_frame = 11;  // start + 8 data + parity + stop
  const uint32_t frame_us = (bits_per_frame * 1000000UL + baud - 1) / baud;
  out.pre_tx_us = std::max<uint32_t>(frame_us / 4, 20);
  out.post_tx_us = std::max<uint32_t>(frame_us / 2, 60);
  out.inter_frame_us = std::max<uint32_t>(frame_us * 7, 2000);
  // The CTS602 can pause for several milliseconds between bytes inside a
  // response, so allow a generous quiet gap before treating a frame as
  // truncated. The overall response timeout still bounds the wait.
  out.rx_gap_us = std::max<uint32_t>(frame_us * 200, 120000);
}

static void log_frame_bytes(const char *what, uint8_t function_code, uint16_t reg, const std::vector<uint8_t> &frame,
                            size_t total_len) {
  std::string hex;
  char byte[4];
  for (size_t i = 0; i < frame.size() && i < 32; i++) {
    snprintf(byte, sizeof(byte), "%02X ", frame[i]);
    hex += byte;
  }
  ESP_LOGD(TAG, "%s FC 0x%02X reg %u %u/%u bytes: %s", what, function_code, reg, (unsigned) frame.size(),
           (unsigned) total_len, hex.c_str());
}

// Reads one complete reply frame. Leading noise is discarded and a quiet gap
// longer than a few character times ends the wait, so a truncated answer does
// not block the bus for the whole timeout.
static bool receive_frame(uart::UARTDevice &uart, uint8_t address, uint8_t function_code, uint16_t reg,
                          size_t total_len, const ModbusTiming &timing, ModbusStats &stats,
                          std::vector<uint8_t> &frame) {
  const uint32_t deadline = millis() + timing.rx_timeout_ms;
  uint32_t last_byte_us = micros();
  bool seen_any = false;

  frame.clear();
  frame.reserve(total_len + 4);

  while ((int32_t) (millis() - deadline) < 0) {
    bool read_something = false;
    while (uart.available()) {
      int c = uart.read();
      if (c < 0)
        break;
      read_something = true;
      seen_any = true;
      last_byte_us = micros();
      frame.push_back((uint8_t) c);

      while (!frame.empty() && frame[0] != address) {
        frame.erase(frame.begin());
        stats.resyncs++;
      }
      if (frame.size() >= 2) {
        const uint8_t fc = frame[1];
        if (fc == (uint8_t) (function_code | 0x80)) {
          if (frame.size() >= 5) {
            ESP_LOGD(TAG, "Modbus exception code 0x%02X for FC 0x%02X reg %u", frame[2], function_code, reg);
            stats.last_exception = frame[2];
            stats.exceptions++;
            return false;
          }
          continue;
        }
        if (fc != function_code) {
          frame.erase(frame.begin());
          stats.resyncs++;
          continue;
        }
      }
      if (frame.size() > total_len) {
        frame.erase(frame.begin());
        stats.resyncs++;
        continue;
      }
      if (frame.size() == total_len) {
        if (modbus_crc16(frame.data(), frame.size()) == 0)
          return true;
        stats.crc_errors++;
        return false;
      }
    }
    if (seen_any && (uint32_t) (micros() - last_byte_us) > timing.rx_gap_us) {
      ESP_LOGD(TAG, "RX gap end after %u us", (unsigned) (micros() - last_byte_us));
      log_frame_bytes("RX gap", function_code, reg, frame, total_len);
      break;
    }
    if (!read_something)
      delayMicroseconds(50);
  }
  log_frame_bytes("RX timeout", function_code, reg, frame, total_len);
  stats.timeouts++;
  return false;
}

// Discards any leftover bytes and waits until the bus has been quiet for
// idle_us, so a late reply from an aborted attempt cannot collide with the
// next request.
static void wait_for_bus_idle(uart::UARTDevice &uart, uint32_t idle_us) {
  const uint32_t start = micros();
  uint32_t last_activity = start;
  for (;;) {
    bool read_something = false;
    while (uart.available()) {
      uart.read();
      read_something = true;
    }
    const uint32_t now = micros();
    if (read_something)
      last_activity = now;
    if ((uint32_t) (now - last_activity) >= idle_us)
      return;
    if ((uint32_t) (now - start) > 50000)
      return;
    delayMicroseconds(50);
  }
}

static void transmit(uart::UARTDevice &uart, GPIOPin *flow, const uint8_t *data, size_t len,
                     const ModbusTiming &timing) {
  if (flow != nullptr) {
    flow->digital_write(true);
    delayMicroseconds(timing.pre_tx_us);
  }
  uart.write_array(data, len);
  uart.flush();
  if (flow != nullptr) {
    delayMicroseconds(timing.post_tx_us);
    flow->digital_write(false);
  }
}

bool modbus_read_registers(uart::UARTDevice &uart, GPIOPin *flow, uint8_t address, uint8_t function_code,
                           uint16_t reg, uint16_t count, std::vector<uint16_t> &out, const ModbusTiming &timing,
                           ModbusStats &stats) {
  if (count == 0 || count > 125) {
    ESP_LOGW(TAG, "Invalid read count %u", (unsigned) count);
    return false;
  }

  stats.transactions++;
  for (uint8_t attempt = 0; attempt < 2; attempt++) {
    if (attempt > 0)
      stats.retries++;
    stats.last_exception = 0;
    wait_for_bus_idle(uart, timing.inter_frame_us);

    uint8_t msg[8];
    msg[0] = address;
    msg[1] = function_code;
    msg[2] = (uint8_t) (reg >> 8);
    msg[3] = (uint8_t) (reg & 0xFF);
    msg[4] = (uint8_t) (count >> 8);
    msg[5] = (uint8_t) (count & 0xFF);
    const uint16_t crc = modbus_crc16(msg, 6);
    msg[6] = (uint8_t) (crc & 0xFF);
    msg[7] = (uint8_t) (crc >> 8);

    transmit(uart, flow, msg, sizeof(msg), timing);

    const uint8_t byte_count = (uint8_t) (count * 2);
    std::vector<uint8_t> frame;
    if (!receive_frame(uart, address, function_code, reg, (size_t) byte_count + 5, timing, stats, frame))
      continue;
    if (frame[2] != byte_count) {
      ESP_LOGW(TAG, "Read byte count mismatch: got %u expected %u", frame[2], byte_count);
      stats.failures++;
      continue;
    }
    out.clear();
    out.reserve(count);
    for (uint16_t i = 0; i < count; i++) {
      out.push_back((uint16_t) (((uint16_t) frame[3 + 2 * i] << 8) | frame[4 + 2 * i]));
    }
    return true;
  }
  stats.failures++;
  return false;
}

bool modbus_write_register(uart::UARTDevice &uart, GPIOPin *flow, uint8_t address, uint16_t reg, uint16_t value,
                           const ModbusTiming &timing, ModbusStats &stats) {
  stats.transactions++;
  for (uint8_t attempt = 0; attempt < 2; attempt++) {
    if (attempt > 0)
      stats.retries++;
    stats.last_exception = 0;
    wait_for_bus_idle(uart, timing.inter_frame_us);

    uint8_t msg[8];
    msg[0] = address;
    msg[1] = 0x06;
    msg[2] = (uint8_t) (reg >> 8);
    msg[3] = (uint8_t) (reg & 0xFF);
    msg[4] = (uint8_t) (value >> 8);
    msg[5] = (uint8_t) (value & 0xFF);
    const uint16_t crc = modbus_crc16(msg, 6);
    msg[6] = (uint8_t) (crc & 0xFF);
    msg[7] = (uint8_t) (crc >> 8);

    transmit(uart, flow, msg, sizeof(msg), timing);

    std::vector<uint8_t> frame;
    if (!receive_frame(uart, address, 0x06, reg, sizeof(msg), timing, stats, frame))
      continue;
    if (frame.size() != sizeof(msg) || frame[2] != msg[2] || frame[3] != msg[3]) {
      ESP_LOGW(TAG, "Write echo mismatch for reg %u", (unsigned) reg);
      continue;
    }
    return true;
  }
  stats.failures++;
  return false;
}

bool modbus_write_registers(uart::UARTDevice &uart, GPIOPin *flow, uint8_t address, uint16_t reg,
                            const uint16_t *values, uint16_t count, const ModbusTiming &timing,
                            ModbusStats &stats) {
  if (count == 0 || count > 123) {
    ESP_LOGW(TAG, "Invalid write count %u", (unsigned) count);
    return false;
  }

  stats.transactions++;
  for (uint8_t attempt = 0; attempt < 2; attempt++) {
    if (attempt > 0)
      stats.retries++;
    stats.last_exception = 0;
    wait_for_bus_idle(uart, timing.inter_frame_us);

    const size_t msg_len = 9 + 2 * count;
    std::vector<uint8_t> msg(msg_len);
    msg[0] = address;
    msg[1] = 0x10;
    msg[2] = (uint8_t) (reg >> 8);
    msg[3] = (uint8_t) (reg & 0xFF);
    msg[4] = (uint8_t) (count >> 8);
    msg[5] = (uint8_t) (count & 0xFF);
    msg[6] = (uint8_t) (count * 2);
    for (uint16_t i = 0; i < count; i++) {
      msg[7 + 2 * i] = (uint8_t) (values[i] >> 8);
      msg[8 + 2 * i] = (uint8_t) (values[i] & 0xFF);
    }
    const uint16_t crc = modbus_crc16(msg.data(), msg_len - 2);
    msg[msg_len - 2] = (uint8_t) (crc & 0xFF);
    msg[msg_len - 1] = (uint8_t) (crc >> 8);

    transmit(uart, flow, msg.data(), msg_len, timing);

    std::vector<uint8_t> frame;
    if (!receive_frame(uart, address, 0x10, reg, 8, timing, stats, frame))
      continue;
    if (frame.size() != 8 || frame[2] != msg[2] || frame[3] != msg[3]) {
      ESP_LOGW(TAG, "Write echo mismatch for reg %u", (unsigned) reg);
      continue;
    }
    return true;
  }
  stats.failures++;
  return false;
}

}  // namespace auto_nilan
}  // namespace esphome

