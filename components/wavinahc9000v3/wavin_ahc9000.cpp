#include "wavin_ahc9000.h"
#include "esphome/core/log.h"
#include "esphome/components/sensor/sensor.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>

namespace esphome {
namespace wavin_ahc9000 {

static const char *const TAG = "wavin_ahc9000";


// Simple Modbus CRC16 (0xA001 poly)
static uint16_t crc16(const uint8_t *frame, size_t len) {
  uint16_t temp = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    temp ^= frame[i];
    for (uint8_t j = 0; j < 8; j++) {
      bool flag = temp & 0x0001;
      temp >>= 1;
      if (flag) temp ^= 0xA001;
    }
  }
  return temp;
}

void WavinAHC9000::setup() {
  uint32_t baud = 38400;
  if (this->parent_ != nullptr) {
    uint32_t parent_baud = this->parent_->get_baud_rate();
    if (parent_baud != 0) baud = parent_baud;
  }
  if (this->tx_enable_pin_ != nullptr) {
    this->tx_enable_pin_->setup();
    this->tx_enable_pin_->digital_write(false);
  }
  if (this->flow_control_pin_ != nullptr) {
    this->flow_control_pin_->setup();
    this->flow_control_pin_->digital_write(false);
  }

  // Assume 11 bits per frame (start + 8 data + parity slot + stop)
  const uint32_t bits_per_frame = 11;
  const uint32_t frame_us = (bits_per_frame * 1000000UL + baud - 1) / baud;  // ceil division

  switch (this->module_profile_) {
    case ModuleProfile::MODULE_USTEPPER: {
      // uStepper RS485 hat needs extra guard time; approximate 12-byte frame plus slack.
      const uint32_t worst_case_bytes = 12;
      const uint32_t computed_guard = frame_us * worst_case_bytes + frame_us * 2;
      this->pre_tx_delay_us_ = std::max<uint32_t>(frame_us / 2, 100);
      this->post_tx_guard_us_ = std::max<uint32_t>(computed_guard, 2500);
      this->flush_rx_before_tx_ = true;
      break;
    }
    case ModuleProfile::MODULE_DEFAULT:
    default:
      this->pre_tx_delay_us_ = 0;
      this->post_tx_guard_us_ = std::max<uint32_t>(250, frame_us * 2);
      this->flush_rx_before_tx_ = false;
      break;
  }

  const char *profile = (this->module_profile_ == ModuleProfile::MODULE_USTEPPER) ? "ustepper" : "default";
  ESP_LOGCONFIG(TAG, "Wavin AHC9000 hub setup (baud=%u module=%s guard=%uus pre=%uus)",
                (unsigned) baud, profile, (unsigned) this->post_tx_guard_us_, (unsigned) this->pre_tx_delay_us_);
}
void WavinAHC9000::loop() {}

void WavinAHC9000::set_channel_friendly_name(uint8_t channel, const std::string &name) {
  if (channel < 1 || channel > 16) return;
  if (this->channel_friendly_names_.size() < 17) this->channel_friendly_names_.assign(17, std::string());
  this->channel_friendly_names_[channel] = name;
}

std::string WavinAHC9000::get_channel_friendly_name(uint8_t channel) const {
  if (channel < 1 || channel > 16) return std::string();
  if (this->channel_friendly_names_.size() < 17) return std::string();
  return this->channel_friendly_names_[channel];
}

void WavinAHC9000::update() {
  // If polling is temporarily suspended (after a write), skip until window expires
  if (this->suspend_polling_until_ != 0 && millis() < this->suspend_polling_until_) {
    ESP_LOGV(TAG, "Polling suspended for %u ms more", (unsigned) (this->suspend_polling_until_ - millis()));
    return;
  }

  // Process any urgent channels first (scheduled due to a write)
  std::vector<uint16_t> regs;
  uint8_t urgent_processed = 0;
  while (!this->urgent_channels_.empty() && urgent_processed < this->poll_channels_per_cycle_) {
    uint8_t ch = this->urgent_channels_.front();
    this->urgent_channels_.erase(this->urgent_channels_.begin());
    uint8_t ch_page = (uint8_t) (ch - 1);
    auto &st = this->channels_[ch];
    // Perform a compact refresh sequence for the channel
    if (this->read_registers(CAT_PACKED, ch_page, PACKED_CONFIGURATION, 1, regs) && regs.size() >= 1) {
      uint16_t raw_cfg = regs[0];
      uint16_t mode_bits = raw_cfg & PACKED_CONFIGURATION_MODE_MASK;
      bool is_off = (mode_bits == PACKED_CONFIGURATION_MODE_STANDBY) || (mode_bits == PACKED_CONFIGURATION_MODE_STANDBY_ALT);
      st.mode = is_off ? climate::CLIMATE_MODE_OFF : climate::CLIMATE_MODE_HEAT;
      st.child_lock = (raw_cfg & PACKED_CONFIGURATION_CHILD_LOCK_MASK) != 0;
      ESP_LOGD(TAG, "CH%u cfg=0x%04X mode=%s child_lock=%s", (unsigned) ch, (unsigned) raw_cfg, is_off ? "OFF" : "HEAT", st.child_lock?"Y":"N");
      // Reconcile desired mode if pending and mismatch
      auto it_des = this->desired_mode_.find(ch);
      if (it_des != this->desired_mode_.end()) {
        auto want = it_des->second;
        if (want != st.mode) {
          uint16_t current = raw_cfg;
          // Enforce standard OFF bits or MANUAL
          uint16_t new_bits = (want == climate::CLIMATE_MODE_OFF) ? PACKED_CONFIGURATION_MODE_STANDBY : PACKED_CONFIGURATION_MODE_MANUAL;
          uint16_t next = (uint16_t) ((current & ~PACKED_CONFIGURATION_MODE_MASK) | (new_bits & PACKED_CONFIGURATION_MODE_MASK));
          ESP_LOGW(TAG, "Reconciling mode for ch=%u cur=0x%04X next=0x%04X", (unsigned) ch, (unsigned) current, (unsigned) next);
          if (this->write_register(CAT_PACKED, ch_page, PACKED_CONFIGURATION, next)) {
            // Schedule another quick check
            this->urgent_channels_.push_back(ch);
            this->suspend_polling_until_ = millis() + 100;
          }
        } else {
          // Achieved desired mode; clear desire
          this->desired_mode_.erase(it_des);
        }
      }
    }
    if (this->read_registers(CAT_PACKED, ch_page, PACKED_MANUAL_TEMPERATURE, 1, regs) && regs.size() >= 1) {
      st.setpoint_c = this->raw_to_c(regs[0]);
    }
    // Read floor min/max (read-only) during urgent refresh in one combined request (reduces bus load)
    if (this->read_registers(CAT_PACKED, ch_page, PACKED_FLOOR_MIN_TEMPERATURE, 2, regs) && regs.size() >= 2) {
      st.floor_min_c = this->raw_to_c(regs[0]);
      st.floor_max_c = this->raw_to_c(regs[1]);
    }
    if (this->read_registers(CAT_CHANNELS, ch_page, CH_TIMER_EVENT, 1, regs) && regs.size() >= 1) {
      bool heating = (regs[0] & CH_TIMER_EVENT_OUTP_ON_MASK) != 0;
      st.action = heating ? climate::CLIMATE_ACTION_HEATING : climate::CLIMATE_ACTION_IDLE;
    }
    if (!st.all_tp_lost && st.primary_index > 0) {
      uint8_t elem_page = (uint8_t) (st.primary_index - 1);
      if (this->read_registers(CAT_ELEMENTS, elem_page, 0x00, 11, regs) && regs.size() > ELEM_AIR_TEMPERATURE) {
        st.current_temp_c = this->raw_to_c(regs[ELEM_AIR_TEMPERATURE]);
        if (regs.size() > ELEM_FLOOR_TEMPERATURE) {
          float ft = this->raw_to_c(regs[ELEM_FLOOR_TEMPERATURE]);
          // Basic plausibility filter (>1..90C) to avoid default/zero noise
          if (ft > 1.0f && ft < 90.0f) {
            st.floor_temp_c = ft;
            st.has_floor_sensor = true;
          } else {
            st.floor_temp_c = NAN;
          }
        }
      }
    }
    urgent_processed++;
  }

  // Round-robin staged reads across active channels; each advances one step per update
  if (this->active_channels_.empty()) {
    // Default to all 1..16 if none explicitly configured
    this->active_channels_.reserve(16);
    for (uint8_t ch = 1; ch <= 16; ch++) this->active_channels_.push_back(ch);
  }

  for (uint8_t i = urgent_processed; i < this->poll_channels_per_cycle_ && !this->active_channels_.empty(); i++) {
    // Wrap active index
    if (this->next_active_index_ >= this->active_channels_.size()) this->next_active_index_ = 0;
    uint8_t ch_num = this->active_channels_[this->next_active_index_]; // 1..16
    uint8_t ch_page = (uint8_t) (ch_num - 1);
    auto &st = this->channels_[ch_num];
    uint8_t &step = this->channel_step_[ch_page];

    // Two steps per update to surface values faster
    for (int s = 0; s < 2; s++) {
      switch (step) {
        case 0: {
          if (this->read_registers(CAT_CHANNELS, ch_page, CH_PRIMARY_ELEMENT, 1, regs) && regs.size() >= 1) {
            uint16_t v = regs[0];
            st.primary_index = v & CH_PRIMARY_ELEMENT_ELEMENT_MASK;
            st.all_tp_lost = (v & CH_PRIMARY_ELEMENT_ALL_TP_LOST_MASK) != 0;
            ESP_LOGD(TAG, "CH%u primary elem=%u lost=%s", ch_num, (unsigned) st.primary_index, st.all_tp_lost ? "Y" : "N");
          } else {
            ESP_LOGW(TAG, "CH%u: primary element read failed", ch_num);
          }
          step = 1;
          break;
        }
        case 1: {
          if (this->read_registers(CAT_PACKED, ch_page, PACKED_CONFIGURATION, 1, regs) && regs.size() >= 1) {
            uint16_t raw_cfg = regs[0];
            uint16_t mode_bits = raw_cfg & PACKED_CONFIGURATION_MODE_MASK;
            bool is_off = (mode_bits == PACKED_CONFIGURATION_MODE_STANDBY) || (mode_bits == PACKED_CONFIGURATION_MODE_STANDBY_ALT);
            st.mode = is_off ? climate::CLIMATE_MODE_OFF : climate::CLIMATE_MODE_HEAT;
            st.child_lock = (raw_cfg & PACKED_CONFIGURATION_CHILD_LOCK_MASK) != 0;
            ESP_LOGD(TAG, "CH%u cfg=0x%04X mode=%s child_lock=%s", ch_num, (unsigned) raw_cfg, is_off ? "OFF" : "HEAT", st.child_lock?"Y":"N");
          } else {
            ESP_LOGW(TAG, "CH%u: mode read failed", ch_num);
          }
          step = 2;
          break;
        }
        case 2: {
          if (this->read_registers(CAT_PACKED, ch_page, PACKED_MANUAL_TEMPERATURE, 1, regs) && regs.size() >= 1) {
            st.setpoint_c = this->raw_to_c(regs[0]);
            ESP_LOGD(TAG, "CH%u setpoint=%.1fC", ch_num, st.setpoint_c);
          } else {
            ESP_LOGW(TAG, "CH%u: setpoint read failed", ch_num);
          }
          step = 3;
          break;
        }
        case 3: {
          // Read floor min+max together (contiguous) to reduce transactions
          if (this->read_registers(CAT_PACKED, ch_page, PACKED_FLOOR_MIN_TEMPERATURE, 2, regs) && regs.size() >= 2) {
            st.floor_min_c = this->raw_to_c(regs[0]);
            st.floor_max_c = this->raw_to_c(regs[1]);
          }
          if (this->read_registers(CAT_CHANNELS, ch_page, CH_TIMER_EVENT, 1, regs) && regs.size() >= 1) {
            bool heating = (regs[0] & CH_TIMER_EVENT_OUTP_ON_MASK) != 0;
            st.action = heating ? climate::CLIMATE_ACTION_HEATING : climate::CLIMATE_ACTION_IDLE;
            ESP_LOGD(TAG, "CH%u action=%s", ch_num, heating ? "HEATING" : "IDLE");
          } else {
            ESP_LOGW(TAG, "CH%u: action read failed", ch_num);
          }
          step = 4;
          break;
        }
        case 4: {
          if (!st.all_tp_lost && st.primary_index > 0) {
            uint8_t elem_page = (uint8_t) (st.primary_index - 1);
            if (this->read_registers(CAT_ELEMENTS, elem_page, 0x00, 11, regs) && regs.size() > ELEM_AIR_TEMPERATURE) {
              st.current_temp_c = this->raw_to_c(regs[ELEM_AIR_TEMPERATURE]);
              if (regs.size() > ELEM_FLOOR_TEMPERATURE) {
                float ft = this->raw_to_c(regs[ELEM_FLOOR_TEMPERATURE]);
                if (ft > 1.0f && ft < 90.0f) {
                  st.floor_temp_c = ft;
                  st.has_floor_sensor = true;
                } else {
                  st.floor_temp_c = NAN;
                }
              }
              ESP_LOGD(TAG, "CH%u current=%.1fC", ch_num, st.current_temp_c);
              // Publish to per-channel temperature sensor if configured
              auto it_t = this->temperature_sensors_.find(ch_num);
              if (it_t != this->temperature_sensors_.end() && it_t->second != nullptr && !std::isnan(st.current_temp_c)) {
                it_t->second->publish_state(st.current_temp_c);
              }
              // Floor sensor publish (on staged read) if configured
              auto it_ft = this->floor_temperature_sensors_.find(ch_num);
              if (it_ft != this->floor_temperature_sensors_.end() && it_ft->second != nullptr && !std::isnan(st.floor_temp_c)) {
                it_ft->second->publish_state(st.floor_temp_c);
              }
              // Battery status if available (0..10 scale)
              if (regs.size() > ELEM_BATTERY_STATUS) {
                uint16_t raw = regs[ELEM_BATTERY_STATUS];
                uint8_t steps = (raw > 10) ? 10 : (uint8_t) raw;
                uint8_t pct = (uint8_t) (steps * 10);
                st.battery_pct = pct;
                auto it = this->battery_sensors_.find(ch_num);
                if (it != this->battery_sensors_.end() && it->second != nullptr) {
                  it->second->publish_state((float) pct);
                }
              }
            } else {
              ESP_LOGW(TAG, "CH%u: element temp read failed", ch_num);
            }
          } else {
            st.current_temp_c = NAN;
          }
          step = 0;
          break;
        }
      }
    }

  // advance to next active channel
  this->next_active_index_ = (uint8_t) ((this->next_active_index_ + 1) % this->active_channels_.size());
  }

  // publish once per cycle
  this->publish_updates();
}

void WavinAHC9000::dump_config() { ESP_LOGCONFIG(TAG, "Wavin AHC9000 (UART test read)"); }
void WavinAHC9000::dump_channel_floor_limits(uint8_t channel) {
  if (channel < 1 || channel > 16) return;
  uint8_t page = (uint8_t) (channel - 1);
  std::vector<uint16_t> regs;
  ESP_LOGI(TAG, "DUMP ch=%u PACKED indices 0x00..0x0F:", (unsigned) channel);
  for (uint8_t idx = 0; idx <= 0x0F; idx++) {
    if (this->read_registers(CAT_PACKED, page, idx, 1, regs) && regs.size() >= 1) {
      ESP_LOGI(TAG, "  PACKED[%u]=0x%04X (%u)", (unsigned) idx, (unsigned) regs[0], (unsigned) regs[0]);
    } else {
      ESP_LOGI(TAG, "  PACKED[%u]=<err>", (unsigned) idx);
    }
  }
  // Try elements if primary known
  auto it = this->channels_.find(channel);
  if (it != this->channels_.end() && it->second.primary_index > 0 && !it->second.all_tp_lost) {
    uint8_t elem_page = (uint8_t) (it->second.primary_index - 1);
    ESP_LOGI(TAG, "DUMP ch=%u ELEMENTS page=%u indices 0x00..0x10:", (unsigned) channel, (unsigned) elem_page);
    for (uint8_t idx = 0; idx <= 0x10; idx++) {
      if (this->read_registers(CAT_ELEMENTS, elem_page, idx, 1, regs) && regs.size() >= 1) {
        ESP_LOGI(TAG, "  ELEM[%u]=0x%04X (%u)", (unsigned) idx, (unsigned) regs[0], (unsigned) regs[0]);
      } else {
        ESP_LOGI(TAG, "  ELEM[%u]=<err>", (unsigned) idx);
      }
    }
  } else {
    ESP_LOGW(TAG, "DUMP ch=%u: primary element unknown or TP lost; elements not dumped", (unsigned) channel);
  }
}

void WavinAHC9000::add_channel_climate(WavinZoneClimate *c) { this->single_ch_climates_.push_back(c); }
void WavinAHC9000::add_group_climate(WavinZoneClimate *c) { this->group_climates_.push_back(c); }
void WavinAHC9000::add_active_channel(uint8_t ch) {
  if (ch < 1 || ch > 16) return;
  if (std::find(this->active_channels_.begin(), this->active_channels_.end(), ch) == this->active_channels_.end()) {
    this->active_channels_.push_back(ch);
  }
}

// Repair functions removed; use normalize_channel_config via API service

bool WavinAHC9000::read_registers(uint8_t category, uint8_t page, uint8_t index, uint8_t count, std::vector<uint16_t> &out) {
  // Retry logic: attempt up to IO_RETRY_ATTEMPTS. First attempt failures are logged at DEBUG; only the
  // final failed attempt escalates to WARN to reduce log noise from transient bus glitches.
  for (uint8_t attempt = 0; attempt < IO_RETRY_ATTEMPTS; attempt++) {
    uint8_t msg[8];
    msg[0] = DEVICE_ADDR;
    msg[1] = FC_READ;
    msg[2] = category;
    msg[3] = index;
    msg[4] = page;
    msg[5] = count;
    uint16_t crc = crc16(msg, 6);
    msg[6] = crc & 0xFF;
    msg[7] = crc >> 8;

    this->prepare_for_tx_();
    ESP_LOGD(TAG, "TX: addr=0x%02X fc=0x%02X cat=%u idx=%u page=%u cnt=%u attempt=%u", msg[0], msg[1], category, index, page, count, (unsigned) attempt + 1);
    this->write_array(msg, 8);
    this->flush();
    this->finish_tx_();

    std::vector<uint8_t> buf;
    uint32_t start = millis();
    while (millis() - start < this->receive_timeout_ms_) {
      while (this->available()) {
        int c = this->read();
        if (c < 0) break;
        buf.push_back((uint8_t) c);
        if (buf.size() >= 5) {
          uint8_t expected = (uint8_t) (buf[2] + 5);
          if (buf[0] == DEVICE_ADDR && buf[1] == FC_READ && buf.size() == expected) {
            uint16_t rcrc = crc16(buf.data(), buf.size());
            if (rcrc != 0) {
              // CRC mismatch: retry unless last attempt
              if (attempt + 1 == IO_RETRY_ATTEMPTS) {
                ESP_LOGW(TAG, "RX: CRC mismatch (len=%u) after %u attempts", (unsigned) buf.size(), (unsigned) IO_RETRY_ATTEMPTS);
              } else {
                ESP_LOGD(TAG, "RX: CRC mismatch attempt %u (len=%u) -> retry", (unsigned) attempt + 1, (unsigned) buf.size());
              }
              goto next_attempt; // break both loops, retry
            }
            uint8_t bytes = buf[2];
            out.clear();
            for (uint8_t i = 0; i + 1 < bytes; i += 2) {
              uint16_t w = (uint16_t) (buf[3 + i] << 8) | buf[3 + i + 1];
              out.push_back(w);
            }
            return true;
          }
        }
      }
      delay(1);
    }
    // Timeout
    if (attempt + 1 == IO_RETRY_ATTEMPTS) {
      ESP_LOGW(TAG, "RX: timeout waiting for response after %u attempts (cat=%u idx=%u page=%u cnt=%u)", (unsigned) IO_RETRY_ATTEMPTS, category, index, page, count);
    } else {
      ESP_LOGD(TAG, "RX: timeout attempt %u (cat=%u idx=%u page=%u) -> retry", (unsigned) attempt + 1, category, index, page);
    }
  next_attempt:;
  }
  return false;
}

bool WavinAHC9000::write_register(uint8_t category, uint8_t page, uint8_t index, uint16_t value) {
  // Similar retry strategy as read_registers() with severity gating.
  for (uint8_t attempt = 0; attempt < IO_RETRY_ATTEMPTS; attempt++) {
    uint8_t msg[10];
    msg[0] = DEVICE_ADDR;
    msg[1] = FC_WRITE;
    msg[2] = category;
    msg[3] = index;
    msg[4] = page;
    msg[5] = 1;  // count
    msg[6] = (uint8_t) (value >> 8);
    msg[7] = (uint8_t) (value & 0xFF);
    uint16_t crc = crc16(msg, 8);
    msg[8] = (uint8_t) (crc & 0xFF);
    msg[9] = (uint8_t) (crc >> 8);

    this->prepare_for_tx_();
    ESP_LOGD(TAG, "TX-WR: cat=%u idx=%u page=%u val=0x%04X attempt=%u", category, index, page, (unsigned) value, (unsigned) attempt + 1);
    this->write_array(msg, 10);
    this->flush();
    this->finish_tx_();

    std::vector<uint8_t> buf;
    uint32_t start = millis();
    while (millis() - start < this->receive_timeout_ms_) {
      while (this->available()) {
        int c = this->read();
        if (c < 0) break;
        buf.push_back((uint8_t) c);
        if (buf.size() >= 5) {
          uint8_t expected = (uint8_t) (buf[2] + 5);
          if (buf[0] == DEVICE_ADDR && buf[1] == FC_WRITE && buf.size() == expected) {
            uint16_t rcrc = crc16(buf.data(), buf.size());
            bool ok = (rcrc == 0);
            if (!ok) {
              if (attempt + 1 == IO_RETRY_ATTEMPTS) {
                ESP_LOGW(TAG, "ACK-WR: CRC mismatch after %u attempts (cat=%u idx=%u page=%u)", (unsigned) IO_RETRY_ATTEMPTS, category, index, page);
              } else {
                ESP_LOGD(TAG, "ACK-WR: CRC mismatch attempt %u -> retry", (unsigned) attempt + 1);
              }
              goto next_wr_attempt;
            }
            ESP_LOGD(TAG, "ACK-WR: OK");
            return true;
          }
        }
      }
      delay(1);
    }
    if (attempt + 1 == IO_RETRY_ATTEMPTS) {
      ESP_LOGW(TAG, "ACK-WR: timeout after %u attempts (cat=%u idx=%u page=%u)", (unsigned) IO_RETRY_ATTEMPTS, category, index, page);
    } else {
      ESP_LOGD(TAG, "ACK-WR: timeout attempt %u (cat=%u idx=%u page=%u) -> retry", (unsigned) attempt + 1, category, index, page);
    }
  next_wr_attempt:;
  }
  return false;
}

bool WavinAHC9000::write_masked_register(uint8_t category, uint8_t page, uint8_t index, uint16_t and_mask, uint16_t or_mask) {
  // Similar retry strategy as write_register(); reduces spurious WARN logs.
  for (uint8_t attempt = 0; attempt < IO_RETRY_ATTEMPTS; attempt++) {
    uint8_t msg[12];
    msg[0] = DEVICE_ADDR;
    msg[1] = FC_WRITE_MASKED;
    msg[2] = category;
    msg[3] = index;
    msg[4] = page;
    msg[5] = 1;  // count
    msg[6] = (uint8_t) (and_mask >> 8);
    msg[7] = (uint8_t) (and_mask & 0xFF);
    msg[8] = (uint8_t) (or_mask >> 8);
    msg[9] = (uint8_t) (or_mask & 0xFF);
    uint16_t crc = crc16(msg, 10);
    msg[10] = (uint8_t) (crc & 0xFF);
    msg[11] = (uint8_t) (crc >> 8);

    this->prepare_for_tx_();
    ESP_LOGD(TAG, "TX-WM: cat=%u idx=%u page=%u and=0x%04X or=0x%04X attempt=%u", category, index, page, (unsigned) and_mask, (unsigned) or_mask, (unsigned) attempt + 1);
    this->write_array(msg, 12);
    this->flush();
    this->finish_tx_();

    std::vector<uint8_t> buf;
    uint32_t start = millis();
    while (millis() - start < this->receive_timeout_ms_) {
      while (this->available()) {
        int c = this->read();
        if (c < 0) break;
        buf.push_back((uint8_t) c);
        if (buf.size() >= 5) {
          uint8_t expected = (uint8_t) (buf[2] + 5);
          if (buf[0] == DEVICE_ADDR && buf[1] == FC_WRITE_MASKED && buf.size() == expected) {
            uint16_t rcrc = crc16(buf.data(), buf.size());
            bool ok = (rcrc == 0);
            if (!ok) {
              if (attempt + 1 == IO_RETRY_ATTEMPTS) {
                ESP_LOGW(TAG, "ACK-WM: CRC mismatch after %u attempts (cat=%u idx=%u page=%u)", (unsigned) IO_RETRY_ATTEMPTS, category, index, page);
              } else {
                ESP_LOGD(TAG, "ACK-WM: CRC mismatch attempt %u -> retry", (unsigned) attempt + 1);
              }
              goto next_wm_attempt;
            }
            ESP_LOGD(TAG, "ACK-WM: OK");
            return true;
          }
        }
      }
      delay(1);
    }
    if (attempt + 1 == IO_RETRY_ATTEMPTS) {
      ESP_LOGW(TAG, "ACK-WM: timeout after %u attempts (cat=%u idx=%u page=%u)", (unsigned) IO_RETRY_ATTEMPTS, category, index, page);
    } else {
      ESP_LOGD(TAG, "ACK-WM: timeout attempt %u (cat=%u idx=%u page=%u) -> retry", (unsigned) attempt + 1, category, index, page);
    }
  next_wm_attempt:;
  }
  return false;
}

// High-level write helpers
void WavinAHC9000::write_channel_setpoint(uint8_t channel, float celsius) {
  if (channel < 1 || channel > 16) return;
  uint8_t page = (uint8_t) (channel - 1);
  uint16_t raw = this->c_to_raw(celsius);
  if (this->write_register(CAT_PACKED, page, PACKED_MANUAL_TEMPERATURE, raw)) {
    this->channels_[channel].setpoint_c = celsius;
  // Schedule a quick refresh on next cycle and briefly suspend normal polling to avoid collisions
  this->urgent_channels_.push_back(channel);
  this->suspend_polling_until_ = millis() + 100; // 100 ms guard
  }
}

void WavinAHC9000::write_group_setpoint(const std::vector<uint8_t> &members, float celsius) {
  for (auto ch : members) this->write_channel_setpoint(ch, celsius);
}

void WavinAHC9000::write_channel_mode(uint8_t channel, climate::ClimateMode mode) {
  if (channel < 1 || channel > 16) return;
  uint8_t page = (uint8_t) (channel - 1);
  this->desired_mode_[channel] = mode;
  // Always use strict baseline to 0x4000/0x4001 for reliable OFF/HEAT
  bool ok = false;
  {
    uint16_t strict_val = (uint16_t) (0x4000 | (mode == climate::CLIMATE_MODE_OFF ? PACKED_CONFIGURATION_MODE_STANDBY : PACKED_CONFIGURATION_MODE_MANUAL));
    ok = this->write_register(CAT_PACKED, page, PACKED_CONFIGURATION, strict_val);
  }
  if (!ok) {
    // Fallback: read-modify-write full register (update only mode bits)
    std::vector<uint16_t> regs;
    if (this->read_registers(CAT_PACKED, page, PACKED_CONFIGURATION, 1, regs) && regs.size() >= 1) {
      uint16_t current = regs[0];
      // Prefer standard standby bits for OFF; otherwise use MANUAL
      uint16_t new_bits = (mode == climate::CLIMATE_MODE_OFF) ? PACKED_CONFIGURATION_MODE_STANDBY : PACKED_CONFIGURATION_MODE_MANUAL;
      uint16_t next = (uint16_t) ((current & ~PACKED_CONFIGURATION_MODE_MASK) | (new_bits & PACKED_CONFIGURATION_MODE_MASK));
      ESP_LOGW(TAG, "WM fallback: PACKED_CONFIGURATION ch=%u cur=0x%04X next=0x%04X", (unsigned) channel, (unsigned) current, (unsigned) next);
      ok = this->write_register(CAT_PACKED, page, PACKED_CONFIGURATION, next);
  // No alternate OFF attempt to avoid special thermostat modes
    } else {
      ESP_LOGW(TAG, "WM fallback: read PACKED_CONFIGURATION failed for ch=%u", (unsigned) channel);
    }
  }
  if (ok) {
    this->channels_[channel].mode = (mode == climate::CLIMATE_MODE_OFF) ? climate::CLIMATE_MODE_OFF : climate::CLIMATE_MODE_HEAT;
    this->urgent_channels_.push_back(channel);
    this->suspend_polling_until_ = millis() + 100; // 100 ms guard
  } else {
    ESP_LOGW(TAG, "Mode write failed for ch=%u", (unsigned) channel);
  }
}

void WavinAHC9000::write_channel_child_lock(uint8_t channel, bool enable) {
  if (channel < 1 || channel > 16) return;
  uint8_t page = (uint8_t) (channel - 1);
  std::vector<uint16_t> regs;
  if (!this->read_registers(CAT_PACKED, page, PACKED_CONFIGURATION, 1, regs) || regs.size() < 1) {
    ESP_LOGW(TAG, "Child lock: read current config failed ch=%u", (unsigned) channel);
    return;
  }
  uint16_t current = regs[0];
  uint16_t next;
  if (enable)
    next = (uint16_t) (current | PACKED_CONFIGURATION_CHILD_LOCK_MASK);
  else
    next = (uint16_t) (current & ~PACKED_CONFIGURATION_CHILD_LOCK_MASK);
  if (next == current) {
    ESP_LOGD(TAG, "Child lock: no change ch=%u (enable=%s)", (unsigned) channel, enable?"true":"false");
    return;
  }
  if (this->write_register(CAT_PACKED, page, PACKED_CONFIGURATION, next)) {
    this->channels_[channel].child_lock = enable;
    this->urgent_channels_.push_back(channel);
    this->suspend_polling_until_ = millis() + 100;
    ESP_LOGI(TAG, "Child lock: set ch=%u -> %s (0x%04X)", (unsigned) channel, enable?"ENABLED":"DISABLED", (unsigned) next);
  } else {
    ESP_LOGW(TAG, "Child lock: write failed ch=%u", (unsigned) channel);
  }
}

void WavinAHC9000::write_channel_floor_min_temperature(uint8_t channel, float celsius) {
  if (channel < 1 || channel > 16) return;
  // Clamp to a sane range; controller likely enforces further constraints
  if (celsius < 5.0f) celsius = 5.0f;
  if (celsius > 35.0f) celsius = 35.0f;
  uint8_t page = (uint8_t) (channel - 1);
  uint16_t raw = this->c_to_raw(celsius);
  if (this->write_register(CAT_PACKED, page, PACKED_FLOOR_MIN_TEMPERATURE, raw)) {
    this->channels_[channel].floor_min_c = celsius;
    this->urgent_channels_.push_back(channel);
    this->suspend_polling_until_ = millis() + 100;
  }
}

void WavinAHC9000::write_channel_floor_max_temperature(uint8_t channel, float celsius) {
  if (channel < 1 || channel > 16) return;
  if (celsius < 5.0f) celsius = 5.0f;
  if (celsius > 35.0f) celsius = 35.0f;
  uint8_t page = (uint8_t) (channel - 1);
  uint16_t raw = this->c_to_raw(celsius);
  if (this->write_register(CAT_PACKED, page, PACKED_FLOOR_MAX_TEMPERATURE, raw)) {
    this->channels_[channel].floor_max_c = celsius;
    this->urgent_channels_.push_back(channel);
    this->suspend_polling_until_ = millis() + 100;
  }
}

void WavinAHC9000::set_strict_mode_write(uint8_t channel, bool enable) {
  if (channel < 1 || channel > 16) return;
  if (enable) this->strict_mode_channels_.insert(channel);
  else this->strict_mode_channels_.erase(channel);
}
bool WavinAHC9000::is_strict_mode_write(uint8_t channel) const {
  return this->strict_mode_channels_.find(channel) != this->strict_mode_channels_.end();
}

void WavinAHC9000::refresh_channel_now(uint8_t channel) {
  if (channel < 1 || channel > 16) return;
  // Just schedule urgent refresh; actual reads happen in update()
  this->urgent_channels_.push_back(channel);
}

void WavinAHC9000::normalize_channel_config(uint8_t channel, bool off) {
  if (channel < 1 || channel > 16) return;
  uint8_t page = (uint8_t) (channel - 1);
  // Force PACKED_CONFIGURATION to exact baseline used by healthy channels
  uint16_t value = (uint16_t) (0x4000 | (off ? PACKED_CONFIGURATION_MODE_STANDBY : PACKED_CONFIGURATION_MODE_MANUAL));
  if (this->write_register(CAT_PACKED, page, PACKED_CONFIGURATION, value)) {
    ESP_LOGW(TAG, "Normalize (strict) applied: ch=%u -> 0x%04X", (unsigned) channel, (unsigned) value);
    this->urgent_channels_.push_back(channel);
    this->suspend_polling_until_ = millis() + 100;
  } else {
    ESP_LOGW(TAG, "Normalize (strict) failed: write not acknowledged for ch=%u", (unsigned) channel);
  }
}

void WavinAHC9000::generate_yaml_suggestion() {
  // One-shot discovery sweep: detect active channels immediately (independent of background polling)
  std::vector<uint8_t> active;
  active.reserve(16);
  // Map primary element index -> list of channels sharing it (for group climate suggestions)
  std::map<uint16_t, std::vector<uint8_t>> primary_groups;
  // Group detection rationale:
  // If multiple channels report the same primary element index they physically share the same thermostat.
  // We propose an optional aggregate climate entity using `members: [a, b, ...]` so users can control
  // all loops for that room with a single setpoint/mode. We keep single-channel suggestions as well
  // so they can choose either approach. Naming uses a compact pattern:
  //   - Exactly two channels:  Zone G <a>&<b>
  //   - More than two:        Zone G <first>-<last>
  // Users can rename afterwards; we avoid including 'Primary' or raw element index to keep it friendly.
  std::vector<uint16_t> regs;
  for (uint8_t ch = 1; ch <= 16; ch++) {
    uint8_t page = (uint8_t) (ch - 1);
    if (this->read_registers(CAT_CHANNELS, page, CH_PRIMARY_ELEMENT, 1, regs) && regs.size() >= 1) {
      uint16_t v = regs[0];
      uint16_t primary_index = v & CH_PRIMARY_ELEMENT_ELEMENT_MASK;
      bool all_tp_lost = (v & CH_PRIMARY_ELEMENT_ALL_TP_LOST_MASK) != 0;
      if (primary_index > 0 && !all_tp_lost) {
        active.push_back(ch);
        // Opportunistically fill cache (does not change behavior)
        auto &st = this->channels_[ch];
        st.primary_index = primary_index;
        st.all_tp_lost = all_tp_lost;
        if (primary_index > 0) {
          primary_groups[primary_index].push_back(ch);
        }
        // Read basic mode + setpoint so climates look sensible in cache
        if (this->read_registers(CAT_PACKED, page, PACKED_CONFIGURATION, 1, regs) && regs.size() >= 1) {
          uint16_t raw_cfg = regs[0];
          uint16_t mode_bits = raw_cfg & PACKED_CONFIGURATION_MODE_MASK;
          bool is_off = (mode_bits == PACKED_CONFIGURATION_MODE_STANDBY) || (mode_bits == PACKED_CONFIGURATION_MODE_STANDBY_ALT);
          st.mode = is_off ? climate::CLIMATE_MODE_OFF : climate::CLIMATE_MODE_HEAT;
          st.child_lock = (raw_cfg & PACKED_CONFIGURATION_CHILD_LOCK_MASK) != 0;
        }
        if (this->read_registers(CAT_PACKED, page, PACKED_MANUAL_TEMPERATURE, 1, regs) && regs.size() >= 1) {
          st.setpoint_c = this->raw_to_c(regs[0]);
        }
        // Floor min/max (read-only) combined into one read (contiguous indices)
        if (this->read_registers(CAT_PACKED, page, PACKED_FLOOR_MIN_TEMPERATURE, 2, regs) && regs.size() >= 2) {
          st.floor_min_c = this->raw_to_c(regs[0]);
          st.floor_max_c = this->raw_to_c(regs[1]);
        }
        // Try to read elements block to surface air/floor temps and detect floor probe immediately
        uint8_t elem_page = (uint8_t) (primary_index - 1);
        if (this->read_registers(CAT_ELEMENTS, elem_page, 0x00, 11, regs) && regs.size() > ELEM_AIR_TEMPERATURE) {
          st.current_temp_c = this->raw_to_c(regs[ELEM_AIR_TEMPERATURE]);
          if (regs.size() > ELEM_FLOOR_TEMPERATURE) {
            float ft = this->raw_to_c(regs[ELEM_FLOOR_TEMPERATURE]);
            if (ft > 1.0f && ft < 90.0f) {
              st.floor_temp_c = ft;
              bool threshold_hit = (ft >= 15.0f);
              bool deviates = (!std::isnan(st.current_temp_c) && std::fabs(st.current_temp_c - ft) > 0.2f);
              if (threshold_hit || deviates) st.has_floor_sensor = true;
            } else {
              st.floor_temp_c = NAN;
            }
          }
          // Battery (optional)
          if (regs.size() > ELEM_BATTERY_STATUS) {
            uint16_t raw = regs[ELEM_BATTERY_STATUS];
            uint8_t steps = (raw > 10) ? 10 : (uint8_t) raw;
            st.battery_pct = (uint8_t) (steps * 10);
          }
        }
      }
    }
  }

  // Build YAML sections; determine grouped channels first so we can comment out their single climates
  std::ostringstream climate_ss;
  climate_ss << "climate:\n";
  std::set<uint8_t> grouped_channels;

  // Group climates: for any primary element shared by >1 channel, propose a members-based climate.
  // Name strategy: "Zone G <first>-<last>" or if exactly 2 channels "Zone G <a>&<b>".
  std::ostringstream group_ss;
  bool have_group_section = false;
  for (const auto &kv : primary_groups) {
    const auto &chs = kv.second;
    if (chs.size() <= 1) continue;
    std::vector<uint8_t> sorted = chs;
    std::sort(sorted.begin(), sorted.end());
    std::string name;
    // If all members have friendly names, build a composite
    bool all_named = true;
    std::vector<std::string> member_names;
    for (auto ch : sorted) {
      auto fn = this->get_channel_friendly_name(ch);
      if (fn.empty()) { all_named = false; break; }
      member_names.push_back(fn);
    }
    if (all_named && !member_names.empty()) {
      if (member_names.size() == 2) {
        name = member_names[0] + " & " + member_names[1];
      } else if (member_names.size() <= 4) {
        // Join with commas and ' & ' before last for readability
        for (size_t i = 0; i < member_names.size(); ++i) {
          if (i > 0) name += (i + 1 == member_names.size() ? " & " : ", ");
          name += member_names[i];
        }
      } else {
        // Too many to list: First - Last pattern
        name = member_names.front() + " – " + member_names.back();
      }
    } else {
      if (sorted.size() == 2) name = "Zone G " + std::to_string((int) sorted[0]) + "&" + std::to_string((int) sorted[1]);
      else name = "Zone G " + std::to_string((int) sorted.front()) + "-" + std::to_string((int) sorted.back());
    }
    if (!have_group_section) {
      group_ss << "\n  # Group climates (shared thermostat across members)\n";
      have_group_section = true;
    }
    group_ss << "  - platform: wavin_ahc9000\n";
    group_ss << "    wavin_ahc9000_id: wavin\n";
    group_ss << "    name: \"" << name << "\"\n";
    group_ss << "    members: [";
    for (size_t i = 0; i < sorted.size(); i++) {
      group_ss << (int) sorted[i];
      if (i + 1 < sorted.size()) group_ss << ", ";
      // Mark channel as grouped
      grouped_channels.insert(sorted[i]);
    }
    group_ss << "]\n";
  }

  // Now append single climates (comment out those that are grouped)
  for (auto ch : active) {
    std::string fname = this->get_channel_friendly_name(ch);
    if (fname.empty()) fname = "Zone " + std::to_string((int) ch);
    bool grouped = grouped_channels.count(ch) != 0;
    const char *prefix = grouped ? "  # " : "  ";
    climate_ss << prefix << "- platform: wavin_ahc9000\n";
    climate_ss << prefix << "  wavin_ahc9000_id: wavin\n";
    climate_ss << prefix << "  name: \"" << fname << "\"\n";
    climate_ss << prefix << "  channel: " << (int) ch << "\n";
    if (grouped)
      climate_ss << "  # Commented out because this channel participates in a group climate below.\n";
  }

    if (have_group_section) climate_ss << group_ss.str();

  // Comfort climates (floor-based current temp) for channels with detected floor sensor
  bool have_comfort_section = false;
  for (auto ch : active) {
    auto it = this->channels_.find(ch);
    if (it != this->channels_.end() && it->second.has_floor_sensor) {
      std::string fname = this->get_channel_friendly_name(ch);
      if (fname.empty()) fname = "Zone " + std::to_string((int) ch);
      if (!have_comfort_section) {
        climate_ss << "\n  # Comfort climates (floor temperature as the current reading)\n";
        have_comfort_section = true;
      }
      climate_ss << "  - platform: wavin_ahc9000\n";
      climate_ss << "    wavin_ahc9000_id: wavin\n";
      climate_ss << "    name: \"" << fname << " Comfort\"\n";
      climate_ss << "    channel: " << (int) ch << "\n";
      climate_ss << "    use_floor_temperature: true\n";
      // Logged suggestion only; no additional bookkeeping required.
    }
  }  // end for(active) comfort climates loop
  climate_ss << std::endl;

  std::ostringstream sensor_ss;
  sensor_ss << "sensor:\n";
  bool have_sensor_entries = false;
  if (!active.empty()) {
    have_sensor_entries = true;
    sensor_ss << "  # Battery level sensors\n";
    for (auto ch : active) {
      std::string fname = this->get_channel_friendly_name(ch);
      if (fname.empty()) fname = "Zone " + std::to_string((int) ch);
      sensor_ss << "  - platform: wavin_ahc9000\n";
      sensor_ss << "    wavin_ahc9000_id: wavin\n";
      sensor_ss << "    name: \"" << fname << " Battery\"\n";
      sensor_ss << "    channel: " << (int) ch << "\n";
      sensor_ss << "    type: battery\n";
    }
    sensor_ss << "\n  # Air temperature sensors\n";
    for (auto ch : active) {
      std::string fname = this->get_channel_friendly_name(ch);
      if (fname.empty()) fname = "Zone " + std::to_string((int) ch);
      sensor_ss << "  - platform: wavin_ahc9000\n";
      sensor_ss << "    wavin_ahc9000_id: wavin\n";
      sensor_ss << "    name: \"" << fname << " Temperature\"\n";
      sensor_ss << "    channel: " << (int) ch << "\n";
      sensor_ss << "    type: temperature\n";
    }
  }
  sensor_ss << std::endl;

  std::ostringstream switch_ss;
  switch_ss << "switch:\n";
  bool have_switch_entries = false;
  if (!active.empty()) {
    have_switch_entries = true;
    switch_ss << "  # Child lock switches\n";
    for (auto ch : active) {
      std::string fname = this->get_channel_friendly_name(ch);
      if (fname.empty()) fname = "Zone " + std::to_string((int) ch);
      switch_ss << "  - platform: wavin_ahc9000\n";
      switch_ss << "    wavin_ahc9000_id: wavin\n";
      switch_ss << "    name: \"" << fname << " Child Lock\"\n";
      switch_ss << "    channel: " << (int) ch << "\n";
    }
  }
  switch_ss << std::endl;

  std::vector<std::string> sections;
  sections.push_back(climate_ss.str());
  if (have_sensor_entries) sections.push_back(sensor_ss.str());
  if (have_switch_entries) sections.push_back(switch_ss.str());

  // Print with conservative chunking to avoid logger line-length truncation.
  ESP_LOGI(TAG, "==================== Wavin YAML SUGGESTION BEGIN ====================");
  for (const auto &block : sections) {
    if (block.empty()) continue;
    std::istringstream stream(block);
    std::string line;
    std::string chunk;
    constexpr size_t CHUNK_MAX = 200;  // keep well under ESPHome logger chunk limit
    while (std::getline(stream, line)) {
      if (!chunk.empty()) chunk += '\n';
      chunk += line;
      if (chunk.size() >= CHUNK_MAX) {
        ESP_LOGI(TAG, "%s", chunk.c_str());
        chunk.clear();
      }
    }
    if (!chunk.empty()) {
      ESP_LOGI(TAG, "%s", chunk.c_str());
    }
  }
  ESP_LOGI(TAG, "===================== Wavin YAML SUGGESTION END =====================");
}

void WavinAHC9000::publish_updates() {
  ESP_LOGV(TAG, "Publishing updates: %u single climates, %u group climates",
           (unsigned) this->single_ch_climates_.size(), (unsigned) this->group_climates_.size());
  for (auto *c : this->single_ch_climates_) c->update_from_parent();
  for (auto *c : this->group_climates_) c->update_from_parent();
  // Channel sensors
  for (auto &kv : this->temperature_sensors_) {
    uint8_t ch = kv.first;
    auto *s = kv.second;
    if (!s) continue;
    float v = this->get_channel_current_temp(ch);
    if (!std::isnan(v)) s->publish_state(v);
  }
  for (auto &kv : this->battery_sensors_) {
    uint8_t ch = kv.first;
    auto *s = kv.second;
    if (!s) continue;
    auto it = this->channels_.find(ch);
    if (it != this->channels_.end() && it->second.battery_pct != 255) {
      s->publish_state((float) it->second.battery_pct);
    }
  }
  for (auto &kv : this->comfort_setpoint_sensors_) {
    uint8_t ch = kv.first;
    auto *s = kv.second;
    if (!s) continue;
    float v = this->get_channel_setpoint(ch);
    if (!std::isnan(v)) s->publish_state(v);
  }
  for (auto &kv : this->floor_temperature_sensors_) {
    uint8_t ch = kv.first;
    auto *s = kv.second;
    if (!s) continue;
    auto it = this->channels_.find(ch);
    if (it != this->channels_.end() && it->second.has_floor_sensor) {
      float v = it->second.floor_temp_c;
      if (!std::isnan(v)) s->publish_state(v);
    }
  }
  // Publish floor limit sensors (read-only)
  for (auto &kv : this->floor_min_temperature_sensors_) {
    uint8_t ch = kv.first;
    auto *s = kv.second;
    if (!s) continue;
    auto it = this->channels_.find(ch);
    if (it != this->channels_.end()) {
      float v = it->second.floor_min_c;
      if (!std::isnan(v)) s->publish_state(v);
    }
  }
  for (auto &kv : this->floor_max_temperature_sensors_) {
    uint8_t ch = kv.first;
    auto *s = kv.second;
    if (!s) continue;
    auto it = this->channels_.find(ch);
    if (it != this->channels_.end()) {
      float v = it->second.floor_max_c;
      if (!std::isnan(v)) s->publish_state(v);
    }
  }

  // Child lock switches
  for (auto &kv : this->child_lock_switches_) {
    uint8_t ch = kv.first;
    auto *sw = kv.second;
    if (!sw) continue;
    auto it = this->channels_.find(ch);
    if (it != this->channels_.end()) {
      sw->publish_state(it->second.child_lock);
    }
  }

}


void WavinAHC9000::clear_stale_rx_() {
  if (!this->flush_rx_before_tx_) return;
  while (this->available()) {
    int c = this->read();
    if (c < 0) break;
  }
}

void WavinAHC9000::prepare_for_tx_() {
  this->clear_stale_rx_();
  if (this->flow_control_pin_ != nullptr) this->flow_control_pin_->digital_write(true);
  if (this->tx_enable_pin_ != nullptr) this->tx_enable_pin_->digital_write(true);
  if (this->pre_tx_delay_us_ > 0) delayMicroseconds(this->pre_tx_delay_us_);
}

void WavinAHC9000::finish_tx_() {
  delayMicroseconds(this->post_tx_guard_us_);
  if (this->tx_enable_pin_ != nullptr) this->tx_enable_pin_->digital_write(false);
  if (this->flow_control_pin_ != nullptr) this->flow_control_pin_->digital_write(false);
}

float WavinAHC9000::get_channel_current_temp(uint8_t channel) const {
  auto it = this->channels_.find(channel);
  return it == this->channels_.end() ? NAN : it->second.current_temp_c;
}
float WavinAHC9000::get_channel_setpoint(uint8_t channel) const {
  auto it = this->channels_.find(channel);
  return it == this->channels_.end() ? NAN : it->second.setpoint_c;
}
float WavinAHC9000::get_channel_floor_temp(uint8_t channel) const {
  auto it = this->channels_.find(channel);
  return it == this->channels_.end() ? NAN : it->second.floor_temp_c;
}
float WavinAHC9000::get_channel_floor_min_temp(uint8_t channel) const {
  auto it = this->channels_.find(channel);
  return it == this->channels_.end() ? NAN : it->second.floor_min_c;
}
float WavinAHC9000::get_channel_floor_max_temp(uint8_t channel) const {
  auto it = this->channels_.find(channel);
  return it == this->channels_.end() ? NAN : it->second.floor_max_c;
}
climate::ClimateMode WavinAHC9000::get_channel_mode(uint8_t channel) const {
  auto it = this->channels_.find(channel);
  return it == this->channels_.end() ? climate::CLIMATE_MODE_HEAT : it->second.mode;
}
climate::ClimateAction WavinAHC9000::get_channel_action(uint8_t channel) const {
  auto it = this->channels_.find(channel);
  return it == this->channels_.end() ? climate::CLIMATE_ACTION_OFF : it->second.action;
}

void WavinZoneClimate::dump_config() { LOG_CLIMATE("  ", "Wavin Zone Climate (minimal)", this); }
climate::ClimateTraits WavinZoneClimate::traits() {
  climate::ClimateTraits t;
  t.set_supported_modes({climate::CLIMATE_MODE_HEAT, climate::CLIMATE_MODE_OFF});
  t.set_supports_current_temperature(true);
  t.set_supports_action(true);
  // Default visual bounds
  float vmin = 5.0f;
  float vmax = 35.0f;
  // For comfort climates (using floor temperature), adopt current floor min/max when available
  if (this->single_channel_set_ && this->use_floor_temperature_) {
    t.set_supports_two_point_target_temperature(true);
    float fmin = this->parent_->get_channel_floor_min_temp(this->single_channel_);
    float fmax = this->parent_->get_channel_floor_max_temp(this->single_channel_);
    if (!std::isnan(fmin)) vmin = fmin;
    if (!std::isnan(fmax)) vmax = fmax;
  }
  t.set_visual_min_temperature(vmin);
  t.set_visual_max_temperature(vmax);
  t.set_visual_temperature_step(0.5f);
  return t;
}
void WavinZoneClimate::control(const climate::ClimateCall &call) {
  // Mode control
  if (call.get_mode().has_value()) {
    auto m = *call.get_mode();
  ESP_LOGD(TAG, "CTRL: mode=%s for %s", (m == climate::CLIMATE_MODE_OFF ? "OFF" : "HEAT"), this->get_name().c_str());
  if (this->parent_->get_allow_mode_writes()) {
      if (this->single_channel_set_) {
        this->parent_->write_channel_mode(this->single_channel_, m);
      } else if (!this->members_.empty()) {
        for (auto ch : this->members_) this->parent_->write_channel_mode(ch, m);
      }
    } else {
      ESP_LOGW(TAG, "Mode writes disabled by config; skipping write for %s", this->get_name().c_str());
    }
    this->mode = (m == climate::CLIMATE_MODE_OFF) ? climate::CLIMATE_MODE_OFF : climate::CLIMATE_MODE_HEAT;
  }

  // Target temperature
  if (call.get_target_temperature().has_value()) {
    float t = *call.get_target_temperature();
  ESP_LOGD(TAG, "CTRL: target=%.1fC for %s", t, this->get_name().c_str());
    // If we're turning OFF in the same call, skip setpoint write to avoid switching back to MANUAL
    bool turning_off = call.get_mode().has_value() && (*call.get_mode() == climate::CLIMATE_MODE_OFF);
    if (!turning_off) {
      if (this->single_channel_set_) {
        this->parent_->write_channel_setpoint(this->single_channel_, t);
      } else if (!this->members_.empty()) {
        this->parent_->write_group_setpoint(this->members_, t);
      }
    }
    this->target_temperature = t;
  }

  // If comfort climate is used, expose floor min/max through target_temperature_low/high
  if (this->use_floor_temperature_ && this->single_channel_set_) {
    bool has_lo = call.get_target_temperature_low().has_value();
    bool has_hi = call.get_target_temperature_high().has_value();
    float current_lo = this->parent_->get_channel_floor_min_temp(this->single_channel_);
    float current_hi = this->parent_->get_channel_floor_max_temp(this->single_channel_);
    float new_lo = current_lo;
    float new_hi = current_hi;
    if (has_lo) new_lo = *call.get_target_temperature_low();
    if (has_hi) new_hi = *call.get_target_temperature_high();
    auto round05 = [](float v) -> float { return std::round(v * 2.0f) / 2.0f; };
    // Clamp to global sane bounds first, then round to 0.5°C step
    if (!std::isnan(new_lo)) {
      if (new_lo < 5.0f) new_lo = 5.0f;
      if (new_lo > 35.0f) new_lo = 35.0f;
      new_lo = round05(new_lo);
    }
    if (!std::isnan(new_hi)) {
      if (new_hi < 5.0f) new_hi = 5.0f;
      if (new_hi > 35.0f) new_hi = 35.0f;
      new_hi = round05(new_hi);
    }
    // Enforce at least 1.0C separation if both present (or infer using the unchanged side)
    if (!std::isnan(new_lo) && !std::isnan(new_hi)) {
      if (new_hi < new_lo + 1.0f) {
        if (has_hi && !has_lo) {
          new_lo = round05(new_hi - 1.0f);
        } else {
          new_hi = round05(new_lo + 1.0f);
        }
      }
    } else if (!std::isnan(new_lo) && std::isnan(new_hi) && !std::isnan(current_hi)) {
      if (current_hi < new_lo + 1.0f) new_lo = round05(current_hi - 1.0f);
    } else if (!std::isnan(new_hi) && std::isnan(new_lo) && !std::isnan(current_lo)) {
      if (new_hi < current_lo + 1.0f) new_hi = round05(current_lo + 1.0f);
    }
    // Write only the values that actually changed after adjustment
    if (!std::isnan(new_lo) && (std::isnan(current_lo) || std::fabs(new_lo - current_lo) > 0.049f)) {
      ESP_LOGD(TAG, "CTRL: floor min(write)=%.1fC (req%s) for %s", new_lo, has_lo?" set":" implied", this->get_name().c_str());
      this->parent_->write_channel_floor_min_temperature(this->single_channel_, new_lo);
    }
    if (!std::isnan(new_hi) && (std::isnan(current_hi) || std::fabs(new_hi - current_hi) > 0.049f)) {
      ESP_LOGD(TAG, "CTRL: floor max(write)=%.1fC (req%s) for %s", new_hi, has_hi?" set":" implied", this->get_name().c_str());
      this->parent_->write_channel_floor_max_temperature(this->single_channel_, new_hi);
    }
  }

  this->publish_state();
}
void WavinZoneClimate::update_from_parent() {
  if (this->single_channel_set_) {
    uint8_t ch = this->single_channel_;
    if (this->use_floor_temperature_) {
      this->current_temperature = this->parent_->get_channel_floor_temp(ch);
    } else {
      this->current_temperature = this->parent_->get_channel_current_temp(ch);
    }
    this->target_temperature = this->parent_->get_channel_setpoint(ch);
    // For comfort climates, surface floor min/max as low/high targets
    if (this->use_floor_temperature_) {
      float fmin = this->parent_->get_channel_floor_min_temp(ch);
      float fmax = this->parent_->get_channel_floor_max_temp(ch);
      if (!std::isnan(fmin)) this->target_temperature_low = fmin;
      if (!std::isnan(fmax)) this->target_temperature_high = fmax;
    }
    this->mode = this->parent_->get_channel_mode(ch);
    // Action: derive from temperatures with a small deadband, fallback to controller bit
    const float db = 0.3f;  // hysteresis in °C
    auto raw_action = this->parent_->get_channel_action(ch);
    if (!std::isnan(this->current_temperature) && !std::isnan(this->target_temperature)) {
      if (this->current_temperature > this->target_temperature + db) {
        this->action = climate::CLIMATE_ACTION_IDLE;
      } else if (this->current_temperature < this->target_temperature - db) {
        this->action = climate::CLIMATE_ACTION_HEATING;
      } else {
        this->action = raw_action;
      }
    } else {
      this->action = raw_action;
    }
  } else if (!this->members_.empty()) {
    float sum_curr = 0.0f, sum_set = 0.0f;
    int n_curr = 0;
    bool any_heat = false;
    bool all_off = true;
    for (auto ch : this->members_) {
      float c = this->parent_->get_channel_current_temp(ch);
      if (!std::isnan(c)) {
        sum_curr += c;
        n_curr++;
      }
      float s = this->parent_->get_channel_setpoint(ch);
      if (!std::isnan(s)) sum_set += s;
      if (this->parent_->get_channel_action(ch) == climate::CLIMATE_ACTION_HEATING) any_heat = true;
      if (this->parent_->get_channel_mode(ch) != climate::CLIMATE_MODE_OFF) all_off = false;
    }
    if (n_curr > 0) this->current_temperature = sum_curr / n_curr;
    if (!this->members_.empty()) this->target_temperature = sum_set / this->members_.size();
    this->mode = all_off ? climate::CLIMATE_MODE_OFF : climate::CLIMATE_MODE_HEAT;
    // Group action: prefer temperature comparison with deadband, fallback to any member heating
    const float db = 0.3f;
    if (!std::isnan(this->current_temperature) && !std::isnan(this->target_temperature)) {
      if (this->current_temperature > this->target_temperature + db) {
        this->action = climate::CLIMATE_ACTION_IDLE;
      } else if (this->current_temperature < this->target_temperature - db) {
        this->action = climate::CLIMATE_ACTION_HEATING;
      } else {
        this->action = any_heat ? climate::CLIMATE_ACTION_HEATING : climate::CLIMATE_ACTION_IDLE;
      }
    } else {
      this->action = any_heat ? climate::CLIMATE_ACTION_HEATING : climate::CLIMATE_ACTION_IDLE;
    }
  }
  this->publish_state();
}

void WavinRepairButton::press_action() {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG, "Repair button pressed but parent hub is missing");
    return;
  }
  if (this->channel_ < 1 || this->channel_ > 16) {
    ESP_LOGW(TAG, "Repair button channel %u is out of range", (unsigned) this->channel_);
    return;
  }
  ESP_LOGI(TAG, "Repair button pressed for channel %u", (unsigned) this->channel_);

  if (this->normalize_ || this->normalize_off_) {
    bool off = this->normalize_off_;
    ESP_LOGI(TAG, "Normalize request: channel=%u off=%s", (unsigned) this->channel_, off ? "true" : "false");
    this->parent_->normalize_channel_config(this->channel_, off);
  }

  if (this->extended_) {
    ESP_LOGI(TAG, "Enabling strict mode writes for channel %u", (unsigned) this->channel_);
    this->parent_->set_strict_mode_write(this->channel_, true);
  }

  if (this->aggressive_) {
    ESP_LOGI(TAG, "Aggressive refresh requested for channel %u", (unsigned) this->channel_);
    this->parent_->refresh_channel_now(this->channel_);
  } else {
    this->parent_->request_status_channel(this->channel_);
  }
}

void WavinYamlDumpButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->generate_yaml_suggestion();
  } else {
    ESP_LOGW(TAG, "YAML dump button pressed but parent hub is missing");
  }
}

}  // namespace wavin_ahc9000
}  // namespace esphome
