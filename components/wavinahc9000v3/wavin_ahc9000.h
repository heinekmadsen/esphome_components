#pragma once

#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/button/button.h"
#include "esphome/core/component.h"

#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <string>

namespace esphome {
namespace sensor { class Sensor; }
namespace switch_ { class Switch; }
namespace button { class Button; }
namespace wavin_ahc9000 {

// Forward
class WavinZoneClimate;
class WavinChildLockSwitch;
class WavinYamlDumpButton;
class WavinRepairButton;

enum class ModuleProfile : uint8_t {
  MODULE_DEFAULT = 0,
  MODULE_USTEPPER = 1,
};

static constexpr ModuleProfile MODULE_DEFAULT = ModuleProfile::MODULE_DEFAULT;
static constexpr ModuleProfile MODULE_USTEPPER = ModuleProfile::MODULE_USTEPPER;

class WavinAHC9000 : public PollingComponent, public uart::UARTDevice {
 public:
  void set_temp_divisor(float d) { this->temp_divisor_ = d; }
  void set_receive_timeout_ms(uint32_t t) { this->receive_timeout_ms_ = t; }
  void set_tx_enable_pin(GPIOPin *p) { this->tx_enable_pin_ = p; }
  // Optional half-duplex RS485 DE/RE (flow control) pin. If provided we drive HIGH to transmit and LOW to receive.
  void set_flow_control_pin(GPIOPin *p) { this->flow_control_pin_ = p; }
  void set_module_profile(ModuleProfile profile) { this->module_profile_ = profile; }
  void set_poll_channels_per_cycle(uint8_t n) { this->poll_channels_per_cycle_ = n == 0 ? 1 : (n > 16 ? 16 : n); }
  void set_allow_mode_writes(bool v) { this->allow_mode_writes_ = v; }
  bool get_allow_mode_writes() const { return this->allow_mode_writes_; }
  // Friendly name support (optional per-channel overrides for generated YAML)
  void set_channel_friendly_name(uint8_t channel, const std::string &name);
  std::string get_channel_friendly_name(uint8_t channel) const;

  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;

  void add_channel_climate(WavinZoneClimate *c);
  void add_group_climate(WavinZoneClimate *c);
  void add_channel_battery_sensor(uint8_t ch, sensor::Sensor *s);
  void add_channel_temperature_sensor(uint8_t ch, sensor::Sensor *s);
  void add_channel_comfort_setpoint_sensor(uint8_t ch, sensor::Sensor *s);
  void add_channel_floor_temperature_sensor(uint8_t ch, sensor::Sensor *s);
  // New read-only floor limit sensors
  void add_channel_floor_min_temperature_sensor(uint8_t ch, sensor::Sensor *s);
  void add_channel_floor_max_temperature_sensor(uint8_t ch, sensor::Sensor *s);
  void add_channel_child_lock_switch(uint8_t ch, switch_::Switch *s) { this->child_lock_switches_[ch] = s; }
  void add_active_channel(uint8_t ch);

  // Send commands
  void write_channel_setpoint(uint8_t channel, float celsius);
  void write_group_setpoint(const std::vector<uint8_t> &members, float celsius);
  void write_channel_mode(uint8_t channel, climate::ClimateMode mode);
  void write_channel_child_lock(uint8_t channel, bool enable);
  // Write floor temperature limits (Celsius), clamped to sane bounds
  void write_channel_floor_min_temperature(uint8_t channel, float celsius);
  void write_channel_floor_max_temperature(uint8_t channel, float celsius);
  void refresh_channel_now(uint8_t channel);
  void set_strict_mode_write(uint8_t channel, bool enable);
  bool is_strict_mode_write(uint8_t channel) const;
  void request_status();
  void request_status_channel(uint8_t ch_index);
  void normalize_channel_config(uint8_t channel, bool off);
  void generate_yaml_suggestion();
  // Debug helper to dump registers for a channel (to identify floor min/max addresses)
  void dump_channel_floor_limits(uint8_t channel);
  bool is_channel_child_locked(uint8_t ch) const {
    auto it = this->channels_.find(ch);
    if (it == this->channels_.end()) return false;
    return it->second.child_lock;
  }

