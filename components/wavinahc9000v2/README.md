## V2 - Wavin

To make it easy to name the entities we make use of substitutions in ESPHome. Add this at the top of your yaml file and edit to your needs:
```yaml
substitutions:
  # UNIQUE NAME FOR THE DEVICE
  device: wavin # Case sensitive!!!
  name: Wavin   # "Friendly name" - not case sensitive!!!
  
  # CHANNEL friendly names (If using spaces remember to add ")
  channel_01_friendly_name: Livingroom
  channel_02_friendly_name: Kitchen
  channel_03_friendly_name: Toilet
  channel_04_friendly_name: "Big room 1"
  channel_05_friendly_name: "Channel 05"
  channel_06_friendly_name: "Channel 06"
  channel_07_friendly_name: "Channel 07"
  channel_08_friendly_name: "Channel 08"
  channel_09_friendly_name: "Channel 09"
  channel_10_friendly_name: "Channel 10"
  channel_11_friendly_name: "Channel 11"
  channel_12_friendly_name: "Channel 12"
  channel_13_friendly_name: "Channel 13"
  channel_14_friendly_name: "Channel 14"
  channel_15_friendly_name: "Channel 15"
  channel_16_friendly_name: "Channel 16"
  
  # CHANNEL ID´S (ONLY LOWER CASE LETTERS, NO SPACES) used for entity ids
  channel_01_id: livingroom
  channel_02_id: kitchen
  channel_03_id: toilet
  channel_04_id: big_room_1
  channel_05_id: channel_05
  channel_06_id: channel_06
  channel_07_id: channel_07
  channel_08_id: channel_08
  channel_09_id: channel_09
  channel_10_id: channel_10
  channel_11_id: channel_11
  channel_12_id: channel_12
  channel_13_id: channel_13
  channel_14_id: channel_14
  channel_15_id: channel_15
  channel_16_id: channel_16

  # CHANNEL Alignments (ONLY change if you have offsets in channel vs temperature. Some have reported this if using multiple strings pr. room thermostat)
  channel_01_sensor: "0x00"
  channel_02_sensor: "0x01"
  channel_03_sensor: "0x02"
  channel_04_sensor: "0x03"
  channel_05_sensor: "0x04"
  channel_06_sensor: "0x05"
  channel_07_sensor: "0x06"
  channel_08_sensor: "0x07"
  channel_09_sensor: "0x08"
  channel_10_sensor: "0x09"
  channel_11_sensor: "0x0A"
  channel_12_sensor: "0x0B"
  channel_13_sensor: "0x0C"
  channel_14_sensor: "0x0D"
  channel_15_sensor: "0x0E"
  channel_16_sensor: "0x0F"

  channel_01: "0x00"
  channel_02: "0x01"
  channel_03: "0x02"
  channel_04: "0x03"
  channel_05: "0x04"
  channel_06: "0x05"
  channel_07: "0x06"
  channel_08: "0x07"
  channel_09: "0x08"
  channel_10: "0x09"
  channel_11: "0x0A"
  channel_12: "0x0B"
  channel_13: "0x0C"
  channel_14: "0x0D"
  channel_15: "0x0E"
  channel_16: "0x0F"
```

Add this after your normal ESPhome config (wifi, ota, api, etc....) and uncomment the config lines for the active channels on your Wavin Ahc 9000
```yaml
# Uncomment the active channels on your Wavin Ahc 9000
packages:
  remote_package:
    url: https://github.com/heinekmadsen/esphome_components
    ref: main
    #ref: develop
    files: 
      - components/wavinahc9000v2/configs/basic.yaml # REQUIRED FOR WAVIN AHC 9000
      - components/wavinahc9000v2/configs/channel_01.yaml
      - components/wavinahc9000v2/configs/channel_01_comfort.yaml # Only for channels with thermostat with IR sensor
      - components/wavinahc9000v2/configs/channel_02.yaml
      - components/wavinahc9000v2/configs/channel_02_comfort.yaml # Only for channels with thermostat with IR sensor
      - components/wavinahc9000v2/configs/channel_03.yaml
      - components/wavinahc9000v2/configs/channel_03_comfort.yaml # Only for channels with thermostat with IR sensor
      - components/wavinahc9000v2/configs/channel_04.yaml 
      - components/wavinahc9000v2/configs/channel_04_comfort.yaml # Only for channels with thermostat with IR sensor
      #- components/wavinahc9000v2/configs/channel_05.yaml 
      #- components/wavinahc9000v2/configs/channel_05_comfort.yaml # Only for channels with thermostat with IR sensor
      #- components/wavinahc9000v2/configs/channel_06.yaml 
      #- components/wavinahc9000v2/configs/channel_06_comfort.yaml # Only for channels with thermostat with IR sensor
      #- components/wavinahc9000v2/configs/channel_07.yaml 
      #- components/wavinahc9000v2/configs/channel_07_comfort.yaml # Only for channels with thermostat with IR sensor
      #- components/wavinahc9000v2/configs/channel_08.yaml 
      #- components/wavinahc9000v2/configs/channel_08_comfort.yaml # Only for channels with thermostat with IR sensor
      #- components/wavinahc9000v2/configs/channel_09.yaml 
      #- components/wavinahc9000v2/configs/channel_09_comfort.yaml # Only for channels with thermostat with IR sensor
      #- components/wavinahc9000v2/configs/channel_10.yaml 
      #- components/wavinahc9000v2/configs/channel_10_comfort.yaml # Only for channels with thermostat with IR sensor
      #- components/wavinahc9000v2/configs/channel_11.yaml 
      #- components/wavinahc9000v2/configs/channel_11_comfort.yaml # Only for channels with thermostat with IR sensor
      #- components/wavinahc9000v2/configs/channel_12.yaml
      #- components/wavinahc9000v2/configs/channel_12_comfort.yaml # Only for channels with thermostat with IR sensor
      #- components/wavinahc9000v2/configs/channel_13.yaml 
      #- components/wavinahc9000v2/configs/channel_13_comfort.yaml # Only for channels with thermostat with IR sensor
      #- components/wavinahc9000v2/configs/channel_14.yaml
      #- components/wavinahc9000v2/configs/channel_14_comfort.yaml # Only for channels with thermostat with IR sensor
      #- components/wavinahc9000v2/configs/channel_15.yaml 
      #- components/wavinahc9000v2/configs/channel_15_comfort.yaml # Only for channels with thermostat with IR sensor
      #- components/wavinahc9000v2/configs/channel_16.yaml 
      #- components/wavinahc9000v2/configs/channel_16_comfort.yaml # Only for channels with thermostat with IR sensor
    refresh: 0s

uart:
  - id: uart_${device}
    rx_pin: GPIO16
    tx_pin: GPIO17
    baud_rate: 38400
    stop_bits: 1
    parity: NONE
    
modbus:
    - id: ${device}_modbus
      uart_id: uart_${device}

modbus_controller:
  id: ${device}_modbus_controller
  address: 1
  modbus_id: wavin_modbus
  update_interval: 5s
```

## Tips an Tricks
Depending on the hw you use, you may need to add "flow_control_pin: xx" to your yaml under modbus. Replace xx with the GPIO you use for flow control.