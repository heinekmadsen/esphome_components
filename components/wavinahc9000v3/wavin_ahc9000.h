#pragma once

#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/button/button.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <string>

namespace esphome {
namespace sensor { class Sensor; }
namespace switch_ { class Switch; }
namespace button { class Button; }
namespace text_sensor { class TextSensor; }
namespace wavinahc9000v3 {

// Forward
class WavinZoneClimate;
class WavinChildLockSwitch;
class WavinYamlDumpButton;
class WavinRepairButton;
class WavinZoneThermostat;
class WavinZoneEnableSwitch;

// ---------------------------------------------------------------------------
// Runtime zone layer: secondary temperature feedback per channel/room.
//
// A zone pairs one or more AHC-9000 channels with an external temperature that
// Home Assistant pushes in. While the zone is enabled, the channel setpoint is
// biased by (target - external) so the AHC-9000's own control loop ends up
// regulating on your sensor instead of its thermostat. Disable the zone and the
// plain target is written back, handing control straight back to the original
// Wavin thermostat.
//
// Zones are commissioned at runtime from HA and stored in NVS, so adding or
// re-mapping a room needs no YAML edit and no recompile.
// ---------------------------------------------------------------------------

// Fixed pool size. Entities are compile-time in ESPHome, so the pool is compiled once and
// individual slots are populated at runtime from Home Assistant.
static constexpr uint8_t WAVIN_MAX_ZONES = 16;
static constexpr uint8_t WAVIN_ZONE_NAME_LEN = 24;

// Persisted-to-NVS zone configuration. POD only, no pointers/std::string, so it can be
// written as a flat blob. Fast-changing runtime state deliberately lives in ZoneRuntime
// instead, so it never triggers a flash write.
struct ZoneConfigPod {
  uint16_t channel_mask;             // bit0 = channel 1 ... bit15 = channel 16
  float target_c;
  uint8_t enabled;
  char name[WAVIN_ZONE_NAME_LEN];
} __attribute__((packed));

struct ZonesBlob {
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  ZoneConfigPod zones[WAVIN_MAX_ZONES];
} __attribute__((packed));

static constexpr uint32_t WAVIN_ZONES_MAGIC = 0x5A4E5731;  // "ZNW1"
// Bumped whenever ZonesBlob changes shape. A mismatch reinitialises the blob,
// so saved targets fall back to their defaults once rather than being misread.
static constexpr uint16_t WAVIN_ZONES_VERSION = 2;

// Volatile per-zone state. Never persisted.
//
// Note that what has actually been written is tracked per *channel*, not per
// zone: a room can span several channels (a joined room like 3+4), each with
// its own thermostat reading and therefore its own computed setpoint.
struct ZoneRuntime {
  float external_temp_c{NAN};
  uint32_t last_seen_ms{0};
  bool ever_seen{false};
  bool failsafe{false};
};

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
  // Overrides the silence the module profile would enforce between
  // transactions. Applied after setup() has computed the profile defaults.
  void set_inter_frame_delay_us(uint32_t us) {
    this->inter_frame_delay_us_ = us;
    this->inter_frame_override_ = true;
  }
  void set_tx_enable_pin(GPIOPin *p) { this->tx_enable_pin_ = p; }
  // Optional half-duplex RS485 DE/RE (flow control) pin. If provided we drive HIGH to transmit and LOW to receive.
  void set_flow_control_pin(GPIOPin *p) { this->flow_control_pin_ = p; }
  void set_module_profile(ModuleProfile profile) { this->module_profile_ = profile; }
  void set_poll_channels_per_cycle(uint8_t n) { this->poll_channels_per_cycle_ = n == 0 ? 1 : (n > 16 ? 16 : n); }
  void set_allow_mode_writes(bool v) { this->allow_mode_writes_ = v; }
  void set_keep_standby_alive(bool v);
  void set_standby_keepalive_interval(uint32_t ms);
  bool get_allow_mode_writes() const { return this->allow_mode_writes_; }
  // Friendly name support (optional per-channel overrides for generated YAML)
  void set_channel_friendly_name(uint8_t channel, const std::string &name);
  std::string get_channel_friendly_name(uint8_t channel) const;

  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;
  void on_shutdown() override;

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

  // --- Zone layer configuration ----------------------------------------
  void set_sensor_timeout(uint32_t ms) { this->sensor_timeout_ms_ = ms; }
  void set_min_write_interval(uint32_t ms) { this->min_write_interval_ms_ = ms; }
  void set_setpoint_deadband(float c) { this->setpoint_deadband_c_ = c; }
  void set_setpoint_bounds(float lo, float hi) {
    this->min_setpoint_c_ = lo;
    this->max_setpoint_c_ = hi;
  }