  // Data access
  float get_channel_current_temp(uint8_t channel) const;
  float get_channel_setpoint(uint8_t channel) const;
  float get_channel_floor_temp(uint8_t channel) const;
  float get_channel_floor_min_temp(uint8_t channel) const;
  float get_channel_floor_max_temp(uint8_t channel) const;
  climate::ClimateMode get_channel_mode(uint8_t channel) const;
  climate::ClimateAction get_channel_action(uint8_t channel) const;

 protected:
  // Low-level protocol helpers (dkjonas framing)
  bool read_registers(uint8_t category, uint8_t page, uint8_t index, uint8_t count, std::vector<uint16_t> &out);
  bool write_register(uint8_t category, uint8_t page, uint8_t index, uint16_t value);
  // Masked write: apply (reg & and_mask) | or_mask semantics
  bool write_masked_register(uint8_t category, uint8_t page, uint8_t index, uint16_t and_mask, uint16_t or_mask);

  void publish_updates();
  void prepare_for_tx_();
  void finish_tx_();
  void clear_stale_rx_();

  // Helpers
  float raw_to_c(float raw) const { return raw / this->temp_divisor_; }
  uint16_t c_to_raw(float c) const { return static_cast<uint16_t>(c * this->temp_divisor_ + 0.5f); }

  // Simple cache per channel
  struct ChannelState {
    float current_temp_c{NAN};
    float floor_temp_c{NAN};
    // New read-only floor limits (Celsius)
    float floor_min_c{NAN};
    float floor_max_c{NAN};
    float setpoint_c{NAN};
    climate::ClimateMode mode{climate::CLIMATE_MODE_HEAT};
    climate::ClimateAction action{climate::CLIMATE_ACTION_OFF};
    uint8_t battery_pct{255}; // 0..100; 255=unknown
    uint16_t primary_index{0};
    bool all_tp_lost{false};
    bool has_floor_sensor{false};
    bool child_lock{false};
  };

  std::map<uint8_t, ChannelState> channels_;
  std::vector<WavinZoneClimate *> single_ch_climates_;
  std::vector<WavinZoneClimate *> group_climates_;
  std::map<uint8_t, sensor::Sensor *> battery_sensors_;
  std::map<uint8_t, sensor::Sensor *> temperature_sensors_;
  std::map<uint8_t, sensor::Sensor *> floor_temperature_sensors_;
  // New read-only floor limit sensor maps
  std::map<uint8_t, sensor::Sensor *> floor_min_temperature_sensors_;
  std::map<uint8_t, sensor::Sensor *> floor_max_temperature_sensors_;
  std::map<uint8_t, sensor::Sensor *> comfort_setpoint_sensors_;
  std::map<uint8_t, switch_::Switch *> child_lock_switches_;
  std::vector<std::string> channel_friendly_names_; // 1-based index mapping (size >=17)
  std::vector<uint8_t> active_channels_;
  std::map<uint8_t, climate::ClimateMode> desired_mode_; // desired mode to reconcile after refresh
  std::set<uint8_t> strict_mode_channels_; // channels opting into strict baseline writes

  float temp_divisor_{10.0f};
  uint32_t last_poll_ms_{0};
  uint32_t receive_timeout_ms_{1000};
  uint32_t suspend_polling_until_{0};
  GPIOPin *tx_enable_pin_{nullptr};
  GPIOPin *flow_control_pin_{nullptr};
  ModuleProfile module_profile_{ModuleProfile::MODULE_DEFAULT};
  uint8_t poll_channels_per_cycle_{2};
  uint8_t next_active_index_{0};
  uint8_t channel_step_[16] = {0};
  std::vector<uint8_t> urgent_channels_{}; // channels scheduled for immediate refresh on next update
  bool allow_mode_writes_{true};
  uint32_t pre_tx_delay_us_{0};
  uint32_t post_tx_guard_us_{300};
  bool flush_rx_before_tx_{false};

