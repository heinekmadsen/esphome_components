#include "auto_nilan.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace auto_nilan {

static const char *const TAG = "auto_nilan";

static const char *const UNIT_TEMP = "\xC2\xB0" "C";

static void json_escape(std::string &out, const char *text) {
  if (text == nullptr)
    return;
  for (const char *p = text; *p != '\0'; p++) {
    switch (*p) {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        if ((uint8_t) *p < 0x20) {
          char buf[8];
          snprintf(buf, sizeof(buf), "\\u%04x", (unsigned) (uint8_t) *p);
          out += buf;
        } else {
          out += *p;
        }
        break;
    }
  }
}

static void append_options(std::string &out, const char *options) {
  if (options == nullptr || options[0] == '\0') {
    out += "null";
    return;
  }
  out += '[';
  const char *start = options;
  bool first = true;
  while (true) {
    const char *bar = strchr(start, '|');
    const size_t len = bar == nullptr ? strlen(start) : (size_t) (bar - start);
    if (!first)
      out += ',';
    out += '"';
    std::string part(start, len);
    json_escape(out, part.c_str());
    out += '"';
    first = false;
    if (bar == nullptr)
      break;
    start = bar + 1;
  }
  out += ']';
}

static void append_value(std::string &out, float value, int8_t scale) {
  if (std::isnan(value)) {
    out += "null";
    return;
  }
  char buf[32];
  if (scale <= -2) {
    snprintf(buf, sizeof(buf), "%.2f", value);
  } else if (scale == -1) {
    snprintf(buf, sizeof(buf), "%.1f", value);
  } else {
    snprintf(buf, sizeof(buf), "%.0f", value);
  }
  out += buf;
}

static uint8_t option_count(const char *options) {
  if (options == nullptr || options[0] == '\0')
    return 0;
  uint8_t count = 1;
  for (const char *p = options; *p != '\0'; p++) {
    if (*p == '|')
      count++;
  }
  return count;
}

static void append_caps(std::string &out, uint16_t caps) {
  out += '[';
  bool first = true;
  for (uint8_t bit = 0; bit < 12; bit++) {
    const uint16_t cap = (uint16_t) (1u << bit);
    if ((caps & cap) == 0)
      continue;
    const char *name = cap_name(cap);
    if (name == nullptr || name[0] == '\0')
      continue;
    if (!first)
      out += ',';
    out += '"';
    out += name;
    out += '"';
    first = false;
  }
  out += ']';
}

void AutoNilan::setup() {
  modbus_setup_gpio(this->flow_control_pin_);

  uint32_t baud = 19200;
  if (this->parent_ != nullptr) {
    const uint32_t parent_baud = this->parent_->get_baud_rate();
    if (parent_baud != 0)
      baud = parent_baud;
  }
  modbus_compute_timing(baud, this->timing_);

  ESP_LOGCONFIG(TAG,
                "Nilan auto-detect hub (addr=%u baud=%u pre=%uus post=%uus inter=%uus gap=%uus timeout=%ums "
                "flow_control=%s)",
                (unsigned) this->address_, (unsigned) baud, (unsigned) this->timing_.pre_tx_us,
                (unsigned) this->timing_.post_tx_us, (unsigned) this->timing_.inter_frame_us,
                (unsigned) this->timing_.rx_gap_us, (unsigned) this->timing_.rx_timeout_ms,
                this->flow_control_pin_ != nullptr ? "external pin" : "uart bus");
}

void AutoNilan::dump_config() {
  ESP_LOGCONFIG(TAG, "Nilan auto-detect:");
  ESP_LOGCONFIG(TAG, "  Address: %u", (unsigned) this->address_);
  ESP_LOGCONFIG(TAG, "  Profile: %s", profile_name(this->profile_));
  ESP_LOGCONFIG(TAG, "  Model: %u (%s)", (unsigned) this->model_, model_name(this->model_));
  ESP_LOGCONFIG(TAG, "  Commissioned: %s", this->commissioned_ ? "yes" : "no");
  ESP_LOGCONFIG(TAG, "  Points: %u", (unsigned) this->points_.size());
  ESP_LOGCONFIG(TAG, "  Receive timeout: %ums", (unsigned) this->timing_.rx_timeout_ms);
}

void AutoNilan::set_error_(const std::string &error) {
  if (this->last_error_ != error) {
    this->last_error_ = error;
    if (!error.empty())
      ESP_LOGW(TAG, "%s", error.c_str());
  }
}

