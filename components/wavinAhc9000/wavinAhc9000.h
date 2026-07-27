#pragma once
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/modbus/modbus.h"

#include <span>
#include <vector>

namespace esphome {
namespace wavinAhc9000 {

class WavinAhc9000 : public PollingComponent, public modbus::ModbusClientDevice {
  public:
    void update() override;
    void loop() override;

    // New modbus client hooks (replace the old on_modbus_data path).
    void on_response(std::span<const uint8_t> request_pdu, std::span<const uint8_t> response_pdu) override;
    bool on_no_response() override;
    void on_not_sent() override { this->waiting_ = false; }

    void register_channel(int channel) { used_channels_[channel] = true; }
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
    // Build a Wavin custom-function-code read PDU (function code + two 16-bit args) and queue it.
    void send_read_(uint8_t function_code, uint16_t arg1, uint16_t arg2);

    int channel_ = -1;
    int state_ = 0;
    int element_ = 0;
    bool start_scan_ = false;
    bool waiting_ = false;
    bool used_channels_[16] = {false};
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
