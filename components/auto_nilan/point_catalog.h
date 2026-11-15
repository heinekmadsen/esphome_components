#pragma once

#include <cstddef>
#include <cstdint>

namespace esphome {
namespace auto_nilan {

static constexpr size_t MAX_CATALOG_POINTS = 256;

enum Profile : uint8_t {
  PROFILE_UNKNOWN = 0,
  PROFILE_CTS602 = 1,
  PROFILE_CTS400 = 2,
};

enum RegType : uint8_t {
  REG_HOLDING = 0,
  REG_INPUT = 1,
};

enum Kind : uint8_t {
  KIND_SENSOR = 0,
  KIND_BINARY = 1,
  KIND_NUMBER = 2,
  KIND_SELECT = 3,
  KIND_SWITCH = 4,
  KIND_BUTTON = 5,
};

enum Cap : uint16_t {
  CAP_NONE = 0,
  CAP_CO2 = 1u << 0,
  CAP_RH = 1u << 1,
  CAP_VOC = 1u << 2,
  CAP_COOLING = 1u << 3,
  CAP_HEATING = 1u << 4,
  CAP_HOT_WATER = 1u << 5,
  CAP_COMPRESSOR = 1u << 6,
  CAP_BYPASS = 1u << 7,
  CAP_CENTRAL_HEAT = 1u << 8,
  CAP_PREHEAT = 1u << 9,
  CAP_EXPANSION = 1u << 10,
  CAP_PRESSURE = 1u << 11,
};

enum PointFlags : uint16_t {
  PF_NONE = 0,
  PF_WRITABLE = 1u << 0,
  PF_SIGNED = 1u << 1,
  PF_DIAG = 1u << 2,
  PF_DISABLED = 1u << 3,
  PF_OPTIONAL = 1u << 4,
  PF_TIME_HHMM = 1u << 5,
};

struct PointDef {
  const char *key;
  const char *name;
  uint16_t addr;
  uint8_t reg;       // RegType
  uint8_t kind;      // Kind
  int8_t scale_exp;  // human = raw * 10^scale_exp
  uint16_t flags;    // PointFlags
  int16_t min_value; // human units (number/switch)
  int16_t max_value; // human units (number/switch)
  const char *unit;
  const char *options; // '|' separated labels, or nullptr
  uint16_t req_cap;    // required capability bit, 0 = always
};

const PointDef *catalog_points(Profile profile);
size_t catalog_size(Profile profile);
const char *model_name(uint16_t model);
bool model_has_cooling(uint16_t model);
uint16_t caps_from_model(uint16_t model);
const char *profile_name(Profile profile);
const char *cap_name(uint16_t cap);
const char *kind_name(uint8_t kind);

}  // namespace auto_nilan
}  // namespace esphome
