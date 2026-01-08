#pragma once

#include <CircularBuffer.hpp>
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/log.h"
#define STROFF "OFF"
#define STRON "ON"
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
namespace esphome
{
  namespace balboa_spa
  {
    class BalboaSpa : public Component, public uart::UARTDevice
    {
    public:
      void setup() override;
      void loop() override;
      void set_temp_sensor(sensor::Sensor *temp_sensor);
      void set_fault_text_sensor(text_sensor::TextSensor *fault_text_sensor);
      void set_circ_binary_sensor(binary_sensor::BinarySensor *circ_binary_sensor);
      void set_heater_binary_sensor(binary_sensor::BinarySensor *heater_binary_sensor);
      void set_blower_binary_sensor(binary_sensor::BinarySensor *blower_binary_sensor);
      void set_rest_binary_sensor(binary_sensor::BinarySensor *rest_binary_sensor);
      void set_high_range_binary_sensor(binary_sensor::BinarySensor *high_range_binary_sensor);
  void set_hour_sensor(sensor::Sensor *hour_sensor) { this->hour_sensor_ = hour_sensor; }
  void set_minute_sensor(sensor::Sensor *minute_sensor) { this->minute_sensor_ = minute_sensor; }
  void set_time_text_sensor(text_sensor::TextSensor *time_text_sensor) { this->time_text_sensor_ = time_text_sensor; }
  void set_heating_status_text_sensor(text_sensor::TextSensor *text) { this->heating_status_text_sensor_ = text; }
      void on_toggle_jet1();
      void on_toggle_jet2();
      void on_toggle_light();
  void on_set_time(int hour, int minute);
  void set_heater_enabled(bool enabled);
      void register_binary_sensor_callback(uint8_t datapoint, const std::function<void(bool)> &func);
      void register_sensor_callback(uint8_t datapoint, const std::function<void(float)> &func);
      void on_set_temp(float temp);

      float target_temp{NAN};
      float temperature{NAN};

    protected:
      std::function<void(bool)>  light_state_update_{nullptr};
      std::function<void(bool)>  heater_state_update_{nullptr};
      std::function<void(bool)>  circulation_pump_state_update_{nullptr};
      std::function<void(bool)>  rest_state_update_{nullptr};
  std::function<void(bool)>  heating_enabled_state_update_{nullptr};
      std::function<void(bool)>  jet1_state_update_{nullptr};
      std::function<void(bool)>  jet2_state_update_{nullptr};
      std::function<void(bool)>  blower_state_update_{nullptr};
      std::function<void(float)>  current_temp_state_update_{nullptr};
      std::function<void(float)>  target_temp_state_update_{nullptr};
  CircularBuffer<uint8_t, 35> Q_in;
  CircularBuffer<uint8_t, 35> Q_out;
  uint8_t x, i;

      uint8_t last_state_crc = 0x00;
      uint8_t send = 0x00;
      uint8_t settemp = 0x00;
      uint8_t sethour = 0x00;
      uint8_t setminute = 0x00;
      uint8_t id = 0x00;
      unsigned long lastrx = 0;
      char have_config = 0;
      char have_faultlog = 0;
      char have_filtersettings = 0;
  bool rest_state_known_{false};
  // Removed unused minute throttling variables

      struct
      {
        uint8_t jet1 : 2;
        uint8_t jet2 : 2;
        uint8_t blower : 1;
        uint8_t light : 1;
        uint8_t restmode : 1;
        uint8_t highrange : 1;
        uint8_t heater : 1;
        uint8_t hour : 5;
        uint8_t minutes : 6;
        uint8_t circ : 1;
      } SpaState;

      struct
      {
        uint8_t pump1 : 2;
        uint8_t pump2 : 2;
        uint8_t pump3 : 2;
        uint8_t pump4 : 2;
        uint8_t pump5 : 2;
        uint8_t pump6 : 2;
        uint8_t light1 : 1;
        uint8_t light2 : 1;
        uint8_t circ : 1;
        uint8_t blower : 1;
        uint8_t mister : 1;
        uint8_t aux1 : 1;
        uint8_t aux2 : 1;
        uint8_t temp_scale : 1;
      } SpaConfig;

      struct
      {
        uint8_t totEntry : 5;
        uint8_t currEntry : 5;
        uint8_t faultCode : 6;
        std::string faultMessage;
        uint8_t daysAgo : 8;
        uint8_t hour : 5;
        uint8_t minutes : 6;
      } SpaFaultLog;

      struct
      {
        uint8_t filt1Hour : 5;
        uint8_t filt1Minute : 6;
        uint8_t filt1DurationHour : 5;
        uint8_t filt1DurationMinute : 6;
        uint8_t filt2Enable : 1;
        uint8_t filt2Hour : 5;
        uint8_t filt2Minute : 6;
        uint8_t filt2DurationHour : 5;
        uint8_t filt2DurationMinute : 6;
      } SpaFilterSettings;

      uint8_t crc8(CircularBuffer<uint8_t, 35> &data);
      void ID_request();
      void ID_ack();
      void rs485_send();
      void decodeSettings();
      void decodeState();
      void decodeFilterSettings();
      void decodeFault();

      sensor::Sensor *temp_sensor_{nullptr};
      binary_sensor::BinarySensor *heater_binary_sensor_{nullptr};
      binary_sensor::BinarySensor *circ_binary_sensor_{nullptr};
      binary_sensor::BinarySensor *blower_binary_sensor_{nullptr};
      binary_sensor::BinarySensor *rest_binary_sensor_{nullptr};
      binary_sensor::BinarySensor *high_range_binary_sensor_{nullptr};
      sensor::Sensor *hour_sensor_{nullptr};
      sensor::Sensor *minute_sensor_{nullptr};
      text_sensor::TextSensor *fault_sensor_{nullptr};
  text_sensor::TextSensor *time_text_sensor_{nullptr};
      text_sensor::TextSensor *heating_status_text_sensor_{nullptr};
    };
  }
}
