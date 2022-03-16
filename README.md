## ESPHome custom components

My creation of custom components fro ESP home.

### Recent updates
16-03-2022 - WavinAhc9000v2 - Inspired by Jopand's work on the nilan custom component I have created an updated version for Wavin Ahc 9000, utilizing the esphome modbus_controller and alot of esphome lambda (Thank you for not using standard modbus function Wavin!)
02-03-2022 - Genvexv2 - Inspired by Jopand's work on the nilan custom component I have created an updated version for Genvex, utilizing the esphome modbus_controller

### Installation

## V2 - Genvex
```yaml
packages:
  remote_package:
    url: https://github.com/heinekmadsen/esphome_components
    ref: main
    files: [components/genvexv2/optima250.yaml]
    refresh: 0s

uart:
  - id: uart_genvex
    rx_pin: GPIO16
    tx_pin: GPIO17
    parity: EVEN
    baud_rate: 19200
    stop_bits: 1
  
modbus:
    - id: genvex_modbus
      uart_id: uart_genvex
 
modbus_controller:
  id: genvex_modbus_controller
  address: 1
  modbus_id: genvex_modbus
  update_interval: 60s
  command_throttle: 10ms
```

## V2 - Wavin
To use the Wavin V2 you need to be running the ESPHome dev version in Home Assistant (https://esphome.io/guides/faq.html#how-do-i-use-the-latest-bleeding-edge-version)
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
```

Add this after your normal ESPhome config (wifi, ota, api, etc....) and uncomment the config lines for the active channels on your Wavin Ahc 9000
```yaml
# Uncomment the active channels on your Wavin Ahc 9000
packages:
  remote_package:
    url: https://github.com/heinekmadsen/esphome_components
    ref: main
    files: 
      - components/wavinahc9000v2/configs/channel_01.yaml 
      - components/wavinahc9000v2/configs/channel_02.yaml 
      - components/wavinahc9000v2/configs/channel_03.yaml 
      - components/wavinahc9000v2/configs/channel_04.yaml 
      - components/wavinahc9000v2/configs/channel_05.yaml 
      - components/wavinahc9000v2/configs/channel_06.yaml 
      - components/wavinahc9000v2/configs/channel_07.yaml 
      - components/wavinahc9000v2/configs/channel_08.yaml 
      - components/wavinahc9000v2/configs/channel_09.yaml 
      - components/wavinahc9000v2/configs/channel_10.yaml 
      - components/wavinahc9000v2/configs/channel_11.yaml 
      - components/wavinahc9000v2/configs/channel_12.yaml
      #- components/wavinahc9000v2/configs/channel_13.yaml 
      #- components/wavinahc9000v2/configs/channel_14.yaml
      #- components/wavinahc9000v2/configs/channel_15.yaml 
      #- components/wavinahc9000v2/configs/channel_16.yaml 
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




## Legacy
Follow the instructions below for the old genvex and wavin components
Add this repository to your ESPHome config yaml:

```yaml
external_components:
  - source: github://heinekmadsen/esphome_components
```

### Example yaml
```yaml
uart:
  rx_pin: GPIO16
  tx_pin: GPIO17
  parity: EVEN
  baud_rate: 19200
  id: uart_modbus
  stop_bits: 1

external_components:
  - source: github://heinekmadsen/esphome_components
    refresh: 0s
    
modbus:
  uart_id: uart_modbus
 
genvex:
  address: 1
  update_interval: 60s
 
sensor:
  - platform: genvex
    temp_t1:
      name: "genvex_temp_t1"
      id: genvex_temp_t1
    target_temp:
      name: "genvex_target_temp"
    speed_mode:
      name: "genvex_speed_mode"

climate:
  - platform: genvex
    name: genvex
    sensor: genvex_temp_t1
```

### Example yaml for PCB to handle both Wavin and Genvex from one ESP.
Reach out to dyrvigk for furter info on PCB
https://github.com/dyrvigk
```yaml
esphome:
  name: genvex-wavin
  platform: ESP32
  board: esp32dev

  # Enable logging
logger:
#  level: VERY_VERBOSE

# Enable Home Assistant API
api:

ota:
 
 
ethernet:
  type: LAN8720
  mdc_pin: GPIO23
  mdio_pin: GPIO18
  clk_mode: GPIO0_IN
  phy_addr: 1
  power_pin: GPIO14
  
status_led:
  pin: GPIO15

output:
  - platform: ledc
    pin: GPIO2
    id: gpio_2
    
    
# Example usage in a light
light:
  - platform: monochromatic
    output: gpio_2
    name: "network Light"
    
interval:
  - interval: 1s
    then:
      if:
        condition:
          api.connected 
        then:
          - output.turn_on: gpio_2
          - output.ledc.set_frequency:
              id: gpio_2
              frequency: "1000Hz"
          - output.set_level:
              id: gpio_2
              level: "10%"    
        else:
          - output.turn_off: gpio_2    

external_components:
  - source: github://heinekmadsen/esphome_components
    refresh: 0s

uart:
  - id: uart_1
    rx_pin: GPIO4
    tx_pin: GPIO5
    baud_rate: 38400
    stop_bits: 1
  - id: uart_2
    rx_pin: GPIO16
    tx_pin: GPIO17
    parity: EVEN
    baud_rate: 19200
    stop_bits: 1
    
modbus:
    - id: mod1
      uart_id: uart_1
    - id: mod2
      uart_id: uart_2
 

    
    
# Wavin
wavinAhc9000:
  modbus_id: mod1
  update_interval: 120s
  rw_pin: 33  

climate:
  - platform: wavinAhc9000
    channel: 1
    name: "Channel 1"
    visual:
      temperature_step: 0.5 °C
    battery_level:
      name: "Channel 1 Battery"
      state_class: "measurement"
    current_temp:
      name: "Channel 1 Temp"
      state_class: ""
  - platform: wavinAhc9000
    channel: 2
    name: "Channel 2"
    visual:
      temperature_step: 0.5 °C
    battery_level:
      name: "Channel 2 Battery"
    current_temp:
      name: "Channel 2 Temp"
      
  - platform: genvex
    name: genvex
    sensor: genvex_temp_t1

# Genvex
genvex:
  modbus_id: mod2
  address: 1
  update_interval: 60s
 
sensor:
  - platform: genvex
    temp_t1:
      name: "genvex_temp_t1"
      id: genvex_temp_t1
    temp_t3:
      name: "genvex_temp_t3"
    temp_t4:
      name: "genvex_temp_t4"
    temp_t7:
      name: "genvex_temp_t7"
    temp_t2_panel:
      name: "genvex_temp_t2_panel"
    measured_humidity:
      name: "genvex_measured_humidity"
    humidity_calculated_setpoint:
      name: "genvex_humidity_calculated_setpoint"
    alarm_bit:
      name: "genvex_alarm_bit"
    inlet_fan:
      name: "genvex_inlet_fan"
    extract_fan:
      name: "genvex_extract_fan"
    bypass:
      name: "genvex_bypass"
    watervalve:
      name: "genvex_watervalve"
    humidity_fan_control:
      name: "genvex_humidity_fan_control"
    bypass_on_off:
      name: "genvex_bypass_on_off"
    target_temp:
      name: "genvex_target_temp"
    speed_mode:
      name: "genvex_speed_mode"
    heat:
      name: "genvex_heat"
    timer:
      name: "genvex_timer"
      
binary_sensor:
  - platform: status
    name: "Genvex Wavin Controller Status"
```
