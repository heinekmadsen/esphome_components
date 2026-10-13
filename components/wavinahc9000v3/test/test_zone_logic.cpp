// Host-side unit tests for the pure zone logic.
//
//   g++ -std=c++17 -Wall -Werror -I.. -o t test_zone_logic.cpp && ./t
//
// These call the same functions the firmware calls, so a regression here is a
// real regression on-device.

#include "wavin_zone_logic.h"

#include <cstdio>
#include <cmath>

using namespace esphome::wavinahc9000v3;

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond, msg)                                                 \
  do {                                                                   \
    g_checks++;                                                          \
    if (!(cond)) {                                                       \
      std::printf("  FAIL: %s  (%s:%d)\n", msg, __FILE__, __LINE__);     \
      g_failures++;                                                      \
    }                                                                    \
  } while (0)

static void test_parse_channel_csv() {
  std::printf("parse_channel_csv\n");
  uint16_t m = 0;

  CHECK(parse_channel_csv("1", m) && m == 0x0001, "single channel 1");
  CHECK(parse_channel_csv("16", m) && m == 0x8000, "single channel 16");
  CHECK(parse_channel_csv("3,4", m) && m == 0x000C, "joined room 3,4");
  CHECK(parse_channel_csv("3, 4", m) && m == 0x000C, "tolerates spaces after comma");
  CHECK(parse_channel_csv("3 4", m) && m == 0x000C, "space separated");
  CHECK(parse_channel_csv("4;3", m) && m == 0x000C, "semicolon separated, order independent");
  CHECK(parse_channel_csv("1,1,1", m) && m == 0x0001, "duplicates collapse");
  CHECK(parse_channel_csv(" 2 , 5 ", m) && m == 0x0012, "leading/trailing whitespace");
  CHECK(parse_channel_csv("3,", m) && m == 0x0004, "trailing comma tolerated");

  // Hostile / malformed input must be rejected, not silently coerced.
  CHECK(!parse_channel_csv("", m), "empty string rejected");
  CHECK(!parse_channel_csv("   ", m), "whitespace-only rejected");
  CHECK(!parse_channel_csv("0", m), "channel 0 rejected");
  CHECK(!parse_channel_csv("17", m), "channel 17 rejected");
  CHECK(!parse_channel_csv("-1", m), "negative rejected");
  CHECK(!parse_channel_csv("3,abc", m), "non-numeric token rejected");
  CHECK(!parse_channel_csv("1,99", m), "out-of-range in list rejects whole list");
  CHECK(!parse_channel_csv("1.5", m), "decimal rejected");

  // A rejected parse must not clobber the caller's mask.
  uint16_t keep = 0xBEEF;
  parse_channel_csv("nonsense", keep);
  CHECK(keep == 0xBEEF, "output untouched on failure");
}

static void test_mask_to_csv() {
  std::printf("mask_to_csv\n");
  CHECK(mask_to_csv(0x0000) == "", "empty mask");
  CHECK(mask_to_csv(0x0001) == "1", "channel 1");
  CHECK(mask_to_csv(0x8000) == "16", "channel 16");
  CHECK(mask_to_csv(0x000C) == "3,4", "joined room");
  CHECK(mask_to_csv(0xFFFF) == "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16", "all channels");

  uint16_t m = 0;
  CHECK(parse_channel_csv(mask_to_csv(0x1234), m) && m == 0x1234, "round-trip 0x1234");
}

static void test_offset_setpoint() {
  std::printf("offset_setpoint\n");

  // External sensor reads colder than the thermostat -> ask the AHC for more heat.
  CHECK(std::fabs(offset_setpoint(22.0f, 21.0f, 20.0f, 5.0f, 35.0f) - 23.0f) < 1e-4f, "cold external biases up");
  // External warmer than the thermostat -> ask for less.
  CHECK(std::fabs(offset_setpoint(22.0f, 21.0f, 23.0f, 5.0f, 35.0f) - 20.0f) < 1e-4f, "warm external biases down");
  // Sensors agree -> behaves exactly like a plain setpoint.
  CHECK(std::fabs(offset_setpoint(21.0f, 21.0f, 21.0f, 5.0f, 35.0f) - 21.0f) < 1e-4f, "no bias when in agreement");
  CHECK(std::fabs(offset_setpoint(19.5f, 22.0f, 19.5f, 5.0f, 35.0f) - 22.0f) < 1e-4f, "agreement at any value");

  // Clamping stops a faulty sensor dragging the setpoint somewhere dangerous.
  CHECK(std::fabs(offset_setpoint(22.0f, 21.0f, -50.0f, 5.0f, 35.0f) - 35.0f) < 1e-4f, "clamped to max");
  CHECK(std::fabs(offset_setpoint(22.0f, 21.0f, 99.0f, 5.0f, 35.0f) - 5.0f) < 1e-4f, "clamped to min");

  // The bias tracks the *difference*, so absolute thermostat error cancels out.
  float a = offset_setpoint(25.0f, 21.0f, 20.0f, 5.0f, 35.0f);
  float b = offset_setpoint(18.0f, 21.0f, 13.0f, 5.0f, 35.0f);
  CHECK(std::fabs(a - b) < 1e-4f, "same 1C gap gives same correction regardless of offset");
}

static void test_should_write_setpoint() {
  std::printf("should_write_setpoint\n");
  const float deadband = 0.3f;
  const uint32_t interval = 600000;  // 10 min

  CHECK(should_write_setpoint(true, 21.0f, NAN, deadband, 0, 0, interval), "first write always allowed");
  CHECK(!should_write_setpoint(false, 21.0f, 21.0f, deadband, 1000000, 500000, interval), "identical suppressed");
  CHECK(!should_write_setpoint(false, 21.2f, 21.0f, deadband, 1000000, 500000, interval), "sub-deadband suppressed");
  CHECK(!should_write_setpoint(false, 22.0f, 21.0f, deadband, 600000, 500000, interval),
        "big change still rate-limited inside the interval");
  CHECK(should_write_setpoint(false, 22.0f, 21.0f, deadband, 1200000, 500000, interval),
        "big change allowed after the interval");
  CHECK(should_write_setpoint(false, 20.0f, 21.0f, deadband, 1200000, 500000, interval),
        "negative delta uses absolute value");

  // Protecting the AHC-9000's EEPROM is the point: a drifting sensor polled
  // every 10 s must not produce a write per poll.
  int writes = 0;
  float applied = 21.0f;
  uint32_t last = 0;
  for (uint32_t t = 10000; t <= 3600000; t += 10000) {
    float desired = 21.0f + 2.0f * (float) std::sin(t / 600000.0);
    if (should_write_setpoint(false, desired, applied, deadband, t, last, interval)) {
      applied = desired;
      last = t;
      writes++;
    }
  }
  CHECK(writes <= 7, "at most ~6 writes/hour despite 360 polls");
  std::printf("    (1 h of 10 s polling produced %d setpoint writes)\n", writes);
}

int main() {
  std::printf("=== Wavin zone logic tests ===\n");
  test_parse_channel_csv();
  test_mask_to_csv();
  test_offset_setpoint();
  test_should_write_setpoint();

  std::printf("==============================\n");
  if (g_failures == 0) {
    std::printf("ALL %d CHECKS PASSED\n", g_checks);
    return 0;
  }
  std::printf("%d of %d CHECKS FAILED\n", g_failures, g_checks);
  return 1;
}