bool AutoNilan::read_raw_(uint8_t reg_type, uint16_t address, uint16_t &value) {
  std::vector<uint16_t> out;
  const uint8_t fc = reg_type == REG_HOLDING ? 0x03 : 0x04;
  if (!modbus_read_registers(*this, this->flow_control_pin_, this->address_, fc, address, 1, out, this->timing_,
                             this->stats_))
    return false;
  if (out.empty())
    return false;
  value = out[0];
  return true;
}

bool AutoNilan::read_block_(uint8_t reg_type, uint16_t address, uint16_t count, std::vector<uint16_t> &out) {
  const uint8_t fc = reg_type == REG_HOLDING ? 0x03 : 0x04;
  return modbus_read_registers(*this, this->flow_control_pin_, this->address_, fc, address, count, out, this->timing_,
                               this->stats_);
}

bool AutoNilan::write_raw_(uint16_t address, uint16_t value) {
  if (this->profile_ == PROFILE_CTS602)
    return modbus_write_registers(*this, this->flow_control_pin_, this->address_, address, &value, 1,
                                  this->timing_, this->stats_);
  return modbus_write_register(*this, this->flow_control_pin_, this->address_, address, value, this->timing_,
                               this->stats_);
}

bool AutoNilan::detect_profile_() {
  uint16_t bus_version = 0;
  uint16_t model = 0;
  const bool bus_ok = this->read_raw_(REG_INPUT, 0, bus_version);
  const bool model_ok = this->read_raw_(REG_HOLDING, 1000, model);

  if (bus_ok && model_ok && bus_version > 0 && bus_version < 200 && model <= 40) {
    this->profile_ = PROFILE_CTS602;
    this->model_ = model;
    this->caps_ = caps_from_model(model);
    return true;
  }

  uint16_t extract_air = 0;
  uint16_t room_setpoint = 0;
  const bool extract_ok = this->read_raw_(REG_INPUT, 24, extract_air);
  const bool room_ok = this->read_raw_(REG_HOLDING, 37, room_setpoint);
  if (extract_ok && room_ok && extract_air <= 1000 && room_setpoint <= 400) {
    this->profile_ = PROFILE_CTS400;
    this->model_ = 0;
    this->caps_ = CAP_HEATING | CAP_RH;
    return true;
  }
  return false;
}

void AutoNilan::probe_capabilities_() {
  uint16_t value = 0;
  if (this->profile_ == PROFILE_CTS602) {
    if (this->read_raw_(REG_INPUT, 3003, value) && value == 1)
      this->caps_ |= CAP_CO2;
    if (this->read_raw_(REG_HOLDING, 4108, value) && value >= 1)
      this->caps_ |= CAP_RH;
    if (this->read_raw_(REG_INPUT, 1700, value) && value > 0)
      this->caps_ |= CAP_HOT_WATER;
    if (this->read_raw_(REG_INPUT, 1500, value) && value > 0)
      this->caps_ |= CAP_COMPRESSOR;
    if (this->read_raw_(REG_INPUT, 3000, value))
      this->caps_ |= CAP_BYPASS;
    if (this->read_raw_(REG_INPUT, 1800, value))
      this->caps_ |= CAP_CENTRAL_HEAT;
    if (this->read_raw_(REG_INPUT, 2100, value))
      this->caps_ |= CAP_PREHEAT;
    if (this->read_raw_(REG_INPUT, 3101, value) && value == 12)
      this->caps_ |= CAP_EXPANSION;
    if (this->read_raw_(REG_INPUT, 2200, value))
      this->caps_ |= CAP_PRESSURE;
  } else if (this->profile_ == PROFILE_CTS400) {
    this->caps_ = CAP_RH;
    if (this->read_raw_(REG_HOLDING, 48, value)) {
      if (value == 1)
        this->caps_ |= CAP_VOC;
      else if (value == 2)
        this->caps_ |= CAP_CO2;
    }
    if (this->read_raw_(REG_INPUT, 23, value))
      this->caps_ |= CAP_BYPASS;
    if (this->read_raw_(REG_HOLDING, 57, value))
      this->caps_ |= CAP_HEATING;
  }
}

