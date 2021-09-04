#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/modbus/modbus.h"

namespace esphome {
namespace nilan {
struct WriteableData
{
  uint16_t register_value;
  uint16_t write_value;
};

class Nilan : public PollingComponent, public modbus::ModbusDevice {
public:

  void set_temp_t0_sensor(sensor::Sensor *temp_t0_sensor) {
    temp_t0_sensor_ = temp_t0_sensor;
  }

  void set_temp_t1_sensor(sensor::Sensor* temp_t1_sensor) {
    temp_t1_sensor_ = temp_t1_sensor;
  }

  void set_temp_t2_sensor(sensor::Sensor* temp_t2_sensor) {
    temp_t2_sensor_ = temp_t2_sensor;
  }

  void set_temp_t3_sensor(sensor::Sensor *temp_t3_sensor) {
    temp_t3_sensor_ = temp_t3_sensor;
  }

  void set_temp_t4_sensor(sensor::Sensor *temp_t4_sensor) {
    temp_t4_sensor_ = temp_t4_sensor;
  }

  void set_temp_t5_sensor(sensor::Sensor* temp_t5_sensor) {
    temp_t5_sensor_ = temp_t5_sensor;
  }

  void set_temp_t6_sensor(sensor::Sensor* temp_t6_sensor) {
    temp_t6_sensor_ = temp_t6_sensor;
  }

  void set_temp_t7_sensor(sensor::Sensor *temp_t7_sensor) {
    temp_t7_sensor_ = temp_t7_sensor;
  }

  void set_temp_t8_sensor(sensor::Sensor *temp_t8_sensor) {
    temp_t8_sensor_ = temp_t8_sensor;
  }

  void set_temp_t9_sensor(sensor::Sensor* temp_t9_sensor) {
    temp_t9_sensor_ = temp_t9_sensor;
  }

  void set_temp_t10_sensor(sensor::Sensor* temp_t10_sensor) {
    temp_t10_sensor_ = temp_t10_sensor;
  }

  void set_temp_t11_sensor(sensor::Sensor* temp_t11_sensor) {
    temp_t11_sensor_ = temp_t11_sensor;
  }

  void set_temp_t12_sensor(sensor::Sensor* temp_t12_sensor) {
    temp_t12_sensor_ = temp_t12_sensor;
  }

  void set_temp_t13_sensor(sensor::Sensor* temp_t13_sensor) {
    temp_t13_sensor_ = temp_t13_sensor;
  }

  void set_temp_t14_sensor(sensor::Sensor* temp_t14_sensor) {
    temp_t14_sensor_ = temp_t14_sensor;
  }

  void set_temp_t15_sensor(sensor::Sensor *temp_t15_sensor) {
    temp_t15_sensor_ = temp_t15_sensor;
  }

  void set_temp_t16_sensor(sensor::Sensor* temp_t16_sensor) {
    temp_t16_sensor_ = temp_t16_sensor;
  }

  void set_temp_t17_sensor(sensor::Sensor* temp_t17_sensor) {
    temp_t17_sensor_ = temp_t17_sensor;
  }

  void set_temp_t18_sensor(sensor::Sensor* temp_t18_sensor) {
    temp_t18_sensor_ = temp_t18_sensor;
  }

  void set_measured_humidity_sensor(sensor::Sensor *measured_humidity_sensor) {
    measured_humidity_sensor_ = measured_humidity_sensor;
  }

  void set_active_alarms_sensor(sensor::Sensor *active_alarms_sensor) {
    active_alarms_sensor_ = active_alarms_sensor;
  }

  void set_cool_target_temp_sensor(sensor::Sensor *cool_target_temp_sensor) {
    cool_target_temp_sensor_ = cool_target_temp_sensor;
  }

  void set_target_temp_sensor(sensor::Sensor *target_temp_sensor) {
    target_temp_sensor_ = target_temp_sensor;
  }

  void set_min_winter_temp_sensor(sensor::Sensor *min_winter_temp_sensor) {
    min_winter_temp_sensor_ = min_winter_temp_sensor;
  }

  void set_max_winter_temp_sensor(sensor::Sensor *max_winter_temp_sensor) {
    max_winter_temp_sensor_ = max_winter_temp_sensor;
  }

  void set_min_summer_temp_sensor(sensor::Sensor *min_summer_temp_sensor) {
    min_summer_temp_sensor_ = min_summer_temp_sensor;
  }

  void set_max_summer_temp_sensor(sensor::Sensor *max_summer_temp_sensor) {
    max_summer_temp_sensor_ = max_summer_temp_sensor;
  }

  void set_heat_exchange_efficiency_sensor(
    sensor::Sensor *heat_exchange_efficiency_sensor) {
    heat_exchange_efficiency_sensor_ = heat_exchange_efficiency_sensor;
  }

