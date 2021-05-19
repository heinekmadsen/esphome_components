#include "wavinAhc9000.h"
#include "esphome/core/log.h"

namespace esphome {
namespace wavinAhc9000 {

static const char *TAG = "wavin";

static const uint8_t MODBUS_READ_REGISTER = 0x43;
static const uint8_t MODBUS_WRITE_REGISTER = 0x44;
static const uint8_t MODBUS_WRITE_MASKED_REGISTER = 0x45;

static const uint8_t CATEGORY_MAIN = 0x00;
static const uint8_t CATEGORY_ELEMENTS = 0x01;
static const uint8_t CATEGORY_PACKED_DATA = 0x02;
static const uint8_t CATEGORY_CHANNELS = 0x03;

static const uint8_t PACKED_DATA_MANUAL_TEMPERATURE = 0x00;
static const uint8_t PACKED_DATA_CONFIGURATION = 0x07;

static const uint8_t PRIMARY_ELEMENT_MASK = 0x3f;
static const uint8_t ALL_TP_LOST_MASK = 0x02;
static const uint8_t CHANNEL_OUTP_ON = 0x10;
static const uint8_t MODE_MASK = 0x07;

void WavinAhc9000::setup() {
  rw_pin_->pin_mode(OUTPUT);
  rw_pin_->digital_write(false);
}

void WavinAhc9000::add_temp_callback(int channel, std::function<void(float)> &&callback) {
  temp_callbacks_[channel].add(std::move(callback));
}

void WavinAhc9000::add_bat_callback(int channel, std::function<void(float)> &&callback) {
  bat_callbacks_[channel].add(std::move(callback));
}

void WavinAhc9000::add_target_temp_callback(int channel, std::function<void(float)> &&callback) {
  target_temp_callbacks_[channel].add(std::move(callback));
}

void WavinAhc9000::add_fan_speed_callback(int channel, std::function<void(int)> &&callback) {
  fan_speed_callbacks_[channel].add(std::move(callback));
}

void WavinAhc9000::add_output_callback(int channel, std::function<void(bool)> &&callback) {
  output_callbacks_[channel].add(std::move(callback));
}

void WavinAhc9000::on_modbus_data(const std::vector<uint8_t> &data) {
  ESP_LOGV(TAG, "Channel %d, state %d, Data: %s", channel_ + 1, state_, hexencode(data).c_str());
  switch (state_) {
    case 1:
      handle_channel_data_(data);
      break;
    case 2:
      handle_element_data_(data);
      break;
    case 3:
      handle_target_temp_data_(data);
      break;
    case 4:
      handle_mode_data_(data);
      break;
  }
  waiting_ = false;
}

void WavinAhc9000::handle_channel_data_(const std::vector<uint8_t> &data) {
  element_ = data[5] & PRIMARY_ELEMENT_MASK;
  if (element_ == 0) { // this channel isn't used
    ESP_LOGV(TAG, "Channel %d isn't used", channel_ + 1);
    state_ = 4;
    return;
  }
  if (data[0] & ALL_TP_LOST_MASK) {
    ESP_LOGD(TAG, "All TP lost for channel %d", channel_ + 1);
    state_++; // skip temp and bat data
  }
  bool output_on = data[0] & CHANNEL_OUTP_ON;
  ESP_LOGD(TAG, "Status channel %i: %s",channel_, ONOFF(output_on));
  output_callbacks_[channel_].call(output_on);
}

void WavinAhc9000::handle_element_data_(const std::vector<uint8_t> &data) {
  float temperature = ((data[0] << 8) + data[1]) / 10.0;
  int battery = data[13] * 10;
  ESP_LOGD(TAG, "Temperature channel %i: %f", channel_ + 1, temperature);
  ESP_LOGD(TAG, "Battery channel %i: %i", channel_ + 1, battery);
  temp_callbacks_[channel_].call(temperature);
  bat_callbacks_[channel_].call(battery);
}

void WavinAhc9000::handle_target_temp_data_(const std::vector<uint8_t> &data) {
  float temperature = ((data[0] << 8) + data[1]) / 10.0;
  ESP_LOGD(TAG, "Target temperature channel %i: %f", channel_ + 1, temperature);
  target_temp_callbacks_[channel_].call(temperature);
}

void WavinAhc9000::handle_mode_data_(const std::vector<uint8_t> &data) {
  int mode = data[0] & MODE_MASK;
  ESP_LOGD(TAG, "Mode channel %i: %d",channel_ + 1, mode );
}

void WavinAhc9000::loop() {
  static long last_update_time = 0;
  if (channel_ < 0)
    return;
  long now = millis();
  if (waiting_ && (last_update_time + 1000 < now)) {
    ESP_LOGD(TAG, "Timeout on channel %d, state %d", channel_ + 1, state_);
    state_ = 4;
    waiting_ = false;
  }
  if (waiting_)
    return;
  if (++state_ > 4) {
    if (++channel_ > 15) {
      state_ = 0;
      channel_ = -1;
      return;
    }
    state_ = 1;
  }

  rw_pin_->digital_write(true);
  ESP_LOGV(TAG, "Sending for channel %d, state %d", channel_ + 1, state_);
  switch(state_) {
    case 1:
      send(MODBUS_READ_REGISTER, (CATEGORY_CHANNELS << 8) + 0, (channel_ << 8) + 3);
      break;
    case 2:
      send(MODBUS_READ_REGISTER, (CATEGORY_ELEMENTS << 8) + 4, (element_ << 8) + 7);
      break;
    case 3:
      send(MODBUS_READ_REGISTER, (CATEGORY_PACKED_DATA << 8) + PACKED_DATA_MANUAL_TEMPERATURE, (channel_ << 8) + 1);
      break;
    case 4:
      send(MODBUS_READ_REGISTER, (CATEGORY_PACKED_DATA << 8) + PACKED_DATA_CONFIGURATION, (channel_ << 8) + 1);
      break;
  }
  parent_->flush();
  delay(1);
  rw_pin_->digital_write(false);
  waiting_ = true;
  last_update_time = now;
}

void WavinAhc9000::update() {
  if (waiting_) {
    ESP_LOGW(TAG, "Update interval overrun");
    return;
  }
  channel_ = 0;
  state_ = 0;
}

/*
// ------------------------------------------------
// Functions for writing to Wavin AHC 9000
void WavinAhc9000::writeFanMode_(int new_fan_speed)
{
	
	//ESP_LOGD(TAG, "Writing new fan speed to system.... (%i)",new_fan_speed);
	//this->send(CMD_WRITE_SINGLE_REG, 100, new_fan_speed);
}

void WavinAhc9000::writeTargetTemperature_(float new_target_temp)
{
	
	//ESP_LOGD(TAG, "Writing new fan speed to system.... (%i)",new_fan_speed);
	//this->send(CMD_WRITE_SINGLE_REG, 100, new_fan_speed);
}
*/

} // wavinAhc9000
} // esphome