  // Protocol constants
  static constexpr uint8_t DEVICE_ADDR = 0x01;
  static constexpr uint8_t FC_READ = 0x43;
  static constexpr uint8_t FC_WRITE = 0x44;
  static constexpr uint8_t FC_WRITE_MASKED = 0x45;

  // Categories & indices (from dkjonas repo)
  static constexpr uint8_t CAT_CHANNELS = 0x03;
  static constexpr uint8_t CAT_ELEMENTS = 0x01;
  static constexpr uint8_t CAT_PACKED = 0x02;

  static constexpr uint8_t CH_TIMER_EVENT = 0x00; // status incl. output bit
  static constexpr uint16_t CH_TIMER_EVENT_OUTP_ON_MASK = 0x0010;
  static constexpr uint8_t CH_PRIMARY_ELEMENT = 0x02;
  static constexpr uint16_t CH_PRIMARY_ELEMENT_ELEMENT_MASK = 0x003f;
  static constexpr uint16_t CH_PRIMARY_ELEMENT_ALL_TP_LOST_MASK = 0x0400;

  static constexpr uint8_t ELEM_AIR_TEMPERATURE = 0x04; // index within block
  static constexpr uint8_t ELEM_FLOOR_TEMPERATURE = 0x05; // index for floor probe
  static constexpr uint8_t ELEM_BATTERY_STATUS = 0x0A;  // not used yet

  static constexpr uint8_t PACKED_MANUAL_TEMPERATURE = 0x00;
  static constexpr uint8_t PACKED_STANDBY_TEMPERATURE = 0x04;
  static constexpr uint8_t PACKED_CONFIGURATION = 0x07;
  // Inferred from field dump: floor min/max setpoints exposed in PACKED page
  static constexpr uint8_t PACKED_FLOOR_MIN_TEMPERATURE = 0x0A; // 21.5C example
  static constexpr uint8_t PACKED_FLOOR_MAX_TEMPERATURE = 0x0B; // 25.5C example
  // Note: PACKED_FLOOR_MIN_TEMPERATURE and PACKED_FLOOR_MAX_TEMPERATURE are contiguous; reads
  // have been consolidated (count=2 starting at MIN) to reduce RS485 transactions.
  static constexpr uint16_t PACKED_CONFIGURATION_MODE_MASK = 0x07;
  static constexpr uint16_t PACKED_CONFIGURATION_MODE_MANUAL = 0x00;
  static constexpr uint16_t PACKED_CONFIGURATION_MODE_STANDBY = 0x01;
  static constexpr uint16_t PACKED_CONFIGURATION_MODE_STANDBY_ALT = 0x04; // fallback for variant firmwares
  static constexpr uint16_t PACKED_CONFIGURATION_PROGRAM_BIT = 0x0008; // suspected schedule/program flag
  static constexpr uint16_t PACKED_CONFIGURATION_PROGRAM_MASK = 0x0018; // extended clear: bits 3 and 4
  static constexpr uint16_t PACKED_CONFIGURATION_STRICT_UNLOCK_MASK = 0x0078; // bits 3..6 (avoid touching mode bits 0..2)
  static constexpr uint16_t PACKED_CONFIGURATION_CHILD_LOCK_MASK = 0x0800; // child lock bit (0x4000->0x4800)

  // I/O reliability: number of attempts for read/write before escalating to WARN
  static constexpr uint8_t IO_RETRY_ATTEMPTS = 2; // first failure logged at DEBUG, final at WARN
};

// Simple dedicated switch subclass for child lock control. Avoids relying on codegen lambdas
// that reference a specific hub variable name. The state is optimistic; an urgent refresh
// scheduled by write_channel_child_lock() will reconcile if the write failed.
class WavinRepairButton : public button::Button {
 public:
  void set_parent(WavinAHC9000 *p) { this->parent_ = p; }
  void set_channel(uint8_t ch) { this->channel_ = ch; }
  void set_extended(bool v) { this->extended_ = v; }
  void set_aggressive(bool v) { this->aggressive_ = v; }
  void set_normalize(bool v) { this->normalize_ = v; }
  void set_normalize_off(bool v) { this->normalize_off_ = v; }

 protected:
  void press_action() override;