  void set_co2_sensor(sensor::Sensor *co2_sensor) {
    co2_sensor_ = co2_sensor;
  }

  void set_inlet_fan_sensor(sensor::Sensor *inlet_fan_sensor) {
    inlet_fan_sensor_ = inlet_fan_sensor;
  }

  void set_exhaust_fan_sensor(sensor::Sensor *exhaust_fan_sensor) {
    exhaust_fan_sensor_ = exhaust_fan_sensor;
  }

  void set_ventilation_speed_sensor(sensor::Sensor *ventilation_speed_sensor) {
    ventilation_speed_sensor_ = ventilation_speed_sensor;
  }

  void set_is_summer_sensor(binary_sensor::BinarySensor *is_summer_sensor) {
    is_summer_sensor_ = is_summer_sensor;
  }

  void set_filter_ok_sensor(binary_sensor::BinarySensor *filter_ok_sensor) {
    filter_ok_sensor_ = filter_ok_sensor;
  }

  void set_door_open_sensor(binary_sensor::BinarySensor *door_open_sensor) {
    door_open_sensor_ = door_open_sensor;
  }

  void set_bypass_on_off_sensor(binary_sensor::BinarySensor *bypass_on_off_sensor)
  {
    bypass_on_off_sensor_ = bypass_on_off_sensor;
  }

  void set_on_off_state_sensor(binary_sensor::BinarySensor *on_off_state_sensor) {
    on_off_state_sensor_ = on_off_state_sensor;
  }

  void set_operation_mode_sensor(text_sensor::TextSensor *operation_mode_sensor) {
    operation_mode_sensor_ = operation_mode_sensor;
  }

  void set_control_state_sensor(text_sensor::TextSensor *control_state_sensor) {
    control_state_sensor_ = control_state_sensor;
  }

  void set_version_info_sensor(text_sensor::TextSensor *version_info_sensor) {
    version_info_sensor_ = version_info_sensor;
  }

  void set_display_line1_sensor(text_sensor::TextSensor *display_line1_sensor) {
    display_line1_sensor_ = display_line1_sensor;
  }

  void set_display_line2_sensor(text_sensor::TextSensor *display_line2_sensor) {
    display_line2_sensor_ = display_line2_sensor;
  }

  // void add_target_temp_callback(std::function<void(float)> &&callback);
  // void add_fan_speed_callback(std::function<void(int)> &&callback);
  // void add_operation_mode_callback(std::function<void(int)>&& callback);

  void loop() override;
  void update() override;

  void on_modbus_data(const std::vector<uint8_t>& data) override;

  void handleData(const std::vector<uint8_t>& data);
  void handleDeviceInputData(const std::vector<uint8_t>& data);
  void handleDiscreteIOInputData(const std::vector<uint8_t>& data);
  void handleAnalogIOInputData(const std::vector<uint8_t>& data);
  void handleAlarmInputData(const std::vector<uint8_t>& data);
  void handleControlInputData(const std::vector<uint8_t>& data);
  void handleAirflowInputData(const std::vector<uint8_t>& data);
  void handleAirtempInputData(const std::vector<uint8_t>& data);
  void handleCentralHeatInputData(const std::vector<uint8_t>& data);
  void handleUserPanelInputData(const std::vector<uint8_t>& data);
  
  void handleAirtempHoldingData(const std::vector<uint8_t>& data);
  void handleControlStateHoldingData(const std::vector<uint8_t>& data);
  void handleFlapsData(const std::vector<uint8_t>& data);
  void handleFanData(const std::vector<uint8_t>& data);

  void publishState(sensor::Sensor *sensor, float value) {
    if (sensor &&
        ((sensor->state != value) ||
         ignore_previous_state_)) sensor->publish_state(value);
  }

  void publishState(binary_sensor::BinarySensor *sensor, bool value) {
    if (sensor &&
        ((sensor->state != value) ||
         ignore_previous_state_)) sensor->publish_state(value);
  }

  void publishState(text_sensor::TextSensor *sensor, const std::string& value) {
    if (sensor &&
        ((sensor->state != value) ||
         ignore_previous_state_)) sensor->publish_state(value);
  }

  void writeTargetTemperature(float new_target_temp);
  void writeFanMode(int new_fan_speed);
  void writeOperationMode(int new_mode);
  void writeRunset(int new_mode);

