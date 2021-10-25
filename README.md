## ESPHome custom components

My creation of custom components fro ESP home.

### Installation
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

# Temporary fix for modbus component error in ESPHome 2021.10.0
# https://github.com/esphome/issues/issues/2630
# Might not be nessesary in future builds, but for now it is
  - source:
      type: git
      url: https://github.com/martgras/esphome
      ref: modbus-fix
    components: [ modbus ]

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
  
# Temporary fix for modbus component error in ESPHome 2021.10.0
# https://github.com/esphome/issues/issues/2630
# Might not be nessesary in future builds, but for now it is
  - source:
      type: git
      url: https://github.com/martgras/esphome
      ref: modbus-fix
    components: [ modbus ]


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