void AutoNilan::build_point_list_() {
  this->points_.clear();
  const PointDef *defs = catalog_points(this->profile_);
  const size_t count = catalog_size(this->profile_);
  this->points_.reserve(count);
  for (size_t i = 0; i < count; i++) {
    const PointDef &def = defs[i];
    if (def.req_cap != 0 && (this->caps_ & def.req_cap) == 0)
      continue;
    RuntimePoint point;
    point.def = &def;
    point.present = true;
    this->points_.push_back(point);
  }
}

void AutoNilan::probe_optional_points_() {
  std::vector<size_t> order;
  order.reserve(this->points_.size());
  for (size_t i = 0; i < this->points_.size(); i++) {
    if ((this->points_[i].def->flags & PF_OPTIONAL) != 0)
      order.push_back(i);
  }
  std::sort(order.begin(), order.end(), [this](size_t a, size_t b) {
    const PointDef *pa = this->points_[a].def;
    const PointDef *pb = this->points_[b].def;
    if (pa->reg != pb->reg)
      return pa->reg < pb->reg;
    return pa->addr < pb->addr;
  });

  for (size_t i = 0; i < order.size();) {
    const PointDef *first = this->points_[order[i]].def;
    const uint8_t reg = first->reg;
    const uint16_t start = first->addr;
    size_t last = i;
    uint16_t count = 1;
    while (last + 1 < order.size() && count < 16) {
      const PointDef *a = this->points_[order[last]].def;
      const PointDef *b = this->points_[order[last + 1]].def;
      if (b->reg != reg || b->addr != (uint16_t) (a->addr + 1))
        break;
      last++;
      count++;
    }

    std::vector<uint16_t> raw;
    const bool ok = this->read_block_(reg, start, count, raw) && raw.size() >= count;
    for (size_t k = i; k <= last; k++) {
      RuntimePoint &point = this->points_[order[k]];
      if (!ok) {
        point.present = false;
        continue;
      }
      const uint16_t value = raw[(size_t) (point.def->addr - start)];
      if (value == 0xFFFF || value == 0x8000) {
        point.present = false;
        continue;
      }
      point.value = decode_(*point.def, value);
      point.fresh = true;
      point.present = plausible_(*point.def, point.value);
    }
    i = last + 1;
  }
}

bool AutoNilan::poll_once_() {
  std::vector<size_t> order;
  order.reserve(this->points_.size());
  for (size_t i = 0; i < this->points_.size(); i++) {
    const RuntimePoint &point = this->points_[i];
    if (!point.present || point.def->kind == KIND_BUTTON)
      continue;
    order.push_back(i);
  }
  std::sort(order.begin(), order.end(), [this](size_t a, size_t b) {
    const PointDef *pa = this->points_[a].def;
    const PointDef *pb = this->points_[b].def;
    if (pa->reg != pb->reg)
      return pa->reg < pb->reg;
    return pa->addr < pb->addr;
  });

  bool any_ok = false;
  for (size_t i = 0; i < order.size();) {
    const PointDef *first = this->points_[order[i]].def;
    const uint8_t reg = first->reg;
    const uint16_t start = first->addr;
    size_t last = i;
    uint16_t count = 1;
    while (last + 1 < order.size() && count < 16) {
      const PointDef *a = this->points_[order[last]].def;
      const PointDef *b = this->points_[order[last + 1]].def;
      if (b->reg != reg || b->addr != (uint16_t) (a->addr + 1))
        break;
      last++;
      count++;
    }

    std::vector<uint16_t> raw;
    if (this->read_block_(reg, start, count, raw) && raw.size() >= count) {
      any_ok = true;
      for (size_t k = i; k <= last; k++) {
        RuntimePoint &point = this->points_[order[k]];
        point.value = decode_(*point.def, raw[(size_t) (point.def->addr - start)]);
        point.fresh = true;
      }
    } else if (count > 1) {
      // Some controllers reject a whole block when a single register in it is
      // not implemented. Read the registers individually so the ones that do
      // exist keep their values; a hard "illegal data address" means the point
      // is not present on this model, so drop it from the topology.
      ESP_LOGD(TAG, "Poll block failed reg=%s addr=%u count=%u, splitting",
               reg == REG_HOLDING ? "holding" : "input", (unsigned) start, (unsigned) count);
      for (size_t k = i; k <= last; k++) {
        RuntimePoint &point = this->points_[order[k]];
        std::vector<uint16_t> single;
        if (this->read_block_(reg, point.def->addr, 1, single) && single.size() >= 1) {
          any_ok = true;
          point.value = decode_(*point.def, single[0]);
          point.fresh = true;
        } else if (this->stats_.last_exception == 0x02) {
          ESP_LOGW(TAG, "Point '%s' (%s %u) reports illegal address, removing", point.def->key,
                   reg == REG_HOLDING ? "HR" : "IR", (unsigned) point.def->addr);
          point.present = false;
          this->topology_dirty_ = true;
        }
      }
    } else {
      ESP_LOGD(TAG, "Poll block failed reg=%s addr=%u count=%u", reg == REG_HOLDING ? "holding" : "input",
               (unsigned) start, (unsigned) count);
      if (this->stats_.last_exception == 0x02) {
        ESP_LOGW(TAG, "Point '%s' (%s %u) reports illegal address, removing", this->points_[order[i]].def->key,
                 reg == REG_HOLDING ? "HR" : "IR", (unsigned) start);
        this->points_[order[i]].present = false;
        this->topology_dirty_ = true;
      }
    }
    i = last + 1;
  }

  this->online_ = any_ok;
  if (any_ok) {
    this->last_success_ms_ = millis();
    this->last_error_.clear();
  } else if (this->commissioned_) {
    this->set_error_("all Modbus reads failed");
  } else {
    this->set_error_("controller not responding");
  }
  this->poll_count_++;
  return any_ok;
}