  WavinAHC9000 *parent_{nullptr};
  uint8_t channel_{0};
  bool extended_{false};
  bool aggressive_{false};
  bool normalize_{false};
  bool normalize_off_{false};
};

class WavinChildLockSwitch : public switch_::Switch {
 public:
  void set_parent(WavinAHC9000 *p) { this->parent_ = p; }
  void set_channel(uint8_t ch) { this->channel_ = ch; }
 protected:
  void write_state(bool state) override {
    if (this->parent_ != nullptr) {
      this->parent_->write_channel_child_lock(this->channel_, state);
    }
    // Optimistic publish; hub publish_updates() will correct after refresh.
    this->publish_state(state);
  }
  WavinAHC9000 *parent_{nullptr};
  uint8_t channel_{0};
};

class WavinYamlDumpButton : public button::Button {
 public:
  void set_parent(WavinAHC9000 *p) { this->parent_ = p; }

 protected:
  void press_action() override;

  WavinAHC9000 *parent_{nullptr};
};

// Inline helpers for configuring sensors
inline void WavinAHC9000::add_channel_battery_sensor(uint8_t ch, sensor::Sensor *s) {
  this->battery_sensors_[ch] = s;
}

inline void WavinAHC9000::add_channel_temperature_sensor(uint8_t ch, sensor::Sensor *s) {
  this->temperature_sensors_[ch] = s;
}

inline void WavinAHC9000::add_channel_comfort_setpoint_sensor(uint8_t ch, sensor::Sensor *s) {
  this->comfort_setpoint_sensors_[ch] = s;
}

inline void WavinAHC9000::add_channel_floor_temperature_sensor(uint8_t ch, sensor::Sensor *s) {
  this->floor_temperature_sensors_[ch] = s;
}

inline void WavinAHC9000::add_channel_floor_min_temperature_sensor(uint8_t ch, sensor::Sensor *s) {
  this->floor_min_temperature_sensors_[ch] = s;
}

inline void WavinAHC9000::add_channel_floor_max_temperature_sensor(uint8_t ch, sensor::Sensor *s) {
  this->floor_max_temperature_sensors_[ch] = s;
}

// numeric yaml_ready sensor removed

class WavinZoneClimate : public climate::Climate, public Component {
 public:
  void set_parent(WavinAHC9000 *p) { this->parent_ = p; }
  void set_single_channel(uint8_t ch) {
    this->single_channel_ = ch;
    this->single_channel_set_ = true;
    this->members_.clear();
  }
  void set_use_floor_temperature(bool v) { this->use_floor_temperature_ = v; }
  void set_members(const std::vector<int> &members) {
    this->members_.clear();
    for (int m : members) this->members_.push_back(static_cast<uint8_t>(m));
    this->single_channel_set_ = false;
  }

  void dump_config() override;

  void update_from_parent();

 protected:
  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;

  WavinAHC9000 *parent_{nullptr};
  uint8_t single_channel_{0};
  bool single_channel_set_{false};
  std::vector<uint8_t> members_{};
  bool use_floor_temperature_{false};
};

// Repair button removed; use API service to normalize

}  // namespace wavin_ahc9000
}  // namespace esphome

// --- Child lock extension placeholders (to integrate in subsequent patch) ---
// NOTE: Full integration attempted earlier but patching context mismatched. The following
// defines will be merged into the class on next edit cycle.
// Child lock bit observed: PACKED_CONFIGURATION (index 0x07) changes from 0x4000 to 0x4800 when enabled => bit 0x0800.
// Planned additions inside WavinAHC9000:
//   - bool is_channel_child_locked(uint8_t ch) const;
//   - void write_channel_child_lock(uint8_t ch, bool enable);
//   - ChannelState::bool child_lock; // per-channel cache
//   - std::map<uint8_t, switch_::Switch*> child_lock_switches_;
//   - static constexpr uint16_t PACKED_CONFIGURATION_CHILD_LOCK_MASK = 0x0800;
// Parsing: when reading PACKED_CONFIGURATION, set child_lock = (raw_cfg & mask) != 0.
// Writing: read-modify-write preserving mode bits and baseline 0x4000 prefix.
