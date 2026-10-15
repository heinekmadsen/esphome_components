// ---------------------------------------------------------------------------
// Secondary temperature feedback for the Wavin AHC-9000 / Jablotron AC-116.
//
// The AC-116 has no writable external-temperature input: AIR TEMPERATURE
// (cat 0x01 idx 0x04) is read-only and TEMPERATURE OFFSET was removed from the
// register map. So the external reading is applied indirectly, by biasing the
// channel setpoint:
//
//     written_setpoint = thermostat_air_temp + (target - external_temp)
//
// The AHC-9000 keeps running its own control loop, hysteresis, actuator PWM,
// pump/relay and frost logic - it just ends up chasing your sensor instead of
// its own thermostat. Nothing is overridden and nothing is deleted, so the
// original thermostat remains fully functional underneath.
//
// Disable a zone (or let its sensor go stale) and the plain target is written
// back: control returns to the Wavin thermostat immediately.
// ---------------------------------------------------------------------------

#include "wavin_ahc9000.h"
#include "wavin_zone_logic.h"

#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/components/text_sensor/text_sensor.h"
#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

#include <cstdio>
#include <cstring>
#include <cmath>

namespace esphome {
namespace wavinahc9000v3 {

static const char *const TAG = "wavinahc9000v3.zone";

static bool zone_index_ok(uint8_t zone) { return zone >= 1 && zone <= WAVIN_MAX_ZONES; }

// Thin wrappers over the unit-tested helpers in wavin_zone_logic.h.
std::string WavinAHC9000::mask_to_csv_(uint16_t mask) const { return mask_to_csv(mask); }

bool WavinAHC9000::parse_channel_csv_(const std::string &csv, uint16_t &mask_out) const {
  if (!parse_channel_csv(csv, mask_out)) {
    ESP_LOGW(TAG, "channel list '%s' is invalid (expect 1..16, comma separated, non-empty)", csv.c_str());
    return false;
  }
  return true;
}

// A channel may belong to at most one zone. This deliberately also covers
// zones that are currently switched off: otherwise you could quietly hand a
// channel to a second zone and then be unable to switch the first one back on.
bool WavinAHC9000::zone_validate_(uint8_t zone, uint16_t mask, const std::string &name) const {
  if (!zone_index_ok(zone)) {
    ESP_LOGW(TAG, "zone %u out of range 1..%u", zone, WAVIN_MAX_ZONES);
    return false;
  }
  if (mask == 0) {
    ESP_LOGW(TAG, "zone %u: channel list must not be empty", zone);
    return false;
  }
  if (name.empty()) {
    ESP_LOGW(TAG, "zone %u: name must not be empty", zone);
    return false;
  }
  for (uint8_t other = 1; other <= WAVIN_MAX_ZONES; other++) {
    if (other == zone) continue;
    // Only a slot that a climate entity is bound to in the *current* config
    // counts. A zone left behind in NVS by an earlier config (e.g. after rooms
    // were added/removed and slots were renumbered) must not permanently
    // reserve a live room's channels and block switching it back on.
    if (!this->zone_climates_.empty() && this->zone_climates_.find(other) == this->zone_climates_.end())
      continue;
    const auto &z = this->zones_.zones[other - 1];
    if (z.channel_mask == 0) continue;
    uint16_t clash = (uint16_t) (z.channel_mask & mask);
    if (clash != 0) {
      ESP_LOGW(TAG, "zone %u: channel(s) %s already used by zone %u ('%s')", zone,
               this->mask_to_csv_(clash).c_str(), other, z.name);
      return false;
    }
  }
  return true;
}

// --- persistence -----------------------------------------------------------

void WavinAHC9000::zones_load_() {
  this->zones_pref_ = global_preferences->make_preference<ZonesBlob>(WAVIN_ZONES_MAGIC);

  ZonesBlob loaded{};
  bool ok = this->zones_pref_.load(&loaded);
  if (ok && loaded.magic == WAVIN_ZONES_MAGIC && loaded.version == WAVIN_ZONES_VERSION) {
    this->zones_ = loaded;
    uint8_t n = 0;
    for (auto &z : this->zones_.zones)
      if (z.enabled) n++;
    ESP_LOGCONFIG(TAG, "Restored %u configured zone(s) from NVS", n);
    return;
  }
  if (ok) {
    ESP_LOGW(TAG, "Zone blob magic/version mismatch (magic=0x%08X ver=%u) - reinitialising",
             (unsigned) loaded.magic, (unsigned) loaded.version);
  }
  memset(&this->zones_, 0, sizeof(this->zones_));
  this->zones_.magic = WAVIN_ZONES_MAGIC;
  this->zones_.version = WAVIN_ZONES_VERSION;
  for (auto &z : this->zones_.zones) z.target_c = 21.0f;
}

// Compare-before-save. NVS lives in flash, and flash sectors only survive
// ~10k-100k erase cycles, so saving on every poll (~8600/day at a 10 s
// interval) would destroy the sector within weeks. Zone config only changes
// during commissioning, so a byte-compare guard makes wear a non-issue.
void WavinAHC9000::zones_save_() {
  ZonesBlob current{};
  if (this->zones_pref_.load(&current) && memcmp(&current, &this->zones_, sizeof(ZonesBlob)) == 0) {
    ESP_LOGV(TAG, "Zone config unchanged - skipping NVS write");
    return;
  }
  if (this->zones_pref_.save(&this->zones_)) {
    ESP_LOGD(TAG, "Zone config saved to NVS");
  } else {
    ESP_LOGW(TAG, "Failed to save zone config to NVS");
  }
}

// --- commissioning API (ESP32 state only; never touches Modbus) ------------

bool WavinAHC9000::zone_configure(uint8_t zone, const std::string &name, const std::string &channels_csv,
                                  bool enabled) {
  if (!zone_index_ok(zone)) {
    ESP_LOGW(TAG, "zone_configure: zone %u out of range 1..%u", zone, WAVIN_MAX_ZONES);
    return false;
  }
  uint16_t mask = 0;
  if (!this->parse_channel_csv_(channels_csv, mask)) return false;
  if (!this->zone_validate_(zone, mask, name)) return false;

  auto &z = this->zones_.zones[zone - 1];
  // Release first, so channels dropped from the mask get their plain target back.
  this->zone_release_(zone);

  z.channel_mask = mask;
  z.enabled = enabled ? 1 : 0;
  snprintf(z.name, WAVIN_ZONE_NAME_LEN, "%s", name.c_str());
  z.name[WAVIN_ZONE_NAME_LEN - 1] = '\0';
  if (std::isnan(z.target_c) || z.target_c <= 0.0f) z.target_c = 21.0f;

  this->zone_reset_channel_state_(zone);

  for (uint8_t ch = 1; ch <= 16; ch++)
    if (mask & (uint16_t) (1u << (ch - 1))) this->add_active_channel(ch);

  ESP_LOGI(TAG, "Zone %u configured: name='%s' channels=[%s] external=%s", zone, z.name,
           this->mask_to_csv_(mask).c_str(), enabled ? "ON" : "off");
  this->zones_save_();
  this->zone_publish_(zone);
  return true;
}

// Room defined in the YAML. The channels and the name are authoritative from
// the config; the target temperature and the on/off state are preserved from
// NVS so the user's setting survives a reboot or a reflash.
void WavinAHC9000::zone_set_static(uint8_t zone, uint16_t channel_mask, const std::string &name) {
  if (!zone_index_ok(zone) || channel_mask == 0) return;
  auto &z = this->zones_.zones[zone - 1];

  bool first_time = (z.channel_mask == 0);
  z.channel_mask = channel_mask;
  snprintf(z.name, WAVIN_ZONE_NAME_LEN, "%s", name.c_str());
  z.name[WAVIN_ZONE_NAME_LEN - 1] = '\0';
  if (first_time) {
    // A room written in the YAML starts on, at a sane target. After that the
    // user's own setting is kept in NVS and never overwritten from the config.
    z.enabled = 1;
    z.target_c = 21.0f;
  }
  this->zone_reset_channel_state_(zone);

  for (uint8_t ch = 1; ch <= 16; ch++)
    if (channel_mask & (uint16_t) (1u << (ch - 1))) this->add_active_channel(ch);

  ESP_LOGCONFIG(TAG, "Room '%s' -> channel(s) %s, target %.1f C, %s", z.name,
                this->mask_to_csv_(channel_mask).c_str(), z.target_c, z.enabled ? "on" : "off");
  this->zones_save_();
}

bool WavinAHC9000::zone_set_enabled(uint8_t zone, bool enabled) {
  if (!zone_index_ok(zone)) return false;
  auto &z = this->zones_.zones[zone - 1];
  if (z.channel_mask == 0) {
    ESP_LOGW(TAG, "zone %u has no channels - commission it first", zone);
    return false;
  }
  if (enabled && !this->zone_validate_(zone, z.channel_mask, z.name[0] ? z.name : "zone")) return false;
  if ((bool) z.enabled == enabled) return true;

  if (!enabled) this->zone_release_(zone);
  z.enabled = enabled ? 1 : 0;
  this->zone_reset_channel_state_(zone);
  ESP_LOGI(TAG, "Zone %u ('%s'): %s", zone, z.name,
           enabled ? "now using the external sensor" : "handed back to the Wavin thermostat");
  this->zones_save_();
  this->zone_publish_(zone);
  return true;
}

bool WavinAHC9000::zone_clear(uint8_t zone) {
  if (!zone_index_ok(zone)) return false;
  this->zone_release_(zone);
  auto &z = this->zones_.zones[zone - 1];
  memset(&z, 0, sizeof(z));
  z.target_c = 21.0f;
  this->zone_rt_[zone - 1] = ZoneRuntime{};
  ESP_LOGI(TAG, "Zone %u cleared", zone);
  this->zones_save_();
  this->zone_publish_(zone);
  return true;
}

void WavinAHC9000::zone_push_temperature(uint8_t zone, float celsius) {
  if (!zone_index_ok(zone)) return;
  if (std::isnan(celsius)) {
    this->zone_push_unavailable(zone);
    return;
  }
  auto &rt = this->zone_rt_[zone - 1];
  rt.external_temp_c = celsius;
  rt.last_seen_ms = millis();
  rt.ever_seen = true;
  if (rt.failsafe) {
    ESP_LOGI(TAG, "Zone %u: external sensor recovered (%.2f C)", zone, celsius);
    rt.failsafe = false;
    this->zone_reset_channel_state_(zone);  // re-apply the bias promptly
  }
  ESP_LOGV(TAG, "Zone %u: external temp %.2f C", zone, celsius);
  this->zone_publish_(zone);
}

// HA forwards unknown/unavailable explicitly, so control returns to the
// thermostat immediately instead of waiting out sensor_timeout.
void WavinAHC9000::zone_push_unavailable(uint8_t zone) {
  if (!zone_index_ok(zone)) return;
  auto &rt = this->zone_rt_[zone - 1];
  if (!rt.failsafe) {
    ESP_LOGW(TAG, "Zone %u: external sensor unavailable - handing back to the Wavin thermostat", zone);
    rt.failsafe = true;
    rt.external_temp_c = NAN;
    this->zone_release_(zone);
  }
  this->zone_publish_(zone);
}

bool WavinAHC9000::zone_set_target(uint8_t zone, float celsius) {
  if (!zone_index_ok(zone)) return false;
  if (std::isnan(celsius) || celsius < this->min_setpoint_c_ || celsius > this->max_setpoint_c_) {
    ESP_LOGW(TAG, "zone %u: target %.1f outside %.1f..%.1f", zone, celsius, this->min_setpoint_c_,
             this->max_setpoint_c_);
    return false;
  }
  this->zones_.zones[zone - 1].target_c = celsius;
  this->zone_reset_channel_state_(zone);
  ESP_LOGI(TAG, "Zone %u target set to %.1f C", zone, celsius);
  this->zones_save_();
  this->zone_publish_(zone);
  return true;
}

// --- accessors -------------------------------------------------------------

bool WavinAHC9000::zone_is_enabled(uint8_t zone) const {
  if (!zone_index_ok(zone)) return false;
  return this->zones_.zones[zone - 1].enabled != 0;
}
float WavinAHC9000::zone_get_target(uint8_t zone) const {
  if (!zone_index_ok(zone)) return NAN;
  return this->zones_.zones[zone - 1].target_c;
}
// The room's current temperature: the external sensor when we have one,
// otherwise the Wavin thermostat's own reading so the card is never blank.
float WavinAHC9000::zone_get_temperature(uint8_t zone) const {
  if (!zone_index_ok(zone)) return NAN;
  const auto &rt = this->zone_rt_[zone - 1];
  if (rt.ever_seen && !rt.failsafe && !std::isnan(rt.external_temp_c)) return rt.external_temp_c;

  const auto &z = this->zones_.zones[zone - 1];
  for (uint8_t ch = 1; ch <= 16; ch++) {
    if (!(z.channel_mask & (uint16_t) (1u << (ch - 1)))) continue;
    float air = this->get_channel_current_temp(ch);
    if (!std::isnan(air)) return air;
  }
  return NAN;
}
bool WavinAHC9000::zone_in_failsafe(uint8_t zone) const {
  if (!zone_index_ok(zone)) return false;
  return this->zone_rt_[zone - 1].failsafe;
}
std::string WavinAHC9000::zone_get_name(uint8_t zone) const {
  if (!zone_index_ok(zone)) return "";
  return std::string(this->zones_.zones[zone - 1].name);
}
std::string WavinAHC9000::zone_get_channels_csv(uint8_t zone) const {
  if (!zone_index_ok(zone)) return "";
  return this->mask_to_csv_(this->zones_.zones[zone - 1].channel_mask);
}

// Report what the controller is actually doing, read back from the channel's
// own OUTP ON bit, rather than guessing from our own arithmetic.
climate::ClimateAction WavinAHC9000::zone_get_action(uint8_t zone) const {
  if (!zone_index_ok(zone)) return climate::CLIMATE_ACTION_OFF;
  const auto &z = this->zones_.zones[zone - 1];
  for (uint8_t ch = 1; ch <= 16; ch++) {
    if (!(z.channel_mask & (uint16_t) (1u << (ch - 1)))) continue;
    if (this->get_channel_action(ch) == climate::CLIMATE_ACTION_HEATING) return climate::CLIMATE_ACTION_HEATING;
  }
  return climate::CLIMATE_ACTION_IDLE;
}

// --- entity binding --------------------------------------------------------

void WavinAHC9000::add_zone_climate(uint8_t zone, WavinZoneThermostat *c) { this->zone_climates_[zone] = c; }
void WavinAHC9000::add_zone_enable_switch(uint8_t zone, switch_::Switch *s) { this->zone_enable_switches_[zone] = s; }

void WavinAHC9000::zone_publish_(uint8_t zone) {
  if (!zone_index_ok(zone)) return;
  const auto &z = this->zones_.zones[zone - 1];

  auto cit = this->zone_climates_.find(zone);
  if (cit != this->zone_climates_.end() && cit->second != nullptr) cit->second->update_from_parent();

  auto sit = this->zone_enable_switches_.find(zone);
  if (sit != this->zone_enable_switches_.end() && sit->second != nullptr) {
    if (sit->second->state != (z.enabled != 0)) sit->second->publish_state(z.enabled != 0);
  }
}

// --- control ---------------------------------------------------------------

// Drop zones that no climate entity in the current config is bound to. These
// are leftovers from an earlier config whose room list changed (slots are
// assigned in declaration order), and they would otherwise keep whole rooms'
// channels reserved forever, silently blocking those rooms from being switched
// back on. Runs once, after every zone entity has registered itself.
void WavinAHC9000::zone_prune_unbound_() {
  if (this->zones_pruned_ || this->zone_climates_.empty()) return;
  this->zones_pruned_ = true;
  bool changed = false;
  for (uint8_t zone = 1; zone <= WAVIN_MAX_ZONES; zone++) {
    auto &z = this->zones_.zones[zone - 1];
    if (z.channel_mask == 0) continue;
    if (this->zone_climates_.find(zone) != this->zone_climates_.end()) continue;
    ESP_LOGW(TAG, "Pruning stale zone %u ('%s', channels %s) - not in the current config", zone, z.name,
             this->mask_to_csv_(z.channel_mask).c_str());
    this->zone_release_(zone);
    memset(&z, 0, sizeof(z));
    z.target_c = 21.0f;
    this->zone_rt_[zone - 1] = ZoneRuntime{};
    changed = true;
  }
  if (changed) this->zones_save_();
}

void WavinAHC9000::zones_tick_() {
  this->zone_prune_unbound_();
  for (uint8_t zone = 1; zone <= WAVIN_MAX_ZONES; zone++) {
    if (this->zones_.zones[zone - 1].enabled == 0) continue;
    this->zone_apply_(zone);
    this->zone_publish_(zone);
  }
}

void WavinAHC9000::zone_apply_(uint8_t zone) {
  const auto &z = this->zones_.zones[zone - 1];
  auto &rt = this->zone_rt_[zone - 1];
  uint32_t now = millis();

  // Sensor gone quiet -> fall back to behaving like an ordinary thermostat.
  if (rt.ever_seen && !rt.failsafe && (uint32_t) (now - rt.last_seen_ms) > this->sensor_timeout_ms_) {
    ESP_LOGW(TAG, "Zone %u ('%s'): no temperature for %u s - falling back to the Wavin thermostat", zone, z.name,
             (unsigned) ((now - rt.last_seen_ms) / 1000));
    rt.failsafe = true;
    this->zone_release_(zone);
  }

  // With a usable external reading we bias the setpoint so the controller
  // chases that sensor. Without one - no sensor configured, a mistyped entity
  // name, or a sensor that has gone away - the room simply behaves like the
  // ordinary Wavin thermostat it replaced, and the target is written straight
  // through. That way a typo degrades to "normal", never to "dead".
  bool have_external = rt.ever_seen && !rt.failsafe && !std::isnan(rt.external_temp_c);
  float external = rt.external_temp_c;

  for (uint8_t ch = 1; ch <= 16; ch++) {
    if (!(z.channel_mask & (uint16_t) (1u << (ch - 1)))) continue;

    float desired = z.target_c;
    float air = NAN;
    bool biased = false;
    if (have_external) {
      air = this->get_channel_current_temp(ch);
      if (std::isnan(air)) {
        // The bias needs the wall thermostat's own reading, which may not have
        // arrived yet - a cold boot, or a channel still learning its element.
        // Writing the plain target meanwhile keeps the room heating to roughly
        // the right place; previously this path wrote nothing at all, so a room
        // could sit indefinitely with its external sensor controlling nothing.
        ESP_LOGD(TAG, "Zone %u ch%u: thermostat reading not known yet - writing the target unbiased", zone, ch);
      } else {
        desired = offset_setpoint(air, z.target_c, external, this->min_setpoint_c_, this->max_setpoint_c_);
        biased = true;
      }
    }

    // MANUAL TEMPERATURE is non-volatile inside the AHC-9000, so this guard is
    // protecting the controller's own EEPROM - a separate concern from the
    // ESP32's NVS above. Tracked per channel so every channel of a joined room
    // gets its own write.
    bool first = !this->ch_applied_valid_[ch] || std::isnan(this->ch_applied_setpoint_[ch]);
    if (!should_write_setpoint(first, desired, this->ch_applied_setpoint_[ch], this->setpoint_deadband_c_, now,
                               this->ch_last_write_ms_[ch], this->min_write_interval_ms_)) {
      ESP_LOGV(TAG, "Zone %u ch%u: write suppressed (want %.2f, applied %.2f)", zone, ch, desired,
               this->ch_applied_setpoint_[ch]);
      continue;
    }

    if (biased) {
      ESP_LOGI(TAG, "Zone %u ch%u: setpoint %.1f C (air=%.2f target=%.1f external=%.2f)", zone, ch, desired, air,
               z.target_c, external);
    } else if (have_external) {
      ESP_LOGI(TAG, "Zone %u ch%u: setpoint %.1f C (target written unbiased - no thermostat reading yet)", zone, ch,
               desired);
    } else {
      ESP_LOGI(TAG, "Zone %u ch%u: setpoint %.1f C (no external sensor - plain thermostat)", zone, ch, desired);
    }
    this->write_channel_setpoint(ch, desired);
    this->ch_applied_setpoint_[ch] = desired;
    this->ch_applied_valid_[ch] = true;
    this->ch_last_write_ms_[ch] = now;
  }
}

void WavinAHC9000::zone_reset_channel_state_(uint8_t zone) {
  if (!zone_index_ok(zone)) return;
  const auto &z = this->zones_.zones[zone - 1];
  for (uint8_t ch = 1; ch <= 16; ch++) {
    if (!(z.channel_mask & (uint16_t) (1u << (ch - 1)))) continue;
    this->ch_applied_valid_[ch] = false;
    this->ch_applied_setpoint_[ch] = NAN;
  }
}

// Write the plain target back, so the Wavin thermostat is in charge again.
void WavinAHC9000::zone_release_(uint8_t zone) {
  if (!zone_index_ok(zone)) return;
  const auto &z = this->zones_.zones[zone - 1];
  if (z.channel_mask == 0) return;

  for (uint8_t ch = 1; ch <= 16; ch++) {
    if (!(z.channel_mask & (uint16_t) (1u << (ch - 1)))) continue;
    if (!this->ch_applied_valid_[ch]) continue;  // never biased, nothing to undo
    ESP_LOGI(TAG, "Zone %u ch%u: restoring plain setpoint %.1f C (thermostat back in control)", zone, ch,
             z.target_c);
    this->write_channel_setpoint(ch, z.target_c);
    this->ch_applied_valid_[ch] = false;
    this->ch_applied_setpoint_[ch] = NAN;
  }
}

void WavinAHC9000::release_all_zones() {
  for (uint8_t zone = 1; zone <= WAVIN_MAX_ZONES; zone++) this->zone_release_(zone);
}

void WavinAHC9000::on_shutdown() {
  // Don't leave a biased setpoint behind across an OTA or a clean reboot.
  this->release_all_zones();
}

// --- discovery -------------------------------------------------------------

// Reports which channels actually have a learned thermostat, so the
// commissioning UI offers real channels instead of guesses.
void WavinAHC9000::discover_channels() {
  std::string json = "{\"channels\":[";
  bool first = true;

  for (uint8_t ch = 1; ch <= 16; ch++) {
    std::vector<uint16_t> regs;
    if (!this->read_registers(CAT_CHANNELS, (uint8_t) (ch - 1), CH_PRIMARY_ELEMENT, 1, regs) || regs.empty())
      continue;
    uint16_t prim = regs[0] & CH_PRIMARY_ELEMENT_ELEMENT_MASK;
    if (prim == 0) continue;  // channel not in use

    float temp = this->get_channel_current_temp(ch);
    uint8_t owner = 0;
    for (uint8_t z = 1; z <= WAVIN_MAX_ZONES; z++) {
      const auto &zz = this->zones_.zones[z - 1];
      if (zz.channel_mask & (uint16_t) (1u << (ch - 1))) {
        owner = z;
        break;
      }
    }

    char buf[128];
    if (std::isnan(temp)) {
      // Not polled yet - report null rather than a misleading 0.0
      snprintf(buf, sizeof(buf), "%s{\"ch\":%u,\"temp\":null,\"zone\":%u}", first ? "" : ",", ch, owner);
    } else {
      snprintf(buf, sizeof(buf), "%s{\"ch\":%u,\"temp\":%.1f,\"zone\":%u}", first ? "" : ",", ch, temp, owner);
    }
    json += buf;
    first = false;
    ESP_LOGI(TAG, "discovery: ch%u in use, temp=%.1f, zone=%u", ch, temp, owner);
  }
  json += "]}";

  // Append the zone table so one sensor shows the whole picture.
  ESP_LOGI(TAG, "--- zones ---");
  for (uint8_t z = 1; z <= WAVIN_MAX_ZONES; z++) {
    const auto &cfg = this->zones_.zones[z - 1];
    const auto &rt = this->zone_rt_[z - 1];
    if (cfg.channel_mask == 0) continue;
    ESP_LOGI(TAG, "  zone %u: '%s' ch=[%s] external=%s target=%.1f temp=%.2f%s", z, cfg.name,
             this->mask_to_csv_(cfg.channel_mask).c_str(), cfg.enabled ? "ON" : "off", cfg.target_c,
             rt.external_temp_c, rt.failsafe ? " (sensor stale - thermostat in control)" : "");
  }

  if (this->discovery_text_sensor_ != nullptr) this->discovery_text_sensor_->publish_state(json);
}

// --- zone entities ---------------------------------------------------------

void WavinZoneThermostat::setup() {
  if (this->parent_ == nullptr) return;

  // Channels declared in the YAML.
  if (this->static_mask_ != 0) {
    this->parent_->zone_set_static(this->zone_, this->static_mask_, this->get_name().str());
  }

  // Subscribe straight to the Home Assistant entity over the native API, so
  // values arrive here by themselves, including "unavailable", with nothing
  // to configure on the HA side.
// USE_API_HOMEASSISTANT_STATES is only defined when at least one room actually
// declares a `sensor:`, so guard on it too or the API method isn't compiled in.
#if defined(USE_API) && defined(USE_API_HOMEASSISTANT_STATES)
  if (!this->source_entity_.empty() && api::global_api_server != nullptr) {
    ESP_LOGCONFIG(TAG, "Room '%s' follows %s", this->get_name().c_str(), this->source_entity_.c_str());
    api::global_api_server->subscribe_home_assistant_state(
        this->source_entity_, optional<std::string>(), [this](StringRef state) {
          if (this->parent_ == nullptr) return;
          auto val = parse_number<float>(state.c_str());
          if (!val.has_value()) {
            // "unknown" / "unavailable" / anything unparseable -> hand the room
            // back to its own Wavin thermostat.
            ESP_LOGW(TAG, "Room '%s': sensor reported '%s'", this->get_name().c_str(), state.c_str());
            this->parent_->zone_push_unavailable(this->zone_);
            return;
          }
          this->parent_->zone_push_temperature(this->zone_, *val);
        });
  }
#endif

  this->update_from_parent();
}

// Wait until the API is up, otherwise the subscription is lost.
float WavinZoneThermostat::get_setup_priority() const { return setup_priority::AFTER_CONNECTION; }

climate::ClimateTraits WavinZoneThermostat::traits() {
  climate::ClimateTraits t;
  t.set_supported_modes({climate::CLIMATE_MODE_HEAT, climate::CLIMATE_MODE_OFF});
  t.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE | climate::CLIMATE_SUPPORTS_ACTION);
  t.set_visual_min_temperature(this->visual_min_);
  t.set_visual_max_temperature(this->visual_max_);
  t.set_visual_target_temperature_step(0.5f);
  t.set_visual_current_temperature_step(0.1f);
  return t;
}

void WavinZoneThermostat::control(const climate::ClimateCall &call) {
  if (this->parent_ == nullptr) return;
  if (call.get_mode().has_value()) {
    // OFF here means "stop using the external sensor", i.e. hand this room
    // back to its original Wavin thermostat. It does not mean "no heating".
    this->parent_->zone_set_enabled(this->zone_, *call.get_mode() != climate::CLIMATE_MODE_OFF);
  }
  if (call.get_target_temperature().has_value()) {
    this->parent_->zone_set_target(this->zone_, *call.get_target_temperature());
  }
  this->update_from_parent();
}

void WavinZoneThermostat::update_from_parent() {
  if (this->parent_ == nullptr) return;
  bool enabled = this->parent_->zone_is_enabled(this->zone_);
  this->mode = enabled ? climate::CLIMATE_MODE_HEAT : climate::CLIMATE_MODE_OFF;
  this->target_temperature = this->parent_->zone_get_target(this->zone_);
  this->current_temperature = this->parent_->zone_get_temperature(this->zone_);
  this->action = enabled ? this->parent_->zone_get_action(this->zone_) : climate::CLIMATE_ACTION_OFF;
  this->publish_state();
}

void WavinZoneThermostat::dump_config() { ESP_LOGCONFIG(TAG, "Wavin zone climate: slot %u", this->zone_); }

void WavinZoneEnableSwitch::write_state(bool state) {
  if (this->parent_ != nullptr && !this->parent_->zone_set_enabled(this->zone_, state)) {
    // Rejected (e.g. not commissioned yet) - snap the UI back to reality.
    this->publish_state(this->parent_->zone_is_enabled(this->zone_));
    return;
  }
  this->publish_state(state);
}

}  // namespace wavinahc9000v3
}  // namespace esphome

