#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/modbus/modbus.h"

namespace esphome {
namespace genvex {

class Genvex : public PollingComponent, public modbus::ModbusDevice {
 public:
  void set_temp_t1_sensor(sensor::Sensor * temp_t1_sensor) { temp_t1_sensor_ = temp_t1_sensor; }
  void set_temp_t2_sensor(sensor::Sensor * temp_t2_sensor) { temp_t2_sensor_ = temp_t2_sensor; }
  void set_temp_t3_sensor(sensor::Sensor * temp_t3_sensor) { temp_t3_sensor_ = temp_t3_sensor; }
  void set_temp_t4_sensor(sensor::Sensor * temp_t4_sensor) { temp_t4_sensor_ = temp_t4_sensor; }
  void set_temp_t5_sensor(sensor::Sensor * temp_t5_sensor) { temp_t5_sensor_ = temp_t5_sensor; }
  void set_temp_t6_sensor(sensor::Sensor * temp_t6_sensor) { temp_t6_sensor_ = temp_t6_sensor; }
  void set_temp_t7_sensor(sensor::Sensor * temp_t7_sensor) { temp_t7_sensor_ = temp_t7_sensor; }
  void set_temp_t8_sensor(sensor::Sensor * temp_t8_sensor) { temp_t8_sensor_ = temp_t8_sensor; }
  void set_temp_t9_sensor(sensor::Sensor * temp_t9_sensor) { temp_t9_sensor_ = temp_t9_sensor; }
  void set_temp_t2_panel_sensor(sensor::Sensor * temp_t2_panel_sensor) { temp_t2_panel_sensor_ = temp_t2_panel_sensor; }
  void set_measured_humidity_sensor(sensor::Sensor *measured_humidity_sensor) { measured_humidity_sensor_ = measured_humidity_sensor; }
  void set_humidity_calculated_setpoint_sensor(sensor::Sensor *humidity_calculated_setpoint_sensor) { humidity_calculated_setpoint_sensor_ = humidity_calculated_setpoint_sensor; }
  void set_alarm_bit_sensor(sensor::Sensor *alarm_bit_sensor) { alarm_bit_sensor_ = alarm_bit_sensor; }
  void set_inlet_fan_sensor(sensor::Sensor *inlet_fan_sensor) { inlet_fan_sensor_ = inlet_fan_sensor; }
  void set_extract_fan_sensor(sensor::Sensor *extract_fan_sensor) { extract_fan_sensor_ = extract_fan_sensor; }
  void set_bypass_sensor(sensor::Sensor *bypass_sensor) { bypass_sensor_ = bypass_sensor; }
  void set_watervalve_sensor(sensor::Sensor *watervalve_sensor) { watervalve_sensor_ = watervalve_sensor; }
  void set_humidity_fan_control_sensor(sensor::Sensor *humidity_fan_control_sensor) { humidity_fan_control_sensor_ = humidity_fan_control_sensor; }
  void set_bypass_on_off_sensor(sensor::Sensor *bypass_on_off_sensor) { bypass_on_off_sensor_ = bypass_on_off_sensor; }
  void set_target_temp_sensor(sensor::Sensor *target_temp_sensor) { target_temp_sensor_ = target_temp_sensor; }
  void set_speed_mode_sensor(sensor::Sensor *speed_mode_sensor) { speed_mode_sensor_ = speed_mode_sensor; }
  void set_heat_sensor(sensor::Sensor *heat_sensor) { heat_sensor_ = heat_sensor; }
  void set_timer_sensor(sensor::Sensor *timer_sensor) { timer_sensor_ = timer_sensor; }

  void add_target_temp_callback(std::function<void(float)> &&callback);
  void add_fan_speed_callback(std::function<void(int)> &&callback);
  
  void loop() override;
  void update() override;

  void on_modbus_data(const std::vector<uint8_t> &data) override;
  
  void writeTargetTemperature(float new_target_temp);
  void writeFanMode(int new_fan_speed);
  
  void dump_config() override;

 protected:
  int state_{0};
  uint8_t CMD_FUNCTION_REG{0x04};
  bool waiting_{false};
  long last_send_{0};
  bool waiting_for_write_ack_{false};
  
  sensor::Sensor *temp_t1_sensor_; 
  sensor::Sensor *temp_t2_sensor_; 
  sensor::Sensor *temp_t3_sensor_; 
  sensor::Sensor *temp_t4_sensor_; 
  sensor::Sensor *temp_t5_sensor_; 
  sensor::Sensor *temp_t6_sensor_; 
  sensor::Sensor *temp_t7_sensor_; 
  sensor::Sensor *temp_t8_sensor_; 
  sensor::Sensor *temp_t9_sensor_; 
  sensor::Sensor *temp_t2_panel_sensor_;
  sensor::Sensor *measured_humidity_sensor_;  
  sensor::Sensor *humidity_calculated_setpoint_sensor_;
  sensor::Sensor *alarm_bit_sensor_;
  sensor::Sensor *inlet_fan_sensor_;
  sensor::Sensor *extract_fan_sensor_;
  sensor::Sensor *bypass_sensor_;
  sensor::Sensor *watervalve_sensor_;
  sensor::Sensor *humidity_fan_control_sensor_;
  sensor::Sensor *bypass_on_off_sensor_;
  sensor::Sensor *target_temp_sensor_;
  sensor::Sensor *speed_mode_sensor_;
  sensor::Sensor *heat_sensor_;
  sensor::Sensor *timer_sensor_;

  CallbackManager<void(float)> target_temp_callback_;
  CallbackManager<void(int)> fan_speed_callback_;
};

}  // namespace genvex
}  // namespace esphome