  // --- Commissioning API -------------------------------------------------
  // These only mutate ESP32 state - they never emit a Modbus write, so a
  // mis-click in the HA UI cannot disturb the heating.
  bool zone_configure(uint8_t zone, const std::string &name, const std::string &channels_csv, bool enabled);
  // Room defined in the YAML: channels and name come from the config, while the
  // target temperature and on/off state are kept from NVS so the user's setting
  // survives a reboot or a reflash.
  void zone_set_static(uint8_t zone, uint16_t channel_mask, const std::string &name);
  bool zone_set_enabled(uint8_t zone, bool enabled);
  bool zone_clear(uint8_t zone);
  void zone_push_temperature(uint8_t zone, float celsius);
  void zone_push_unavailable(uint8_t zone);
  bool zone_set_target(uint8_t zone, float celsius);
  void discover_channels();

  // --- Zone accessors used by the bound entities -------------------------
  bool zone_is_enabled(uint8_t zone) const;
  float zone_get_target(uint8_t zone) const;
  float zone_get_temperature(uint8_t zone) const;
  bool zone_in_failsafe(uint8_t zone) const;
  climate::ClimateAction zone_get_action(uint8_t zone) const;
  std::string zone_get_name(uint8_t zone) const;
  std::string zone_get_channels_csv(uint8_t zone) const;

  // --- Entity binding ----------------------------------------------------
  void add_zone_climate(uint8_t zone, WavinZoneThermostat *c);
  void add_zone_enable_switch(uint8_t zone, switch_::Switch *s);
  void set_discovery_text_sensor(text_sensor::TextSensor *s) { this->discovery_text_sensor_ = s; }

  // Restore native thermostat control on every zone (clean reboot / OTA).
  void release_all_zones();

 protected:
  // Low-level protocol helpers (dkjonas framing)
  bool read_registers(uint8_t category, uint8_t page, uint8_t index, uint8_t count, std::vector<uint16_t> &out);
  bool write_register(uint8_t category, uint8_t page, uint8_t index, uint16_t value);
  // Masked write: apply (reg & and_mask) | or_mask semantics
  bool write_masked_register(uint8_t category, uint8_t page, uint8_t index, uint16_t and_mask, uint16_t or_mask);

  void publish_updates();
  void prepare_for_tx_();
  void finish_tx_();
  uint32_t clear_stale_rx_();
  // Reads one reply frame, resynchronising past leading noise. Returns true on
  // a CRC-valid frame of the expected length.
  bool receive_frame_(uint8_t function_code, uint8_t expected_payload_bytes, std::vector<uint8_t> &frame);
  void log_bus_stats_();
  // Reads an element page, reusing a recent read of the same page so channels
  // that share a thermostat do not each fetch it.
  bool read_element_page_(uint8_t elem_page, std::vector<uint16_t> &out);
  void handle_standby_keepalive_(uint8_t channel, bool is_off, std::vector<uint8_t> &reassert_list);
  // Inter-frame silence to ensure bus turnaround between consecutive transactions
  void inter_frame_delay_();
  bool standby_keepalive_enabled_() const {
    return this->keep_standby_alive_ && this->standby_keepalive_interval_ms_ > 0;
  }

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
  // Deadline for a reply to *start*. Once bytes arrive, rx_gap_us_ decides when
  // the frame has ended, so this only bounds how long a dead transaction costs.
  // Measured on an AHC-9000: replies start between 3 ms and 39 ms, so this is a
  // wide margin over the observed tail while still well under the old 350 ms,
  // which blocked the main loop for most of a second on a dead transaction.
  uint32_t receive_timeout_ms_{200};
  uint32_t suspend_polling_until_{0};
  GPIOPin *tx_enable_pin_{nullptr};
  GPIOPin *flow_control_pin_{nullptr};
  ModuleProfile module_profile_{ModuleProfile::MODULE_DEFAULT};
  uint8_t poll_channels_per_cycle_{2};
  uint8_t next_active_index_{0};
  uint8_t channel_step_[16] = {0};
  uint32_t primary_elem_checked_ms_[16] = {0};  // last CH_PRIMARY_ELEMENT read per channel
  // One element page can serve several channels, so a page read within this
  // window is reused rather than repeated on the bus.
  uint32_t element_read_ms_[16] = {0};
  uint16_t element_cache_[16][11] = {{0}};
  bool element_cache_valid_[16] = {false};
  static constexpr uint32_t PRIMARY_ELEM_RECHECK_MS = 300000;  // 5 minutes
  static constexpr uint32_t ELEMENT_CACHE_MS = 4000;
  std::vector<uint8_t> urgent_channels_{}; // channels scheduled for immediate refresh on next update
  bool allow_mode_writes_{true};
  uint32_t pre_tx_delay_us_{0};
  uint32_t post_tx_guard_us_{300};
  uint32_t inter_frame_delay_us_{0};  // silence between consecutive transactions, set by setup()
  bool inter_frame_override_{false};  // true when the YAML pinned it
  bool flush_rx_before_tx_{false};
  // True only when the transceiver keeps receiving while it transmits, i.e.
  // a bare tx_enable_pin. A combined DE/RE flow-control pin mutes the receiver.
  bool expect_echo_{false};
  uint8_t tx_len_{0};        // bytes in the frame just sent, bounds the echo drain
  uint32_t tx_done_us_{0};   // when the driver was released, for reply timing
  uint32_t rx_gap_us_{3000}; // silence that ends a reply, set from the baud rate

