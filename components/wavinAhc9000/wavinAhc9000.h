#pragma once
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/modbus/modbus.h"

namespace esphome {
namespace wavinAhc9000 {

class WavinAhc9000 : public PollingComponent, public modbus::ModbusDevice {
  public:
    void setup();
    void update() override;
    void loop() override;
    void on_modbus_data(const std::vector<uint8_t> &data) override;

    void set_rw_pin(GPIOPin *pin) { rw_pin_ = pin; }
    void add_temp_callback(int channel, std::function<void(float)> &&callback);
    void add_bat_level_callback(int channel, std::function<void(float)> &&callback);
    void add_target_temp_callback(int channel, std::function<void(float)> &&callback);
    void add_mode_callback(int channel, std::function<void(int)> &&callback);
    void add_output_callback(int channel, std::function<void(bool)> &&callback);
    void set_target_temp(int channel, float temperature);

  private:
    void handle_channel_data_(const std::vector<uint8_t> &data);
    void handle_element_data_(const std::vector<uint8_t> &data);
    void handle_target_temp_data_(const std::vector<uint8_t> &data);
    void handle_mode_data_(const std::vector<uint8_t> &data);

    GPIOPin *rw_pin_;
    int channel_ = -1;
    int state_ = 0;
    int element_ = 0;
    bool start_scan_ = false;
    bool waiting_ = false;
    std::vector<float> set_temp_;
    std::vector<float> temp_channel_;

    CallbackManager<void(float)> temp_callbacks_[16];
    CallbackManager<void(float)> bat_level_callbacks_[16];
    CallbackManager<void(float)> target_temp_callbacks_[16];
    CallbackManager<void(int)> mode_callbacks_[16];
    CallbackManager<void(bool)> output_callbacks_[16];
};
}
}
