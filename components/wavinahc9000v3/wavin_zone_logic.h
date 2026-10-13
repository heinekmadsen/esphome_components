#pragma once

// Pure, dependency-free helpers for the zone layer.
//
// Deliberately free of ESPHome/Arduino headers so the logic can be unit-tested
// natively on a host compiler (see test/test_zone_logic.cpp) instead of only
// being exercised on-device.

#include <cstdint>
#include <string>

namespace esphome {
namespace wavinahc9000v3 {

// Parses "3,4" / "3 4" / "3;4" into a channel bitmask (bit0 = channel 1).
// Returns false on a non-numeric token, a channel outside 1..16, or an empty
// result. The input comes straight from user text, so it is treated as
// hostile.
inline bool parse_channel_csv(const std::string &csv, uint16_t &mask_out) {
  uint16_t mask = 0;
  std::string tok;

  auto flush = [&mask, &tok]() -> bool {
    if (tok.empty()) return true;
    int v = 0;
    for (char c : tok) {
      if (c < '0' || c > '9') return false;
      v = v * 10 + (c - '0');
      if (v > 999) return false;
    }
    tok.clear();
    if (v < 1 || v > 16) return false;
    mask |= (uint16_t) (1u << (v - 1));
    return true;
  };

  for (char c : csv) {
    if (c == ',' || c == ';' || c == ' ' || c == '\t') {
      if (!flush()) return false;
    } else {
      tok += c;
    }
  }
  if (!flush()) return false;
  if (mask == 0) return false;
  mask_out = mask;
  return true;
}

inline std::string mask_to_csv(uint16_t mask) {
  std::string out;
  for (uint8_t ch = 1; ch <= 16; ch++) {
    if (mask & (uint16_t) (1u << (ch - 1))) {
      if (!out.empty()) out += ",";
      out += std::to_string((int) ch);
    }
  }
  return out;
}

// The whole trick: bias the setpoint by how far the external sensor disagrees
// with the thermostat, so the AHC-9000's own loop ends up chasing the external
// reading. Clamped so a wild sensor can't drag the setpoint somewhere unsafe.
inline float offset_setpoint(float air_temp, float target, float external, float lo, float hi) {
  float desired = air_temp + (target - external);
  if (desired < lo) desired = lo;
  if (desired > hi) desired = hi;
  return desired;
}

// True when a setpoint write is actually worth sending. MANUAL TEMPERATURE is
// non-volatile inside the AHC-9000, so needless writes wear out its EEPROM.
inline bool should_write_setpoint(bool first_write, float desired, float applied, float deadband, uint32_t now_ms,
                                  uint32_t last_write_ms, uint32_t min_interval_ms) {
  if (first_write) return true;
  float diff = desired - applied;
  if (diff < 0) diff = -diff;
  if (diff < deadband) return false;
  if (last_write_ms != 0 && (uint32_t) (now_ms - last_write_ms) < min_interval_ms) return false;
  return true;
}

}  // namespace wavinahc9000v3
}  // namespace esphome