void AutoNilan::commission() {
  this->commission_attempts_++;
  this->last_commission_attempt_ms_ = millis();
  this->profile_ = PROFILE_UNKNOWN;
  this->model_ = 0;
  this->caps_ = 0;
  this->commissioned_ = false;
  this->points_.clear();
  this->topology_json_.clear();

  ESP_LOGI(TAG, "Commissioning (attempt %u)", (unsigned) this->commission_attempts_);
  if (!this->detect_profile_()) {
    this->set_error_("no Nilan controller detected");
    this->build_status_(this->status_json_);
    if (this->status_sensor_ != nullptr)
      this->status_sensor_->publish_state(this->status_json_);
    return;
  }

  this->probe_capabilities_();
  this->build_point_list_();
  this->probe_optional_points_();
  this->commissioned_ = true;
  this->last_error_.clear();

  ESP_LOGI(TAG, "Detected %s, model %u (%s), caps 0x%04X, %u points", profile_name(this->profile_),
           (unsigned) this->model_, model_name(this->model_), (unsigned) this->caps_,
           (unsigned) this->points_.size());

  this->build_topology_(this->points_, this->profile_, this->model_, this->caps_, this->topology_json_);
  this->topology_dirty_ = true;
  this->poll_once_();
  this->publish_payloads_();
}

void AutoNilan::update() {
  const uint32_t start = millis();
  if (!this->commissioned_) {
    const bool due = this->commission_attempts_ == 0 ||
                     (uint32_t) (millis() - this->last_commission_attempt_ms_) >= 60000;
    if (due) {
      this->commission();
    } else {
      this->build_status_(this->status_json_);
      if (this->status_sensor_ != nullptr)
        this->status_sensor_->publish_state(this->status_json_);
    }
    this->poll_duration_ms_ = millis() - start;
    return;
  }

  this->last_poll_ms_ = millis();
  this->poll_once_();
  this->poll_duration_ms_ = millis() - start;
  this->publish_payloads_();
}

void AutoNilan::publish_payloads_() {
  this->build_values_(this->values_json_);
  this->build_status_(this->status_json_);
  if (this->topology_dirty_ && this->commissioned_) {
    this->build_topology_(this->points_, this->profile_, this->model_, this->caps_, this->topology_json_);
    if (this->topology_sensor_ != nullptr && !this->topology_json_.empty())
      this->topology_sensor_->publish_state(this->topology_json_);
    this->topology_dirty_ = false;
  }
  if (this->values_sensor_ != nullptr)
    this->values_sensor_->publish_state(this->values_json_);
  if (this->status_sensor_ != nullptr)
    this->status_sensor_->publish_state(this->status_json_);
}

AutoNilan::RuntimePoint *AutoNilan::find_point_(const std::string &key) {
  for (auto &point : this->points_) {
    if (key == point.def->key)
      return &point;
  }
  return nullptr;
}

float AutoNilan::decode_(const PointDef &def, uint16_t raw) {
  int32_t value = raw;
  if ((def.flags & PF_SIGNED) != 0)
    value = (int16_t) raw;
  return (float) value * powf(10.0f, (float) def.scale_exp);
}

