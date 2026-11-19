#pragma once

#include <cmath>
#include <string>
#include <vector>

#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/gpio.h"

#include "modbus_rtu.h"
#include "point_catalog.h"

namespace esphome {
namespace auto_nilan {

class AutoNilan : public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_address(uint8_t address) { this->address_ = address; }
  void set_flow_control_pin(GPIOPin *pin) { this->flow_control_pin_ = pin; }
  void set_receive_timeout_ms(uint32_t ms) { this->timing_.rx_timeout_ms = ms; }
  void set_inter_frame_delay_us(uint32_t us) { this->timing_.inter_frame_us = us; }

  void set_topology_text_sensor(text_sensor::TextSensor *sensor) { this->topology_sensor_ = sensor; }
  void set_values_text_sensor(text_sensor::TextSensor *sensor) { this->values_sensor_ = sensor; }
  void set_status_text_sensor(text_sensor::TextSensor *sensor) { this->status_sensor_ = sensor; }

  // Re-runs controller detection and capability probing.
  void commission();
  // Writes a single point by catalog key. Returns false and fills *error on failure.
  bool write_point(const std::string &key, float value, std::string *error);
  // Presses a command point (button kind) using its maximum value.
  bool press_point(const std::string &key, std::string *error);
  void reset_alarm();

  const std::string &topology_json() const { return this->topology_json_; }
  const std::string &values_json() const { return this->values_json_; }
  const std::string &status_json() const { return this->status_json_; }

  Profile profile() const { return this->profile_; }
  bool commissioned() const { return this->commissioned_; }

 protected:
  struct RuntimePoint {
    const PointDef *def{nullptr};
    float value{NAN};
    bool present{false};
    bool fresh{false};
  };

  bool detect_profile_();
  void probe_capabilities_();
  void build_point_list_();
  void probe_optional_points_();
  bool poll_once_();
  void publish_payloads_();

  bool read_raw_(uint8_t reg_type, uint16_t address, uint16_t &value);
  bool read_block_(uint8_t reg_type, uint16_t address, uint16_t count, std::vector<uint16_t> &out);
  bool write_raw_(uint16_t address, uint16_t value);

  RuntimePoint *find_point_(const std::string &key);
  static float decode_(const PointDef &def, uint16_t raw);
  static uint16_t encode_(const PointDef &def, float human);
  static bool plausible_(const PointDef &def, float human);
  static void build_topology_(const std::vector<RuntimePoint> &points, Profile profile, uint16_t model,
                              uint16_t caps, std::string &out);
  void build_values_(std::string &out);
  void build_status_(std::string &out);
  void set_error_(const std::string &error);

  uint8_t address_{30};
  GPIOPin *flow_control_pin_{nullptr};
  ModbusTiming timing_;
  ModbusStats stats_;

  Profile profile_{PROFILE_UNKNOWN};
  uint16_t model_{0};
  uint16_t caps_{0};
  bool commissioned_{false};
  uint32_t commission_attempts_{0};
  uint32_t last_commission_attempt_ms_{0};

  std::vector<RuntimePoint> points_;

  uint32_t poll_duration_ms_{0};
  uint32_t last_poll_ms_{0};
  uint32_t last_success_ms_{0};
  uint32_t poll_count_{0};
  bool online_{false};
  bool topology_dirty_{true};
  std::string last_error_;

  std::string topology_json_;
  std::string values_json_;
  std::string status_json_;

  text_sensor::TextSensor *topology_sensor_{nullptr};
  text_sensor::TextSensor *values_sensor_{nullptr};
  text_sensor::TextSensor *status_sensor_{nullptr};
};

}  // namespace auto_nilan
}  // namespace esphome
