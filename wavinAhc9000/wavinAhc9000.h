#pragma once
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace wavinAhc9000 {

class WavinAhc9000 : public PollingComponent {
    public:

      void set_temp_c1_sensor(sensor::Sensor * temp_c1_sensor) { temp_c1_sensor_ = temp_c1_sensor; }
      void set_bat_c1_sensor(sensor::Sensor * bat_c1_sensor) { bat_c1_sensor_ = bat_c1_sensor; }
      void set_temp_c2_sensor(sensor::Sensor * temp_c2_sensor) { temp_c2_sensor_ = temp_c2_sensor; }
      void set_bat_c2_sensor(sensor::Sensor * bat_c2_sensor) { bat_c2_sensor_ = bat_c2_sensor; }
      void set_temp_c3_sensor(sensor::Sensor * temp_c3_sensor) { temp_c3_sensor_ = temp_c3_sensor; }
      void set_bat_c3_sensor(sensor::Sensor * bat_c3_sensor) { bat_c3_sensor_ = bat_c3_sensor; }
      void set_temp_c4_sensor(sensor::Sensor * temp_c4_sensor) { temp_c4_sensor_ = temp_c4_sensor; }
      void set_bat_c4_sensor(sensor::Sensor * bat_c4_sensor) { bat_c4_sensor_ = bat_c4_sensor; }
      void set_temp_c5_sensor(sensor::Sensor * temp_c5_sensor) { temp_c5_sensor_ = temp_c5_sensor; }
      void set_bat_c5_sensor(sensor::Sensor * bat_c5_sensor) { bat_c5_sensor_ = bat_c5_sensor; }
      void set_temp_c6_sensor(sensor::Sensor * temp_c6_sensor) { temp_c6_sensor_ = temp_c6_sensor; }
      void set_bat_c6_sensor(sensor::Sensor * bat_c6_sensor) { bat_c6_sensor_ = bat_c6_sensor; }
      void set_temp_c7_sensor(sensor::Sensor * temp_c7_sensor) { temp_c7_sensor_ = temp_c7_sensor; }
      void set_bat_c7_sensor(sensor::Sensor * bat_c7_sensor) { bat_c7_sensor_ = bat_c7_sensor; }
      void set_temp_c8_sensor(sensor::Sensor * temp_c8_sensor) { temp_c8_sensor_ = temp_c8_sensor; }
      void set_bat_c8_sensor(sensor::Sensor * bat_c8_sensor) { bat_c8_sensor_ = bat_c8_sensor; }
      void set_temp_c9_sensor(sensor::Sensor * temp_c9_sensor) { temp_c9_sensor_ = temp_c9_sensor; }
      void set_bat_c9_sensor(sensor::Sensor * bat_c9_sensor) { bat_c9_sensor_ = bat_c9_sensor; }
      void set_temp_c10_sensor(sensor::Sensor * temp_c10_sensor) { temp_c10_sensor_ = temp_c10_sensor; }
      void set_bat_c10_sensor(sensor::Sensor * bat_c10_sensor) { bat_c10_sensor_ = bat_c10_sensor; }
      void set_temp_c11_sensor(sensor::Sensor * temp_c11_sensor) { temp_c11_sensor_ = temp_c11_sensor; }
      void set_bat_c11_sensor(sensor::Sensor * bat_c11_sensor) { bat_c11_sensor_ = bat_c11_sensor; }
      void set_temp_c12_sensor(sensor::Sensor * temp_c12_sensor) { temp_c12_sensor_ = temp_c12_sensor; }
      void set_bat_c12_sensor(sensor::Sensor * bat_c12_sensor) { bat_c12_sensor_ = bat_c12_sensor; }
      void set_temp_c13_sensor(sensor::Sensor * temp_c13_sensor) { temp_c13_sensor_ = temp_c13_sensor; }
      void set_bat_c13_sensor(sensor::Sensor * bat_c13_sensor) { bat_c13_sensor_ = bat_c13_sensor; }
      void set_temp_c14_sensor(sensor::Sensor * temp_c14_sensor) { temp_c14_sensor_ = temp_c14_sensor; }
      void set_bat_c14_sensor(sensor::Sensor * bat_c14_sensor) { bat_c14_sensor_ = bat_c14_sensor; }
      void set_temp_c15_sensor(sensor::Sensor * temp_c15_sensor) { temp_c15_sensor_ = temp_c15_sensor; }
      void set_bat_c15_sensor(sensor::Sensor * bat_c15_sensor) { bat_c15_sensor_ = bat_c15_sensor; }
      void set_temp_c16_sensor(sensor::Sensor * temp_c16_sensor) { temp_c16_sensor_ = temp_c16_sensor; }
      void set_bat_c16_sensor(sensor::Sensor * bat_c16_sensor) { bat_c16_sensor_ = bat_c16_sensor; }

    void setup();
    void prep();
    void loop() override;
    void update() override;

    bool readRegisters(uint8_t category, uint8_t page, uint8_t index, uint8_t count, uint16_t *reply);
    bool writeRegister(uint8_t category, uint8_t page, uint8_t index, uint16_t value);
    bool writeMaskedRegister(uint8_t category, uint8_t page, uint8_t index, uint16_t value, uint16_t mask);
    float temperatureAsFloatString(uint16_t temperature);

    static const uint8_t CATEGORY_MAIN =        0x00;
    static const uint8_t CATEGORY_ELEMENTS =    0x01;
    static const uint8_t CATEGORY_PACKED_DATA = 0x02;//
    static const uint8_t CATEGORY_CHANNELS =    0x03;
    static const uint8_t CATEGORY_RELAYS =      0x04;
    static const uint8_t CATEGORY_CLOCK =       0x05;
    static const uint8_t CATEGORY_SCHEDULES =   0x06;
    static const uint8_t CATEGORY_INFO =        0x07;

    static const uint8_t ELEMENTS_AIR_TEMPERATURE = 0x04;
    static const uint8_t ELEMENTS_BATTERY_STATUS  = 0x0A;
    static const uint8_t ELEMENTS_SYNC_GROUP      = 0x0B;

    static const uint8_t PACKED_DATA_MANUAL_TEMPERATURE = 0x00; //
    static const uint8_t PACKED_DATA_STANDBY_TEMPERATURE = 0x04;
    static const uint8_t PACKED_DATA_CONFIGURATION = 0x07;
    static const uint8_t PACKED_DATA_CONFIGURATION_MODE_MASK = 0x07;
    static const uint8_t PACKED_DATA_CONFIGURATION_MODE_MANUAL = 0x00;
    static const uint8_t PACKED_DATA_CONFIGURATION_MODE_STANDBY = 0x01;

    static const uint8_t  NUMBER_OF_CHANNELS = 16;
    static const uint8_t  CHANNELS_TIMER_EVENT = 0x00;
    static const uint16_t CHANNELS_TIMER_EVENT_OUTP_ON_MASK = 0x0010;
    static const uint8_t  CHANNELS_PRIMARY_ELEMENT = 0x02;
    static const uint16_t CHANNELS_PRIMARY_ELEMENT_ELEMENT_MASK = 0x003f;
    static const uint16_t CHANNELS_PRIMARY_ELEMENT_ALL_TP_LOST_MASK = 0x0400;

    void add_target_temp_callback(std::function<void(float)> &&callback);
    void add_fan_speed_callback(std::function<void(int)> &&callback);
    void writeFanMode(int new_fan_speed);
    void writeTargetTemperature(float new_target_temp);
    
    void readstatus(uint8_t channel);
    void readtempandbat(uint8_t channel);
    void readsetpoint(uint8_t channel);
    void readmode(uint8_t channel);


    void add_target_temp_callback(std::function<void(float,uint8_t)> &&callback);


  private:
    uint8_t txEnablePin;
    uint16_t recieveTimeout_ms;
    //void transmit(uint8_t *data, uint8_t lenght);
    //bool recieve(uint16_t *reply, uint8_t cmdtype);
    //unsigned int calculateCRC(unsigned char *frame, unsigned char bufferSize);

    const uint8_t MODBUS_DEVICE = 0x01;
    const uint8_t MODBUS_READ_REGISTER = 0x43;
    const uint8_t MODBUS_WRITE_REGISTER = 0x44;
    const uint8_t MODBUS_WRITE_MASKED_REGISTER = 0x45;
    
    // Largest page contains 22 registers of 2 bytes + 5 bytes header
    const uint8_t RECIEVE_BUFFER_SIZE =  22 * 2 + 5;    
  
  protected:
    sensor::Sensor *temp_c1_sensor_; 
    sensor::Sensor *bat_c1_sensor_;
    sensor::Sensor *temp_c2_sensor_; 
    sensor::Sensor *bat_c2_sensor_;
    sensor::Sensor *temp_c3_sensor_; 
    sensor::Sensor *bat_c3_sensor_;
    sensor::Sensor *temp_c4_sensor_; 
    sensor::Sensor *bat_c4_sensor_;
    sensor::Sensor *temp_c5_sensor_; 
    sensor::Sensor *bat_c5_sensor_;
    sensor::Sensor *temp_c6_sensor_; 
    sensor::Sensor *bat_c6_sensor_;
    sensor::Sensor *temp_c7_sensor_; 
    sensor::Sensor *bat_c7_sensor_;
    sensor::Sensor *temp_c8_sensor_; 
    sensor::Sensor *bat_c8_sensor_;
    sensor::Sensor *temp_c9_sensor_; 
    sensor::Sensor *bat_c9_sensor_;
    sensor::Sensor *temp_c10_sensor_; 
    sensor::Sensor *bat_c10_sensor_;
    sensor::Sensor *temp_c11_sensor_; 
    sensor::Sensor *bat_c11_sensor_;
    sensor::Sensor *temp_c12_sensor_; 
    sensor::Sensor *bat_c12_sensor_;
    sensor::Sensor *temp_c13_sensor_; 
    sensor::Sensor *bat_c13_sensor_;
    sensor::Sensor *temp_c14_sensor_; 
    sensor::Sensor *bat_c14_sensor_;
    sensor::Sensor *temp_c15_sensor_; 
    sensor::Sensor *bat_c15_sensor_;
    sensor::Sensor *temp_c16_sensor_; 
    sensor::Sensor *bat_c16_sensor_;
    CallbackManager<void(float,uint8_t)> target_temp_callback_;
    CallbackManager<void(int)> fan_speed_callback_;
};
}
}