uint16_t AutoNilan::encode_(const PointDef &def, float human) {
  const double scaled = (double) human / pow(10.0, (double) def.scale_exp);
  int32_t raw = (int32_t) lround(scaled);
  if ((def.flags & PF_SIGNED) != 0) {
    if (raw < -32768)
      raw = -32768;
    if (raw > 32767)
      raw = 32767;
    return (uint16_t) (int16_t) raw;
  }
  if (raw < 0)
    raw = 0;
  if (raw > 65535)
    raw = 65535;
  return (uint16_t) raw;
}

bool AutoNilan::plausible_(const PointDef &def, float human) {
  if (std::isnan(human))
    return false;
  if (def.kind == KIND_BINARY)
    return human == 0.0f || human == 1.0f;
  if (def.kind != KIND_SENSOR && def.kind != KIND_NUMBER)
    return true;
  if (def.unit != nullptr && strcmp(def.unit, UNIT_TEMP) == 0)
    return human > -60.0f && human < 150.0f;
  if (def.unit != nullptr && strcmp(def.unit, "%") == 0)
    return human > -1.0f && human < 101.0f;
  if (def.unit != nullptr && strcmp(def.unit, "ppm") == 0)
    return human >= 0.0f && human < 10000.0f;
  return human > -100000.0f && human < 100000.0f;
}

void AutoNilan::build_topology_(const std::vector<RuntimePoint> &points, Profile profile, uint16_t model,
                                uint16_t caps, std::string &out) {
  out.clear();
  out.reserve(16384);
  out += "{\"version\":1,\"profile\":\"";
  out += profile_name(profile);
  out += "\",\"model\":";
  out += std::to_string(model);
  out += ",\"model_name\":\"";
  json_escape(out, model_name(model));
  out += "\",\"caps\":";
  append_caps(out, caps);
  out += ",\"points\":[";

  bool first = true;
  for (const auto &point : points) {
    if (!point.present)
      continue;
    const PointDef &def = *point.def;
    if (!first)
      out += ',';
    first = false;

    out += "{\"key\":\"";
    json_escape(out, def.key);
    out += "\",\"name\":\"";
    json_escape(out, def.name);
    out += "\",\"kind\":\"";
    out += kind_name(def.kind);
    out += "\",\"reg\":\"";
    out += def.reg == REG_HOLDING ? "holding" : "input";
    out += "\",\"addr\":";
    out += std::to_string(def.addr);
    out += ",\"scale\":";
    out += std::to_string((int) def.scale_exp);
    out += ",\"signed\":";
    out += (def.flags & PF_SIGNED) != 0 ? "true" : "false";

    if (def.kind == KIND_SENSOR || def.kind == KIND_BINARY) {
      out += ",\"min\":null,\"max\":null";
    } else {
      out += ",\"min\":";
      out += std::to_string((int) def.min_value);
      out += ",\"max\":";
      out += std::to_string((int) def.max_value);
    }

    out += ",\"unit\":";
    if (def.unit == nullptr || def.unit[0] == '\0') {
      out += "null";
    } else {
      out += '"';
      json_escape(out, def.unit);
      out += '"';
    }
    out += ",\"options\":";
    append_options(out, def.options);
    out += ",\"writable\":";
    out += (def.flags & PF_WRITABLE) != 0 ? "true" : "false";
    out += ",\"diagnostic\":";
    out += (def.flags & PF_DIAG) != 0 ? "true" : "false";
    out += ",\"disabled\":";
    out += (def.flags & PF_DISABLED) != 0 ? "true" : "false";
    out += ",\"cap\":";
    const char *cap = def.req_cap != 0 ? cap_name(def.req_cap) : "";
    if (cap == nullptr || cap[0] == '\0') {
      out += "null";
    } else {
      out += '"';
      out += cap;
      out += '"';
    }
    out += '}';
  }
  out += "]}";
}

void AutoNilan::build_values_(std::string &out) {
  out.clear();
  out.reserve(8192);
  out += "{\"ts\":";
  out += std::to_string(millis() / 1000);
  out += ",\"online\":";
  out += this->online_ ? "true" : "false";
  out += ",\"profile\":\"";
  out += profile_name(this->profile_);
  out += "\",\"values\":{";
  bool first = true;
  for (const auto &point : this->points_) {
    if (!point.present || std::isnan(point.value))
      continue;
    if (!first)
      out += ',';
    first = false;
    out += '"';
    out += point.def->key;
    out += "\":";
    append_value(out, point.value, point.def->scale_exp);
  }
  out += "}}";
}

