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

void WavinAhc9000::add_temp_callback(int channel, std::function<void(float)> &&callback) {
  temp_callbacks_[channel].add(std::move(callback));
}

void WavinAhc9000::add_bat_level_callback(int channel, std::function<void(float)> &&callback) {
  bat_level_callbacks_[channel].add(std::move(callback));
}

void WavinAhc9000::add_target_temp_callback(int channel, std::function<void(float)> &&callback) {
  target_temp_callbacks_[channel].add(std::move(callback));
}

void WavinAhc9000::add_mode_callback(int channel, std::function<void(int)> &&callback) {
  mode_callbacks_[channel].add(std::move(callback));
}

void WavinAhc9000::add_output_callback(int channel, std::function<void(bool)> &&callback) {
  output_callbacks_[channel].add(std::move(callback));
}

void WavinAhc9000::set_target_temp(int channel, float temperature) {
  set_temp_.push_back(temperature);
  temp_channel_.push_back(channel);
}

void WavinAhc9000::send_read_(uint8_t function_code, uint16_t arg1, uint16_t arg2) {
  // Wavin custom read: [function][arg1 hi][arg1 lo][arg2 hi][arg2 lo].
  // send_pdu() prepends the device address and appends the CRC.
  const uint8_t pdu[5] = {function_code, (uint8_t) (arg1 >> 8), (uint8_t) (arg1 & 0xff),
                         (uint8_t) (arg2 >> 8), (uint8_t) (arg2 & 0xff)};
  this->send_pdu(std::span<const uint8_t>(pdu, sizeof(pdu)));
}

void WavinAhc9000::on_response(std::span<const uint8_t> request_pdu, std::span<const uint8_t> response_pdu) {
  this->waiting_ = false;
  // response_pdu is the PDU without address/CRC: [function_code][byte_count][payload...]
  if (response_pdu.size() < 2) {
    ESP_LOGW(TAG, "Invalid modbus response - too short");
    return;
  }
  // Skip function code + byte count to reach the payload, matching the original parser.
  std::vector<uint8_t> payload(response_pdu.begin() + 2, response_pdu.end());

  float temperature;
  switch (state_) {
    case 0:
      temperature = ((payload[0] << 8) + payload[1]) / 10.0;
      ESP_LOGD(TAG, "Confirmed target temperature channel %i: %.1f", channel_ + 1, temperature);
      channel_ = -1;
      break;
    case 1:
      handle_channel_data_(payload);
      break;
    case 2:
      handle_element_data_(payload);
      break;
    case 3:
      handle_target_temp_data_(payload);
      break;
    case 4:
      handle_mode_data_(payload);
      break;
  }
}

bool WavinAhc9000::on_no_response() {
  this->waiting_ = false;
  if (state_ == 0) {
    ESP_LOGD(TAG, "Timeout on set temperature on channel %d", channel_ + 1);
    channel_ = -1;
  } else {
    ESP_LOGD(TAG, "Timeout on channel %d, state %d", channel_ + 1, state_);
    state_ = 4;  // skip remaining reads for this channel
  }
  return false;  // do not ask the hub to retry
}

void WavinAhc9000::handle_channel_data_(const std::vector<uint8_t> &data) {
  element_ = (data[5] & PRIMARY_ELEMENT_MASK) - 1;
  if (element_ == -1) { // this channel isn't used
    ESP_LOGV(TAG, "Channel %d isn't used", channel_ + 1);
    state_ = 4;
    return;
  }
  if (data[0] & ALL_TP_LOST_MASK) {
    ESP_LOGD(TAG, "All TP lost for channel %d", channel_ + 1);
    state_++; // skip temp and bat data
  }
  bool output_on = data[1] & CHANNEL_OUTP_ON;
  ESP_LOGD(TAG, "Status channel %i: %s",channel_ + 1, ONOFF(output_on));
  output_callbacks_[channel_].call(output_on);
}

