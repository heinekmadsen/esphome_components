#include "sentiov2.h"

#include "esphome/core/log.h"

#ifdef USE_WIFI
#include "esphome/components/wifi/wifi_component.h"
#endif

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace esphome {
namespace sentiov2 {

static const char *const TAG = "sentiov2";

static constexpr uint8_t FC_READ_DISCRETE = 0x02;
static constexpr uint8_t FC_READ_HOLDING = 0x03;
static constexpr uint8_t FC_READ_INPUT = 0x04;
static constexpr uint8_t FC_WRITE_SINGLE = 0x06;

static constexpr uint16_t INVALID_FP100 = 0x7FFF;
static constexpr uint16_t ROOM_STRIDE = 100;

static bool is_valid_raw(uint16_t raw) { return raw != 0 && raw != 0xFFFF && raw != INVALID_FP100; }

void SentioV2::setup() {
  if (this->flow_control_pin_ != nullptr) {
    this->flow_control_pin_->setup();
    this->flow_control_pin_->digital_write(false);
  }

  uint32_t baud = 19200;
  if (this->parent_ != nullptr) {
    uint32_t parent_baud = this->parent_->get_baud_rate();
    if (parent_baud != 0)
      baud = parent_baud;
  }
  const uint32_t frame_us = (11UL * 1000000UL + baud - 1) / baud;
  this->pre_tx_delay_us_ = std::max<uint32_t>(frame_us / 4, 100);
  this->post_tx_guard_us_ = std::max<uint32_t>(frame_us / 2, 200);
  this->rx_gap_us_ = std::max<uint32_t>(frame_us * 4, 2000);

  ESP_LOGCONFIG(TAG, "Sentio V2 initialised: address=%u, baud=%u, timeout=%ums",
                this->address_, (unsigned) baud, (unsigned) this->receive_timeout_ms_);
}

void SentioV2::dump_config() {
  ESP_LOGCONFIG(TAG, "Wavin Sentio V2:");
  ESP_LOGCONFIG(TAG, "  Modbus address: %u", this->address_);
  ESP_LOGCONFIG(TAG, "  Flow control pin: %s", this->flow_control_pin_ != nullptr ? "yes" : "no");
  ESP_LOGCONFIG(TAG, "  Receive timeout: %u ms", (unsigned) this->receive_timeout_ms_);
}

void SentioV2::update() {
  this->read_vacation_();
  for (uint8_t room = 1; room <= SENTIO_MAX_ROOMS; room++)
    this->read_room_(room);
  this->apply_sync_();
  this->publish_topology();
}

uint8_t SentioV2::first_control_room_() const {
  for (uint8_t i = 0; i < SENTIO_MAX_ROOMS; i++) {
    const auto &room = this->rooms_[i];
    if (room.present && !room.dummy && room.has_control)
      return i + 1;
  }
  return 0;
}

void SentioV2::apply_sync_() {
  if (!this->sync_all_rooms_)
    return;
  const uint8_t master = this->first_control_room_();
  if (master == 0)
    return;
  const float target = this->rooms_[master - 1].target;
  if (std::isnan(target))
    return;
  for (uint8_t room = 1; room <= SENTIO_MAX_ROOMS; room++) {
    if (room == master)
      continue;
    auto &state = this->rooms_[room - 1];
    if (!state.present || state.dummy || !state.has_control || std::isnan(state.target))
      continue;
    if (std::fabs(state.target - target) >= 0.25f)
      this->zone_set_target(room, target);
  }
}

void SentioV2::read_vacation_() {
  std::vector<uint16_t> values;
  if (this->read_holding_registers_(27, 1, values) && !values.empty())
    this->vacation_ = values[0] != 0;
}

void SentioV2::read_room_(uint8_t room) {
  if (room < 1 || room > SENTIO_MAX_ROOMS)
    return;

  auto &state = this->rooms_[room - 1];
  const uint16_t base = (uint16_t) room * ROOM_STRIDE;

  std::vector<uint16_t> input;
  std::vector<uint16_t> room_type_values;
  std::vector<uint16_t> name_values;
  std::vector<uint16_t> setpoint_values;
  std::vector<uint16_t> lock_values;
  std::vector<uint16_t> mode_values;
  std::vector<uint16_t> heat_state_values;
  std::vector<bool> discrete;

  const bool input_ok = this->read_input_registers_(base + 101, 7, input);
  const bool setpoint_ok = this->read_holding_registers_(base + 19, 1, setpoint_values);

  // A combined Sentio room can have its thermostat in one room object and its
  // sensors in the next. A room with neither a writable setpoint nor sensor data
  // is not configured and is skipped.
  if (!input_ok && !setpoint_ok)
    return;

  const bool room_type_ok =
      input_ok && this->read_input_registers_(base + 127, 1, room_type_values);
  const bool name_ok = this->read_holding_registers_(base + 1, 16, name_values);
  const bool lock_ok = this->read_holding_registers_(base + 20, 1, lock_values);
  const bool mode_ok = this->read_holding_registers_(base + 17, 2, mode_values);
  const bool heat_state_ok =
      input_ok && this->read_input_registers_(base + 117, 4, heat_state_values);
  const bool discrete_ok =
      input_ok && this->read_discrete_inputs_(base + 101, 4, discrete);

  if (input_ok && input.size() >= 7) {
    const uint16_t room_type =
        room_type_ok && !room_type_values.empty() ? room_type_values[0] : 0xFF;
    if (room_type == 1)
      state.dummy = true;

    state.state = (uint8_t) (input[1] & 0xFF);
    if (heat_state_ok && heat_state_values.size() >= 4) {
      state.radiator_state = (uint8_t) (heat_state_values[0] & 0xFF);
      state.ufh_state = (uint8_t) (heat_state_values[1] & 0xFF);
      state.drying_state = (uint8_t) (heat_state_values[2] & 0xFF);
      state.thermal_state = (uint8_t) (heat_state_values[3] & 0xFF);
      if (state.state <= 1) {
        for (uint8_t candidate : {state.ufh_state, state.radiator_state,
                                  state.thermal_state, state.drying_state}) {
          if (candidate >= 2) {
            state.state = candidate;
            break;
          }
        }
      }
    }

    float value;
    if (this->parse_temperature_(input[3], value)) {
      state.current = value;
      state.has_sensor = true;
      state.last_good_ms = millis();
    }
    if (this->parse_temperature_(input[4], value))
      state.floor = value;
    else if (input[4] == INVALID_FP100 || input[4] == 0)
      state.floor = NAN;
    if (this->parse_humidity_(input[5], value))
      state.humidity = value;
    else if (input[5] == INVALID_FP100 || input[5] == 0)
      state.humidity = NAN;
    if (this->parse_temperature_(input[6], value))
      state.dewpoint = value;
    else if (input[6] == INVALID_FP100 || input[6] == 0)
      state.dewpoint = NAN;

    float desired;
    if (this->parse_float_(input[0], 5.0f, 40.0f, desired) && !state.off)
      state.target = desired;
  }

  if (setpoint_ok && !setpoint_values.empty()) {
    state.has_control = true;
    const uint16_t setpoint = setpoint_values[0];
    if (setpoint == INVALID_FP100 || setpoint == 0xFFFF) {
      state.off = true;
      state.target = NAN;
    } else {
      float target;
      if (this->parse_float_(setpoint, 5.0f, 40.0f, target)) {
        state.target = target;
        state.off = false;
      }
    }
  }

  if (name_ok && name_values.size() >= 16) {
    const std::string name = this->parse_name_(name_values);
    if (!name.empty()) {
      state.name = name;
      state.name_valid = true;
    }
  }

  if (lock_ok && !lock_values.empty()) {
    const uint16_t lock = lock_values[0];
    if (lock == 8 || lock == 16 || lock == 32)
      state.lock_value = (uint8_t) lock;
  }

  if (mode_ok && mode_values.size() >= 2) {
    state.room_mode = (uint8_t) (mode_values[0] & 0xFF);
    state.override_mode = (uint8_t) (mode_values[1] & 0xFF);
  }

  if (discrete_ok && discrete.size() >= 4) {
    state.warning = discrete[0];
    state.error = discrete[1];
    state.battery_low = discrete[2];
  }

  if (!state.name_valid)
    state.name = "Room " + std::to_string(room);

  state.present = state.present || state.has_control || state.has_sensor;
}

bool SentioV2::parse_float_(uint16_t raw, float minimum, float maximum, float &out) const {
  if (!is_valid_raw(raw))
    return false;
  const float value = ((int16_t) raw) / 100.0f;
  if (std::isnan(value) || std::isinf(value) || value < minimum || value > maximum)
    return false;
  out = value;
  return true;
}

bool SentioV2::parse_temperature_(uint16_t raw, float &out) const {
  return this->parse_float_(raw, -20.0f, 80.0f, out);
}

bool SentioV2::parse_humidity_(uint16_t raw, float &out) const {
  return this->parse_float_(raw, 1.0f, 100.0f, out);
}

std::string SentioV2::parse_name_(const std::vector<uint16_t> &registers) const {
  std::string raw;
  raw.reserve(32);
  for (size_t i = 0; i < 16 && i < registers.size(); i++) {
    raw.push_back((char) (registers[i] >> 8));
    raw.push_back((char) (registers[i] & 0xFF));
  }

  size_t start = 0;
  if (raw.size() >= 2 && (uint8_t) raw[0] == 0 && (uint8_t) raw[1] <= 32)
    start = 2;

  std::string name;
  for (size_t i = start; i < raw.size(); i++) {
    const uint8_t c = (uint8_t) raw[i];
    if (c == 0)
      break;
    if (c < 0x20 && c != '\t')
      continue;
    name.push_back((char) c);
  }
  while (!name.empty() && (name.back() == ' ' || name.back() == '\r' || name.back() == '\n'))
    name.pop_back();
  return name;
}

std::string SentioV2::action_name_(uint8_t state) const {
  switch (state) {
    case 1:
      return "idle";
    case 2:
      return "heating";
    case 3:
      return "cooling";
    case 4:
      return "blocked_heating";
    case 5:
      return "blocked_cooling";
    default:
      return "unknown";
  }
}

void SentioV2::append_json_string_(std::string &out, const std::string &value) const {
  out.push_back('"');
  for (const unsigned char c : value) {
    switch (c) {
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
        if (c < 0x20)
          out.push_back(' ');
        else
          out.push_back((char) c);
        break;
    }
  }
  out.push_back('"');
}

void SentioV2::append_json_number_(std::string &out, float value) const {
  if (std::isnan(value) || std::isinf(value)) {
    out += "null";
    return;
  }
  char buffer[24];
  snprintf(buffer, sizeof(buffer), "%.2f", value);
  out += buffer;
}

void SentioV2::publish_topology() {
  int rssi = -127;
#ifdef USE_WIFI
  if (wifi::global_wifi_component != nullptr)
    rssi = wifi::global_wifi_component->wifi_rssi();
#endif

  // Sentio can represent a combined room as a sensor-only room immediately
  // before its control room. Probe that shape from the topology instead of
  // assuming that room numbers equal output/channel numbers.
  bool sensor_paired[SENTIO_MAX_ROOMS] = {false};
  bool control_paired[SENTIO_MAX_ROOMS] = {false};
  int sensor_for_control[SENTIO_MAX_ROOMS];
  for (uint8_t i = 0; i < SENTIO_MAX_ROOMS; i++)
    sensor_for_control[i] = -1;

  // Prefer sensor-room -> next control-room. This is the shape used when a
  // combined room's sensor is exposed in the room object just before its
  // thermostat.
  for (uint8_t i = 0; i < SENTIO_MAX_ROOMS; i++) {
    const auto &room = this->rooms_[i];
    if (!room.present || room.dummy || !room.has_sensor || room.has_control)
      continue;
    if (i + 1 < SENTIO_MAX_ROOMS) {
      const auto &next = this->rooms_[i + 1];
      if (next.present && !next.dummy && next.has_control && !next.has_sensor &&
          !control_paired[i + 1]) {
        sensor_paired[i] = true;
        control_paired[i + 1] = true;
        sensor_for_control[i + 1] = i;
      }
    }
  }

  // Fallback: sensor-room -> previous control-room.
  for (uint8_t i = 0; i < SENTIO_MAX_ROOMS; i++) {
    const auto &room = this->rooms_[i];
    if (!room.present || room.dummy || !room.has_sensor || room.has_control ||
        sensor_paired[i])
      continue;
    if (i > 0) {
      const auto &prev = this->rooms_[i - 1];
      if (prev.present && !prev.dummy && prev.has_control && !prev.has_sensor &&
          !control_paired[i - 1]) {
        sensor_paired[i] = true;
        control_paired[i - 1] = true;
        sensor_for_control[i - 1] = i;
      }
    }
  }

  std::string out;
  out.reserve(4096);
  out += "{\"version\":1,\"model\":\"sentio\",\"rssi\":";
  out += std::to_string(rssi);
  out += ",\"vacation\":";
  out += this->vacation_ ? "true" : "false";
  const uint8_t master = this->first_control_room_();
  out += ",\"sync\":";
  out += this->sync_all_rooms_ ? "true" : "false";
  out += ",\"master_zone\":";
  out += std::to_string(master);
  out += ",\"master_name\":";
  this->append_json_string_(out, master ? this->rooms_[master - 1].name : "");
  out += ",\"zones\":[";

  bool first = true;
  for (uint8_t i = 0; i < SENTIO_MAX_ROOMS; i++) {
    const auto &control = this->rooms_[i];
    if (!control.present || control.dummy || !control.has_control)
      continue;

    const SentioRoom *sensor = &control;
    uint8_t sensor_room = i + 1;
    if (sensor_for_control[i] >= 0) {
      sensor = &this->rooms_[sensor_for_control[i]];
      sensor_room = (uint8_t) (sensor_for_control[i] + 1);
    }

    if (!first)
      out.push_back(',');
    first = false;

    out += "{\"id\":";
    out += std::to_string(i + 1);
    out += ",\"name\":";
    this->append_json_string_(out, control.name);
    out += ",\"enabled\":true";
    out += ",\"dummy\":";
    out += control.dummy ? "true" : "false";
    out += ",\"sensor_room\":";
    out += std::to_string(sensor_room);
    out += ",\"sensor_target\":";
    this->append_json_number_(out, sensor->target);
    out += ",\"current\":";
    this->append_json_number_(out, sensor->current);
    out += ",\"target\":";
    this->append_json_number_(out, control.target);
    out += ",\"floor\":";
    this->append_json_number_(out, sensor->floor);
    out += ",\"humidity\":";
    this->append_json_number_(out, sensor->humidity);
    out += ",\"dewpoint\":";
    this->append_json_number_(out, sensor->dewpoint);
    out += ",\"action\":";
    std::string action = this->action_name_(sensor->state);
    if (!control.has_sensor && action == "idle" && !std::isnan(control.target) &&
        !std::isnan(sensor->current) && control.target > sensor->current + 0.2f) {
      action = "heating";
    }
    this->append_json_string_(out, action);
    out += ",\"room_mode\":";
    out += std::to_string(control.room_mode);
    out += ",\"override_mode\":";
    out += std::to_string(control.override_mode);
    out += ",\"radiator_state\":";
    out += std::to_string(sensor->radiator_state);
    out += ",\"ufh_state\":";
    out += std::to_string(sensor->ufh_state);
    out += ",\"drying_state\":";
    out += std::to_string(sensor->drying_state);
    out += ",\"thermal_state\":";
    out += std::to_string(sensor->thermal_state);
    out += ",\"off\":";
    out += control.off ? "true" : "false";
    out += ",\"child_lock\":";
    out += control.lock_value == 8 ? "true" : "false";
    out += ",\"battery_low\":";
    out += sensor->battery_low ? "true" : "false";
    out += ",\"warning\":";
    out += sensor->warning ? "true" : "false";
    out += ",\"error\":";
    out += sensor->error ? "true" : "false";
    out += "}";
  }

  out += "]}";
  if (this->topology_text_sensor_ != nullptr)
    this->topology_text_sensor_->publish_state(out);
}

bool SentioV2::zone_set_target(uint8_t zone, float temperature) {
  if (zone < 1 || zone > SENTIO_MAX_ROOMS || !this->rooms_[zone - 1].present)
    return false;
  if (std::isnan(temperature))
    return false;
  temperature = std::max(5.0f, std::min(35.0f, temperature));
  const uint16_t base = (uint16_t) zone * ROOM_STRIDE;
  if (!this->write_single_register_(base + 17, 1))
    return false;
  if (!this->write_single_register_(base + 18, 0))
    return false;
  if (!this->write_single_register_(base + 19, (uint16_t) lroundf(temperature * 100.0f)))
    return false;
  this->rooms_[zone - 1].room_mode = 1;
  this->rooms_[zone - 1].override_mode = 0;
  this->rooms_[zone - 1].target = temperature;
  this->rooms_[zone - 1].off = false;
  this->publish_topology();
  return true;
}

bool SentioV2::zone_set_hvac_mode(uint8_t zone, bool turn_off) {
  if (zone < 1 || zone > SENTIO_MAX_ROOMS || !this->rooms_[zone - 1].present)
    return false;
  auto &room = this->rooms_[zone - 1];
  const uint16_t base = (uint16_t) zone * ROOM_STRIDE;
  if (!this->write_single_register_(base + 17, 1))
    return false;
  if (!this->write_single_register_(base + 18, 0))
    return false;
  room.room_mode = 1;
  room.override_mode = 0;
  if (turn_off) {
    if (!this->write_single_register_(base + 19, INVALID_FP100))
      return false;
    room.off = true;
    room.target = NAN;
  } else {
    float target = room.target;
    if (std::isnan(target) || target < 5.0f || target > 35.0f)
      target = 21.0f;
    if (!this->write_single_register_(base + 19, (uint16_t) lroundf(target * 100.0f)))
      return false;
    room.target = target;
    room.off = false;
  }
  this->publish_topology();
  return true;
}

bool SentioV2::zone_set_child_lock(uint8_t zone, bool enabled) {
  if (zone < 1 || zone > SENTIO_MAX_ROOMS || !this->rooms_[zone - 1].present)
    return false;
  const uint16_t address = (uint16_t) zone * ROOM_STRIDE + 20;
  const uint16_t value = enabled ? 8 : 32;
  if (!this->write_single_register_(address, value))
    return false;
  this->rooms_[zone - 1].lock_value = (uint8_t) value;
  this->publish_topology();
  return true;
}

bool SentioV2::zone_set_vacation(bool enabled) {
  if (!this->write_single_register_(27, enabled ? 1 : 0))
    return false;
  this->vacation_ = enabled;
  this->publish_topology();
  return true;
}

bool SentioV2::zone_set_sync(bool enabled) {
  if (!enabled) {
    this->sync_all_rooms_ = false;
    this->publish_topology();
    return true;
  }
  const uint8_t master = this->first_control_room_();
  if (master == 0) {
    ESP_LOGW(TAG, "Cannot enable room sync: no controllable rooms available");
    return false;
  }
  const float target = this->rooms_[master - 1].target;
  if (std::isnan(target))
    return false;
  this->sync_all_rooms_ = true;
  for (uint8_t room = 1; room <= SENTIO_MAX_ROOMS; room++) {
    if (room == master)
      continue;
    auto &state = this->rooms_[room - 1];
    if (!state.present || state.dummy || !state.has_control)
      continue;
    this->zone_set_target(room, target);
  }
  ESP_LOGI(TAG, "Room sync enabled: all rooms follow room %u at %.1f C",
           (unsigned) master, target);
  this->publish_topology();
  return true;
}

bool SentioV2::read_input_registers_(uint16_t start, uint16_t count, std::vector<uint16_t> &out) {
  std::vector<uint8_t> payload;
  if (!this->read_registers_(FC_READ_INPUT, start, count, payload))
    return false;
  out.clear();
  out.reserve(payload.size() / 2);
  for (size_t i = 0; i + 1 < payload.size(); i += 2)
    out.push_back((uint16_t) ((payload[i] << 8) | payload[i + 1]));
  return true;
}

bool SentioV2::read_input_individually_(uint16_t start, uint8_t count, std::vector<uint16_t> &out) {
  out.clear();
  bool any = false;
  for (uint8_t i = 0; i < count; i++) {
    std::vector<uint16_t> value;
    if (this->read_input_registers_(start + i, 1, value) && !value.empty()) {
      out.push_back(value[0]);
      any = true;
    } else {
      out.push_back(0);
    }
  }
  return any;
}

bool SentioV2::read_holding_registers_(uint16_t start, uint16_t count, std::vector<uint16_t> &out) {
  std::vector<uint8_t> payload;
  if (!this->read_registers_(FC_READ_HOLDING, start, count, payload))
    return false;
  out.clear();
  out.reserve(payload.size() / 2);
  for (size_t i = 0; i + 1 < payload.size(); i += 2)
    out.push_back((uint16_t) ((payload[i] << 8) | payload[i + 1]));
  return true;
}

bool SentioV2::read_discrete_inputs_(uint16_t start, uint16_t count, std::vector<bool> &out) {
  std::vector<uint8_t> payload;
  if (!this->read_registers_(FC_READ_DISCRETE, start, count, payload))
    return false;
  out.clear();
  out.reserve(count);
  for (uint16_t i = 0; i < count; i++)
    out.push_back(((payload[i / 8] >> (i % 8)) & 0x01) != 0);
  return true;
}

bool SentioV2::read_registers_(uint8_t function_code, uint16_t start, uint16_t count,
                                std::vector<uint8_t> &payload) {
  const uint8_t expected_payload = (uint8_t) ((function_code == FC_READ_DISCRETE) ? ((count + 7) / 8) : (count * 2));
  const uint16_t expected_length = (uint16_t) expected_payload + 5;

  for (uint8_t attempt = 0; attempt < 2; attempt++) {
    this->inter_frame_delay_();
    uint8_t frame[8];
    frame[0] = this->address_;
    frame[1] = function_code;
    frame[2] = (uint8_t) (start >> 8);
    frame[3] = (uint8_t) (start & 0xFF);
    frame[4] = (uint8_t) (count >> 8);
    frame[5] = (uint8_t) (count & 0xFF);
    const uint16_t crc = this->crc16_(frame, 6);
    frame[6] = (uint8_t) (crc & 0xFF);
    frame[7] = (uint8_t) (crc >> 8);

    this->prepare_tx_();
    this->write_array(frame, sizeof(frame));
    this->flush();
    this->finish_tx_();

    std::vector<uint8_t> response;
    if (!this->receive_frame_(function_code, expected_length, response))
      continue;
    if (response.size() != expected_length || response[2] != expected_payload)
      continue;
    payload.assign(response.begin() + 3, response.end() - 2);
    return true;
  }

  ESP_LOGW(TAG, "Read failed: fc=0x%02X start=%u count=%u", function_code, start, count);
  return false;
}

bool SentioV2::write_single_register_(uint16_t address, uint16_t value) {
  return this->write_register_(address, value);
}

bool SentioV2::write_register_(uint16_t address, uint16_t value) {
  for (uint8_t attempt = 0; attempt < 2; attempt++) {
    this->inter_frame_delay_();
    uint8_t frame[8];
    frame[0] = this->address_;
    frame[1] = FC_WRITE_SINGLE;
    frame[2] = (uint8_t) (address >> 8);
    frame[3] = (uint8_t) (address & 0xFF);
    frame[4] = (uint8_t) (value >> 8);
    frame[5] = (uint8_t) (value & 0xFF);
    const uint16_t crc = this->crc16_(frame, 6);
    frame[6] = (uint8_t) (crc & 0xFF);
    frame[7] = (uint8_t) (crc >> 8);

    this->prepare_tx_();
    this->write_array(frame, sizeof(frame));
    this->flush();
    this->finish_tx_();

    std::vector<uint8_t> response;
    if (!this->receive_frame_(FC_WRITE_SINGLE, 8, response))
      continue;
    if (response.size() != 8)
      continue;
    if (response[2] == frame[2] && response[3] == frame[3] &&
        response[4] == frame[4] && response[5] == frame[5])
      return true;
  }

  ESP_LOGW(TAG, "Write failed: address=%u value=%u", address, value);
  return false;
}

bool SentioV2::receive_frame_(uint8_t function_code, uint16_t expected_length,
                               std::vector<uint8_t> &frame) {
  const uint32_t deadline = millis() + this->receive_timeout_ms_;
  uint32_t last_byte_us = micros();
  bool seen_any = false;
  frame.clear();

  while (millis() < deadline) {
    while (this->available()) {
      int c = this->read();
      if (c < 0)
        break;
      last_byte_us = micros();
      seen_any = true;
      frame.push_back((uint8_t) c);

      while (!frame.empty() && frame[0] != this->address_)
        frame.erase(frame.begin());

      if (frame.size() >= 2 && frame[1] != function_code && frame[1] != (function_code | 0x80)) {
        frame.erase(frame.begin());
        continue;
      }

      if (frame.size() >= expected_length) {
        if (this->crc16_(frame.data(), frame.size()) == 0)
          return true;
        frame.erase(frame.begin());
      }
    }

    if (seen_any && (micros() - last_byte_us) > this->rx_gap_us_)
      break;
    delayMicroseconds(50);
  }

  return false;
}

void SentioV2::prepare_tx_() {
  while (this->available())
    this->read();
  if (this->flow_control_pin_ != nullptr)
    this->flow_control_pin_->digital_write(true);
  if (this->pre_tx_delay_us_ > 0)
    delayMicroseconds(this->pre_tx_delay_us_);
}

void SentioV2::finish_tx_() {
  if (this->post_tx_guard_us_ > 0)
    delayMicroseconds(this->post_tx_guard_us_);
  if (this->flow_control_pin_ != nullptr)
    this->flow_control_pin_->digital_write(false);
  this->tx_done_us_ = micros();
}

void SentioV2::inter_frame_delay_() {
  if (this->inter_frame_delay_us_ > 0)
    delayMicroseconds(this->inter_frame_delay_us_);
}

uint16_t SentioV2::crc16_(const uint8_t *data, size_t length) const {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      const bool lsb = (crc & 0x0001) != 0;
      crc >>= 1;
      if (lsb)
        crc ^= 0xA001;
    }
  }
  return crc;
}

}  // namespace sentiov2
}  // namespace esphome