void AutoNilan::build_status_(std::string &out) {
  out.clear();
  out.reserve(1024);
  out += "{\"online\":";
  out += this->online_ ? "true" : "false";
  out += ",\"commissioned\":";
  out += this->commissioned_ ? "true" : "false";
  out += ",\"profile\":\"";
  out += profile_name(this->profile_);
  out += "\",\"model\":";
  out += std::to_string(this->model_);
  out += ",\"model_name\":\"";
  json_escape(out, model_name(this->model_));
  out += "\",\"caps\":";
  append_caps(out, this->caps_);
  out += ",\"points\":";
  out += std::to_string(this->points_.size());
  size_t present = 0;
  for (const auto &point : this->points_) {
    if (point.present)
      present++;
  }
  out += ",\"present\":";
  out += std::to_string(present);
  out += ",\"poll_ms\":";
  out += std::to_string(this->poll_duration_ms_);
  out += ",\"polls\":";
  out += std::to_string(this->poll_count_);
  out += ",\"uptime\":";
  out += std::to_string(millis() / 1000);
  out += ",\"tx\":";
  out += std::to_string(this->stats_.transactions);
  out += ",\"retries\":";
  out += std::to_string(this->stats_.retries);
  out += ",\"failures\":";
  out += std::to_string(this->stats_.failures);
  out += ",\"timeouts\":";
  out += std::to_string(this->stats_.timeouts);
  out += ",\"crc_errors\":";
  out += std::to_string(this->stats_.crc_errors);
  out += ",\"exceptions\":";
  out += std::to_string(this->stats_.exceptions);
  out += ",\"topology_len\":";
  out += std::to_string(this->topology_json_.size());
  out += ",\"values_len\":";
  out += std::to_string(this->values_json_.size());
  out += ",\"last_error\":\"";
  json_escape(out, this->last_error_.c_str());
  out += "\"}";
}

bool AutoNilan::write_point(const std::string &key, float value, std::string *error) {
  RuntimePoint *point = this->find_point_(key);
  if (point == nullptr || !point->present) {
    if (error != nullptr)
      *error = "unknown or unavailable point: " + key;
    return false;
  }
  if ((point->def->flags & PF_WRITABLE) == 0) {
    if (error != nullptr)
      *error = "point is read-only: " + key;
    return false;
  }
  if (point->def->reg != REG_HOLDING) {
    if (error != nullptr)
      *error = "point is not a holding register: " + key;
    return false;
  }

  if (point->def->kind == KIND_SELECT) {
    const uint8_t options = option_count(point->def->options);
    if (options > 0 && value > (float) (options - 1))
      value = (float) (options - 1);
  }
  if (value < (float) point->def->min_value)
    value = (float) point->def->min_value;
  if (value > (float) point->def->max_value)
    value = (float) point->def->max_value;

  const uint16_t raw = encode_(*point->def, value);
  ESP_LOGD(TAG, "Write '%s' reg %u value %.2f (raw %u)", key.c_str(), (unsigned) point->def->addr, value,
           (unsigned) raw);
  if (!this->write_raw_(point->def->addr, raw)) {
    if (error != nullptr)
      *error = "Modbus write failed for " + key;
    return false;
  }
  point->value = value;
  point->fresh = true;
  if (error != nullptr)
    error->clear();
  this->publish_payloads_();
  return true;
}

bool AutoNilan::press_point(const std::string &key, std::string *error) {
  RuntimePoint *point = this->find_point_(key);
  if (point == nullptr || !point->present) {
    if (error != nullptr)
      *error = "unknown or unavailable point: " + key;
    return false;
  }
  return this->write_point(key, (float) point->def->max_value, error);
}

void AutoNilan::reset_alarm() {
  if (this->profile_ == PROFILE_CTS602) {
    if (!this->write_raw_(400, 255))
      ESP_LOGW(TAG, "Alarm reset failed");
  } else if (this->profile_ == PROFILE_CTS400) {
    if (!this->write_raw_(30, 1))
      ESP_LOGW(TAG, "Alarm reset failed");
  }
}

}  // namespace auto_nilan
}  // namespace esphome