  // User
  void writeUserFuncAct(int new_userfuncact);
  void writeUserFuncSet(int new_userfuncset);
  void writeUserTimeSet(int new_usertimeset);
  void writeUserVentSet(int new_userventset);
  void writeUserTempSet(int new_usertempset);
  void writeUserOffsSet(int new_useroffsset);
  // User2
  void writeUser2FuncAct(int new_user2funcact);
  void writeUser2FuncSet(int new_user2funcset);
  void writeUser2TimeSet(int new_user2timeset);
  void writeUser2VentSet(int new_user2ventset);
  void writeUser2TempSet(int new_user2tempset);
  void writeUser2OffsSet(int new_user2offsset);

  void dump_config() override;

protected:

  enum ReadRegister {
    device_input,
    discrete_io_input,
    analog_io_input,
    alarm_input,
    control_input,
    airflow_input,
    airtemp_input,
    central_heat_input,
    user_panel_input,
    time_holding,
    airtemp_holding,
    control_state_holding,
    flaps_data,
    fan_data,
    user_functions,
    user_functions2
  };

  enum ReadWriteMode {
    read,
    write,
    idle
  };

  const std::vector<ReadRegister>enabled_read_registers_ = {
    device_input,
    discrete_io_input,
    analog_io_input,
    alarm_input,
    control_input,
    // airflow_input,
    airtemp_input,
    // central_heat_input,
    // user_panel_input,
    // time_holding,
    airtemp_holding,
    control_state_holding,
    flaps_data,
    fan_data,
    user_functions,
    user_functions2
  };


  ReadWriteMode current_read_write_mode_                = { Nilan::read };
  std::vector<ReadRegister>::const_iterator read_state_ =
  { enabled_read_registers_.begin() };
  std::deque<WriteableData>writequeue_;
  bool waiting_{ false };
  long last_send_{ 0 };

  sensor::Sensor *temp_t0_sensor_;
  sensor::Sensor *temp_t1_sensor_;
  sensor::Sensor *temp_t2_sensor_;
  sensor::Sensor *temp_t3_sensor_;
  sensor::Sensor *temp_t4_sensor_;
  sensor::Sensor *temp_t5_sensor_;
  sensor::Sensor *temp_t6_sensor_;
  sensor::Sensor *temp_t7_sensor_;
  sensor::Sensor *temp_t8_sensor_;
  sensor::Sensor *temp_t9_sensor_;
  sensor::Sensor *temp_t10_sensor_;
  sensor::Sensor *temp_t11_sensor_;
  sensor::Sensor *temp_t12_sensor_;
  sensor::Sensor *temp_t13_sensor_;
  sensor::Sensor *temp_t14_sensor_;
  sensor::Sensor *temp_t15_sensor_;
  sensor::Sensor *temp_t16_sensor_;
  sensor::Sensor *temp_t17_sensor_;
  sensor::Sensor *temp_t18_sensor_;
  sensor::Sensor *measured_humidity_sensor_;
  sensor::Sensor *active_alarms_sensor_;
  sensor::Sensor *cool_target_temp_sensor_;
  sensor::Sensor *min_summer_temp_sensor_;
  sensor::Sensor *max_summer_temp_sensor_;
  sensor::Sensor *min_winter_temp_sensor_;
  sensor::Sensor *max_winter_temp_sensor_;
  sensor::Sensor *heat_exchange_efficiency_sensor_;
  sensor::Sensor *co2_sensor_;
  sensor::Sensor *exhaust_fan_sensor_;
  sensor::Sensor *inlet_fan_sensor_;
  sensor::Sensor *target_temp_sensor_;
  sensor::Sensor *ventilation_speed_sensor_;
  binary_sensor::BinarySensor *on_off_state_sensor_;
  binary_sensor::BinarySensor *is_summer_sensor_;
  binary_sensor::BinarySensor *filter_ok_sensor_;
  binary_sensor::BinarySensor *door_open_sensor_;
  binary_sensor::BinarySensor *bypass_on_off_sensor_;
  text_sensor::TextSensor *version_info_sensor_;
  text_sensor::TextSensor *control_state_sensor_;
  text_sensor::TextSensor *operation_mode_sensor_;
  text_sensor::TextSensor *display_line1_sensor_;
  text_sensor::TextSensor *display_line2_sensor_;

  CallbackManager<void(float)>target_temp_callback_;
  CallbackManager<void(int)>fan_speed_callback_;
  CallbackManager<void(int)>operation_mode_callback_;

private:

  void     writeModbusRegister(WriteableData write_data);
  uint16_t get_16bit(const std::vector<uint8_t>& data, size_t i) {
    return (uint16_t(data[i]) << 8) | uint16_t(data[i + 1]);
  }

  float scaleAndConvertToFloat(uint16_t rawValue) {
    return static_cast<int16_t>(rawValue) / 100.0;
  }

  void nextReadState(bool rollover);
  void loopRead();
  void idleToWriteMode();

  bool ignore_previous_state_ = true;
};
} // namespace nilan
} // namespace esphome
