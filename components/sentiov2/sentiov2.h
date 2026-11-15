#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/text_sensor/text_sensor.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace esphome {
namespace sentiov2 {

static constexpr uint8_t SENTIO_MAX_ROOMS = 16;

struct SentioRoom {
  bool present{false};
  bool dummy{false};
  bool has_control{false};
  bool has_sensor{false};
  bool name_valid{false};
  std::string name;
  float current{NAN};
  float target{NAN};
  float floor{NAN};
  float humidity{NAN};
  float dewpoint{NAN};
  uint8_t state{0};
  uint8_t radiator_state{0};
  uint8_t ufh_state{0};
  uint8_t drying_state{0};
  uint8_t thermal_state{0};
  uint8_t room_mode{0};
  uint8_t override_mode{0};
  uint8_t lock_value{32};
  bool battery_low{false};
  bool warning{false};
  bool error{false};
  bool off{false};
  uint32_t last_good_ms{0};
};

class SentioV2 : public PollingComponent, public uart::UARTDevice {
 public:
  void set_flow_control_pin(GPIOPin *pin) { this->flow_control_pin_ = pin; }
  void set_address(uint8_t address) { this->address_ = address; }
  void set_receive_timeout_ms(uint32_t timeout) { this->receive_timeout_ms_ = timeout; }
  void set_inter_frame_delay_us(uint32_t delay) { this->inter_frame_delay_us_ = delay; }
  void set_topology_text_sensor(text_sensor::TextSensor *sensor) {
    this->topology_text_sensor_ = sensor;
  }

  void setup() override;
  void update() override;
  void dump_config() override;

  void publish_topology();

  bool zone_set_target(uint8_t zone, float temperature);
  bool zone_set_hvac_mode(uint8_t zone, bool turn_off);
  bool zone_set_child_lock(uint8_t zone, bool enabled);
  bool zone_set_vacation(bool enabled);
  bool zone_set_sync(bool enabled);
  bool sync_enabled() const { return this->sync_all_rooms_; }

 protected:
  bool read_input_registers_(uint16_t start, uint16_t count, std::vector<uint16_t> &out);
  bool read_input_individually_(uint16_t start, uint8_t count, std::vector<uint16_t> &out);
  bool read_holding_registers_(uint16_t start, uint16_t count, std::vector<uint16_t> &out);
  bool read_discrete_inputs_(uint16_t start, uint16_t count, std::vector<bool> &out);
  bool write_single_register_(uint16_t address, uint16_t value);

  bool read_registers_(uint8_t function_code, uint16_t start, uint16_t count,
                       std::vector<uint8_t> &payload);
  bool write_register_(uint16_t address, uint16_t value);
  bool receive_frame_(uint8_t function_code, uint16_t expected_length,
                      std::vector<uint8_t> &frame);

  void read_room_(uint8_t room);
  void read_vacation_();
  uint8_t first_control_room_() const;
  void apply_sync_();
  bool parse_float_(uint16_t raw, float minimum, float maximum, float &out) const;
  bool parse_temperature_(uint16_t raw, float &out) const;
  bool parse_humidity_(uint16_t raw, float &out) const;
  std::string parse_name_(const std::vector<uint16_t> &registers) const;
  std::string action_name_(uint8_t state) const;

  void prepare_tx_();
  void finish_tx_();
  void inter_frame_delay_();
  uint16_t crc16_(const uint8_t *data, size_t length) const;

  void append_json_string_(std::string &out, const std::string &value) const;
  void append_json_number_(std::string &out, float value) const;

  uint8_t address_{1};
  uint32_t receive_timeout_ms_{300};
  uint32_t inter_frame_delay_us_{4000};
  uint32_t pre_tx_delay_us_{400};
  uint32_t post_tx_guard_us_{600};
  uint32_t rx_gap_us_{2500};
  uint32_t tx_done_us_{0};
  GPIOPin *flow_control_pin_{nullptr};
  text_sensor::TextSensor *topology_text_sensor_{nullptr};
  SentioRoom rooms_[SENTIO_MAX_ROOMS];
  bool vacation_{false};
  bool sync_all_rooms_{false};
};

}  // namespace sentiov2
}  // namespace esphome
