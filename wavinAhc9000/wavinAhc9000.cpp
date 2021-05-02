#include "wavinAhc9000.h"
#include "HardwareSerial.h"
#include "esphome/core/log.h"
#include "WavinController.h"

namespace esphome {
namespace wavinAhc9000 {

static const char *TAG = "wavin";
unsigned long lastUpdateTime = 0;
const uint16_t POLL_TIME_MS = 300000;
const uint8_t TX_ENABLE_PIN = 25;
const uint16_t RECIEVE_TIMEOUT_MS = 1000;
WavinController wavinController(TX_ENABLE_PIN, RECIEVE_TIMEOUT_MS);

uint16_t registers[11];
uint16_t primaryElement = registers[0] & WavinController::CHANNELS_PRIMARY_ELEMENT_ELEMENT_MASK;
bool allThermostatsLost = registers[0] & WavinController::CHANNELS_PRIMARY_ELEMENT_ALL_TP_LOST_MASK;   

void WavinAhc9000::add_target_temp_callback(std::function<void(float,uint8_t)> &&callback) { target_temp_callback_.add(std::move(callback)); }
void WavinAhc9000::add_fan_speed_callback(std::function<void(int)> &&callback) { fan_speed_callback_.add(std::move(callback)); }

void WavinAhc9000::prep()
{
  txEnablePin = 25;
  pinMode(txEnablePin, OUTPUT);
  digitalWrite(txEnablePin, LOW);

  recieveTimeout_ms = 1000;

  Serial2.begin(38400);


}
// ------------------------------------------------
// Functions for reading from Wavin AHC 9000
// Read the current setpoint programmed for channel
void WavinAhc9000::readsetpoint(uint8_t channel)
{
  if (wavinController.readRegisters(WavinController::CATEGORY_PACKED_DATA, channel, WavinController::PACKED_DATA_MANUAL_TEMPERATURE, 1, registers))
  {
    float float_setpoint = (((float)registers[0]) / 10);
    ESP_LOGD(TAG, "Setpoint %i: %f",channel, float_setpoint );
  }
}

// Read the current mode for the channel
void WavinAhc9000::readmode(uint8_t channel)
{
  if (wavinController.readRegisters(WavinController::CATEGORY_PACKED_DATA, channel, WavinController::PACKED_DATA_CONFIGURATION, 1, registers))
  {
    float float_mode = ((float)(registers[0] & WavinController::PACKED_DATA_CONFIGURATION_MODE_MASK));
    ESP_LOGD(TAG, "Mode %i: %f",channel, float_mode );
  }
}

// Read the current status of the output for channel
void WavinAhc9000::readstatus(uint8_t channel)
{
  if (wavinController.readRegisters(WavinController::CATEGORY_CHANNELS, channel, WavinController::CHANNELS_TIMER_EVENT, 1, registers))
  {
    uint16_t status = registers[0] & WavinController::CHANNELS_TIMER_EVENT_OUTP_ON_MASK;
    String payload;
    if (status & WavinController::CHANNELS_TIMER_EVENT_OUTP_ON_MASK)
    {
      payload = "on";
    }
    else
    {
      payload = "on";
    }
    ESP_LOGD(TAG, "Status %i: %s",channel, payload );
  }
}

// If a thermostat for the channel is connected to the controller
void WavinAhc9000::readtempandbat(uint8_t channel)
{
  uint16_t primaryElement = registers[0] & WavinController::CHANNELS_PRIMARY_ELEMENT_ELEMENT_MASK;
  bool allThermostatsLost = registers[0] & WavinController::CHANNELS_PRIMARY_ELEMENT_ALL_TP_LOST_MASK;   

  if(!allThermostatsLost)
  {
    // Read values from the primary thermostat connected to this channel 
    // Primary element from controller is returned as index+1, so 1 i subtracted here to read the correct element
    if (wavinController.readRegisters(WavinController::CATEGORY_ELEMENTS, primaryElement-1, 0, 11, registers))
    {
      float temperature = (((float)registers[WavinController::ELEMENTS_AIR_TEMPERATURE])/10);
//      float battery = (((float)registers[WavinController::ELEMENTS_BATTERY_STATUS])*10); // In 10% steps
      int battery = (((int)registers[WavinController::ELEMENTS_BATTERY_STATUS])*10); // In 10% steps
      

      ESP_LOGD(TAG, "Tempetature channel %i: %f",channel,temperature );
      ESP_LOGD(TAG, "Battery channel %i: %i",channel,battery );
      if(channel == 0)
      {
        if(this->temp_c1_sensor_ != nullptr)
          this->temp_c1_sensor_ ->publish_state(temperature);
        if(this->bat_c1_sensor_ != nullptr)
          this->bat_c1_sensor_ ->publish_state(battery);
      }
      if(channel == 1)
      {
        if(this->temp_c2_sensor_ != nullptr)
          this->temp_c2_sensor_ ->publish_state(temperature);
        if(this->bat_c2_sensor_ != nullptr)
          this->bat_c2_sensor_ ->publish_state(battery);
      }
      if(channel == 2)
      {
        if(this->temp_c3_sensor_ != nullptr)
          this->temp_c3_sensor_ ->publish_state(temperature);
        if(this->bat_c3_sensor_ != nullptr)
          this->bat_c3_sensor_ ->publish_state(battery);
      }
      if(channel == 3)
      {
        if(this->temp_c4_sensor_ != nullptr)
          this->temp_c4_sensor_ ->publish_state(temperature);
        if(this->bat_c4_sensor_ != nullptr)
          this->bat_c4_sensor_ ->publish_state(battery);
      }
      if(channel == 4)
      {
        if(this->temp_c5_sensor_ != nullptr)
          this->temp_c5_sensor_ ->publish_state(temperature);
        if(this->bat_c5_sensor_ != nullptr)
          this->bat_c5_sensor_ ->publish_state(battery);
      }
      if(channel == 5)
      {
        if(this->temp_c6_sensor_ != nullptr)
          this->temp_c6_sensor_ ->publish_state(temperature);
        if(this->bat_c6_sensor_ != nullptr)
          this->bat_c6_sensor_ ->publish_state(battery);
      }
      if(channel == 6)
      {
        if(this->temp_c7_sensor_ != nullptr)
          this->temp_c7_sensor_ ->publish_state(temperature);
        if(this->bat_c7_sensor_ != nullptr)
          this->bat_c7_sensor_ ->publish_state(battery);
      }
      if(channel == 7)
      {
        if(this->temp_c8_sensor_ != nullptr)
          this->temp_c8_sensor_ ->publish_state(temperature);
        if(this->bat_c8_sensor_ != nullptr)
          this->bat_c8_sensor_ ->publish_state(battery);
      }                  
      if(channel == 8)
      {
        if(this->temp_c9_sensor_ != nullptr)
          this->temp_c9_sensor_ ->publish_state(temperature);
        if(this->bat_c9_sensor_ != nullptr)
          this->bat_c9_sensor_ ->publish_state(battery);
      }
      if(channel == 9)
      {
        if(this->temp_c10_sensor_ != nullptr)
          this->temp_c10_sensor_ ->publish_state(temperature);
        if(this->bat_c10_sensor_ != nullptr)
          this->bat_c10_sensor_ ->publish_state(battery);
      }
      if(channel == 10)
      {
        if(this->temp_c11_sensor_ != nullptr)
          this->temp_c11_sensor_ ->publish_state(temperature);
        if(this->bat_c11_sensor_ != nullptr)
          this->bat_c11_sensor_ ->publish_state(battery);
      }
      if(channel == 11)
      {
        if(this->temp_c12_sensor_ != nullptr)
          this->temp_c12_sensor_ ->publish_state(temperature);
        if(this->bat_c12_sensor_ != nullptr)
          this->bat_c12_sensor_ ->publish_state(battery);
      }
      if(channel == 12)
      {
        if(this->temp_c13_sensor_ != nullptr)
          this->temp_c13_sensor_ ->publish_state(temperature);
        if(this->bat_c13_sensor_ != nullptr)
          this->bat_c13_sensor_ ->publish_state(battery);
      }
      if(channel == 13)
      {
        if(this->temp_c14_sensor_ != nullptr)
          this->temp_c14_sensor_ ->publish_state(temperature);
        if(this->bat_c14_sensor_ != nullptr)
          this->bat_c14_sensor_ ->publish_state(battery);
      }
      if(channel == 14)
      {
        if(this->temp_c15_sensor_ != nullptr)
          this->temp_c15_sensor_ ->publish_state(temperature);
        if(this->bat_c15_sensor_ != nullptr)
          this->bat_c15_sensor_ ->publish_state(battery);
      }
      if(channel == 15)
      {
        if(this->temp_c16_sensor_ != nullptr)
          this->temp_c16_sensor_ ->publish_state(temperature);
        if(this->bat_c16_sensor_ != nullptr)
          this->bat_c16_sensor_ ->publish_state(battery);
      }                                    
    }
  }
}

// ------------------------------------------------

// ------------------------------------------------
// Functions for writing to Wavin AHC 9000
void WavinAhc9000::writeFanMode(int new_fan_speed)
{
	
	//ESP_LOGD(TAG, "Writing new fan speed to system.... (%i)",new_fan_speed);
	//this->send(CMD_WRITE_SINGLE_REG, 100, new_fan_speed);
}

void WavinAhc9000::writeTargetTemperature(float new_target_temp)
{
	
	//ESP_LOGD(TAG, "Writing new fan speed to system.... (%i)",new_fan_speed);
	//this->send(CMD_WRITE_SINGLE_REG, 100, new_fan_speed);
}
// ------------------------------------------------




void WavinAhc9000::setup()
{
  prep();
  //readsetpoint(1);
}

void WavinAhc9000::loop() 
{
  if (lastUpdateTime + POLL_TIME_MS < millis())
  {
    //prep();
    lastUpdateTime = millis();

    //uint16_t registers[11];
      for(uint8_t channel = 0; channel < WavinController::NUMBER_OF_CHANNELS; channel++)
      {
        if (wavinController.readRegisters(WavinController::CATEGORY_CHANNELS, channel, WavinController::CHANNELS_PRIMARY_ELEMENT, 1, registers))
        {
          readtempandbat(channel);
          readmode(channel);
          readsetpoint(channel);
          readstatus(channel);
        }
        
      }
  }
}

void WavinAhc9000::update() {  }


















// TO BE DELETED
/* // Returns temperature in degrees with one decimal
float temperatureAsFloatString(uint16_t temperature)
{
  float temperatureAsFloat = ((float)temperature) / 10;
  return temperatureAsFloat;
}





unsigned int WavinAhc9000::calculateCRC(uint8_t *frame, uint8_t bufferSize)
{
  uint16_t temp = 0xFFFF;
  bool flag;

  for (uint8_t i = 0; i < bufferSize; i++)
  {
    temp = temp ^ frame[i];
    for (uint8_t j = 1; j <= 8; j++)
    {
      flag = temp & 0x0001;
      temp >>= 1;
      if (flag)
      {
        temp ^= 0xA001;
      }
    }
  }

  return temp;
}


bool WavinAhc9000::recieve(uint16_t *reply, uint8_t cmdtype)
{
  uint8_t buffer[RECIEVE_BUFFER_SIZE];
  uint8_t n = 0;
  unsigned long start_time = millis();

  while (millis() - start_time < recieveTimeout_ms)
  {
    while (Serial2.available() && n<RECIEVE_BUFFER_SIZE)
    {
      buffer[n] = Serial2.read();
      ESP_LOGD(TAG,"RB %i: %u", n,buffer[n]);
      n++;

      if (n > 5 &&
        buffer[0] == MODBUS_DEVICE &&
        buffer[1] == cmdtype &&
        buffer[2] + 5 == n)
      {
        // Complete package
        uint16_t crc = calculateCRC(buffer, n);
        if (crc != 0) return false;

        // CRC ok, copy to reply buffer
        for (int j = 0; j < buffer[2] / 2; j++)
        {
          
          reply[j] = (buffer[3 + j * 2] << 8) + buffer[4 + j * 2];
          ESP_LOGD(TAG,"reply  %u", reply[j]);
        }

        return true;
      }
    }
  }
  return false;
}


void WavinAhc9000::transmit(uint8_t *data, uint8_t lenght)
{
  // Empty recieve buffer before sending
  while (Serial2.read() != -1);

  digitalWrite(txEnablePin, HIGH);
  
  Serial2.write(data, lenght);

  Serial2.flush(); // Wait for data to be sent
  delayMicroseconds(250); // Wait for last char to be transmitted

  digitalWrite(txEnablePin, LOW);
}


bool WavinAhc9000::readRegisters(uint8_t category, uint8_t page, uint8_t index, uint8_t count, uint16_t *reply)
{
  uint8_t message[8];
  message[0] = MODBUS_DEVICE;
  message[1] = MODBUS_READ_REGISTER;
  message[2] = category; // 2
  message[3] = index; // 0
  message[4] = page; // 4
  message[5] = count; //1

  uint16_t crc = calculateCRC(message, 6);

  message[6] = crc & 0xFF;
  message[7] = crc >> 8;
  Serial.print("0 - Device address: ");
  Serial.println((message[0]));
  Serial.print("1 - Read register: ");
  Serial.println((message[1]));
  Serial.print("2 - category: ");
  Serial.println((message[2]));
  Serial.print("3 - index: ");
  Serial.println((message[3]));
  Serial.print("4 - page: ");
  Serial.println((message[4]));
  Serial.print("5 - count: ");
  Serial.println((message[5]));
  Serial.print("6 - crc & 0xFF: ");
  Serial.println((message[6]));
  Serial.print("7 - crc >> 8: ");
  Serial.println((message[7]));

  transmit(message, 8);
  
  return recieve(reply, MODBUS_READ_REGISTER);
}


bool WavinAhc9000::writeRegister(uint8_t category, uint8_t page, uint8_t index, uint16_t value)
{
  uint8_t message[10];

  message[0] = MODBUS_DEVICE;
  message[1] = MODBUS_WRITE_REGISTER;
  message[2] = category;
  message[3] = index;
  message[4] = page;
  message[5] = 1;
  message[6] = value >> 8;
  message[7] = value & 0xFF;

  uint16_t crc = calculateCRC(message, 8);

  message[8] = crc & 0xFF;
  message[9] = crc >> 8;

  transmit(message, 10);

  uint16_t reply[1];
  return recieve(reply, MODBUS_WRITE_REGISTER); // Recieve reply but ignore it. Asume it's ok
}

bool WavinAhc9000::writeMaskedRegister(uint8_t category, uint8_t page, uint8_t index, uint16_t value, uint16_t mask)
{
  uint8_t message[12];

  message[0] = MODBUS_DEVICE;
  message[1] = MODBUS_WRITE_MASKED_REGISTER;
  message[2] = category;
  message[3] = index;
  message[4] = page;
  message[5] = 1;
  message[6] = value >> 8;
  message[7] = value & 0xFF;
  message[8] = mask >> 8;
  message[9] = mask & 0xFF;

  uint16_t crc = calculateCRC(message, 10);

  message[10] = crc & 0xFF;
  message[11] = crc >> 8;

  transmit(message, 12);

  uint16_t reply[1];
  return recieve(reply, MODBUS_WRITE_MASKED_REGISTER); // Recieve reply but ignore it. Asume it's ok
} */
}
}