  // --- bus diagnostics -------------------------------------------------
  // Counted for every transaction so the health of the RS-485 link can be
  // measured rather than guessed at. Reported by log_bus_stats_().
  struct BusStats {
    uint32_t transactions{0};     // transactions started
    uint32_t retries{0};          // attempts beyond the first
    uint32_t failures{0};         // transactions that gave up entirely
    uint32_t timeouts{0};         // attempts that saw no usable frame
    uint32_t crc_errors{0};       // attempts that saw a frame with a bad CRC
    uint32_t resyncs{0};          // leading bytes dropped to find a header
    uint32_t discarded_pre_tx{0}; // stale bytes dropped before transmitting
    uint32_t discarded_post_tx{0};// bytes dropped by the post-TX flush
    uint32_t reply_us_min{0xFFFFFFFF};
    uint32_t reply_us_max{0};
    uint64_t reply_us_sum{0};
    uint32_t reply_count{0};
  } bus_;
  uint32_t bus_log_due_{0};
  static constexpr uint32_t BUS_STATS_INTERVAL_MS = 300000;  // 5 minutes

  bool keep_standby_alive_{false};
  uint32_t standby_keepalive_interval_ms_{180000};
  std::map<uint8_t, uint32_t> standby_keepalive_deadlines_;

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

  // --- Zone layer internals -------------------------------------------------
  void zones_load_();
  void zones_save_();
  void zones_tick_();
  void zone_prune_unbound_();
  void zone_apply_(uint8_t zone);
  void zone_release_(uint8_t zone);
  void zone_publish_(uint8_t zone);
  bool zone_validate_(uint8_t zone, uint16_t mask, const std::string &name) const;
  bool parse_channel_csv_(const std::string &csv, uint16_t &mask_out) const;
  std::string mask_to_csv_(uint16_t mask) const;

  ZonesBlob zones_{};
  ZoneRuntime zone_rt_[WAVIN_MAX_ZONES]{};
  ESPPreferenceObject zones_pref_;
  bool zones_pruned_{false};

  // What we last wrote, per channel (index 1..16). A channel belongs to at
  // most one zone, so one flat array is enough and every channel in a joined
  // room gets written independently.
  float ch_applied_setpoint_[17]{};
  uint32_t ch_last_write_ms_[17]{};
  bool ch_applied_valid_[17]{};
  void zone_reset_channel_state_(uint8_t zone);

  uint32_t sensor_timeout_ms_{1800000};     // 30 min without a push -> hand back
  uint32_t min_write_interval_ms_{600000};  // 10 min; guards the AHC-9000's EEPROM
  float setpoint_deadband_c_{0.3f};
  float min_setpoint_c_{5.0f};
  float max_setpoint_c_{35.0f};

  std::map<uint8_t, WavinZoneThermostat *> zone_climates_;
  std::map<uint8_t, switch_::Switch *> zone_enable_switches_;
  text_sensor::TextSensor *discovery_text_sensor_{nullptr};
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

// ---------------------------------------------------------------------------
// Zone-bound entities. One compiled pool; slots populated at runtime.
// ---------------------------------------------------------------------------

class WavinZoneThermostat : public climate::Climate, public Component {
 public:
  void set_parent(WavinAHC9000 *p) { this->parent_ = p; }
  void set_zone(uint8_t z) { this->zone_ = z; }
  uint8_t get_zone() const { return this->zone_; }
  void set_visual_range(float lo, float hi) {
    this->visual_min_ = lo;
    this->visual_max_ = hi;
  }
  // Channels from the YAML (bit0 = channel 1). 0 means "commission at runtime".
  void set_static_channels(uint16_t mask) { this->static_mask_ = mask; }
  // Home Assistant entity this room follows. ESPHome subscribes to it over the
  // native API, so nothing has to be configured on the HA side.
  void set_source_entity(const std::string &e) { this->source_entity_ = e; }

  void setup() override;
  float get_setup_priority() const override;
  void update_from_parent();
  void dump_config() override;

 protected:
  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;

  WavinAHC9000 *parent_{nullptr};
  uint8_t zone_{0};
  float visual_min_{5.0f};
  float visual_max_{35.0f};
  uint16_t static_mask_{0};
  std::string source_entity_{};
};

class WavinZoneEnableSwitch : public switch_::Switch {
 public:
  void set_parent(WavinAHC9000 *p) { this->parent_ = p; }
  void set_zone(uint8_t z) { this->zone_ = z; }

 protected:
  void write_state(bool state) override;
  WavinAHC9000 *parent_{nullptr};
  uint8_t zone_{0};
};

}  // namespace wavinahc9000v3
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