void WavinAhc9000::handle_element_data_(const std::vector<uint8_t> &data) {
  float temperature = ((data[0] << 8) + data[1]) / 10.0;
  int battery = data[13] * 10;
  ESP_LOGD(TAG, "Temperature channel %i: %.1f", channel_ + 1, temperature);
  ESP_LOGD(TAG, "Battery channel %i: %i", channel_ + 1, battery);
  temp_callbacks_[channel_].call(temperature);
  bat_level_callbacks_[channel_].call(battery);
}

void WavinAhc9000::handle_target_temp_data_(const std::vector<uint8_t> &data) {
  float temperature = ((data[0] << 8) + data[1]) / 10.0;
  ESP_LOGD(TAG, "Target temperature channel %i: %.1f", channel_ + 1, temperature);
  target_temp_callbacks_[channel_].call(temperature);
}

void WavinAhc9000::handle_mode_data_(const std::vector<uint8_t> &data) {
  int mode = data[0] & MODE_MASK;
  ESP_LOGD(TAG, "Mode channel %i: %d",channel_ + 1, mode );
  mode_callbacks_[channel_].call(mode);
}

void WavinAhc9000::loop() {
  // One transaction in flight at a time; on_response()/on_no_response() clear this.
  if (this->waiting_)
    return;

  if (set_temp_.size() && (channel_ < 0)) {
    int temperature = ((roundf(set_temp_.front() * 2.0) / 2) * 10);
    ESP_LOGV(TAG, "Rounded to nearest half for channel %d: %d", channel_ + 1, temperature);
    set_temp_.erase(set_temp_.begin());
    channel_ = temp_channel_.front();
    temp_channel_.erase(temp_channel_.begin());
    ESP_LOGV(TAG, "Setting temperature for channel %d: %d", channel_ + 1, temperature);
    // Wavin custom write: [function][category][index][channel][count=1][value hi][value lo].
    const uint8_t pdu[7] = {MODBUS_WRITE_REGISTER, CATEGORY_PACKED_DATA, PACKED_DATA_MANUAL_TEMPERATURE,
                           (uint8_t) channel_, 1, (uint8_t) (temperature >> 8), (uint8_t) (temperature & 0xff)};
    this->send_pdu(std::span<const uint8_t>(pdu, sizeof(pdu)));
    this->waiting_ = true;
    return;
  }

  if (start_scan_) {
    start_scan_ = false;
    channel_ = 0;
    // Find first configured channel
    while (channel_ < 16 && !used_channels_[channel_]) {
      channel_++;
    }
    if (channel_ >= 16) {
      channel_ = -1;
      return;
    }
  }
  if (channel_ < 0)
    return;
  if (++state_ > 4) {
    // Move to next configured channel
    do {
      channel_++;
    } while (channel_ < 16 && !used_channels_[channel_]);

    if (channel_ >= 16) {
      state_ = 0;
      channel_ = -1;
      return;
    }
    state_ = 1;
  }

  ESP_LOGV(TAG, "Sending for channel %d, state %d", channel_ + 1, state_);
  switch(state_) {
    case 1:
      send_read_(MODBUS_READ_REGISTER, (CATEGORY_CHANNELS << 8) + 0, (channel_ << 8) + 3);
      break;
    case 2:
      ESP_LOGV(TAG, "Reading data for element %d", element_);
      send_read_(MODBUS_READ_REGISTER, (CATEGORY_ELEMENTS << 8) + 4, (element_ << 8) + 7);
      break;
    case 3:
      send_read_(MODBUS_READ_REGISTER, (CATEGORY_PACKED_DATA << 8) + PACKED_DATA_MANUAL_TEMPERATURE, (channel_ << 8) + 1);
      break;
    case 4:
      send_read_(MODBUS_READ_REGISTER, (CATEGORY_PACKED_DATA << 8) + PACKED_DATA_CONFIGURATION, (channel_ << 8) + 1);
      break;
  }
  this->waiting_ = true;
}

void WavinAhc9000::update() {
  start_scan_ = true;
}

} // wavinAhc9000
} // esphome
