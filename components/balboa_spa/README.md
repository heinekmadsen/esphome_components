## ESPHome Balboa SPA ##

### Example yaml ###
```yaml
esphome:
  name: "wemos_spa"
  friendly_name: "wemos spa"

esp8266:
  board: d1_mini_pro

# Enable logging
logger:

external_components:
  - source:
      type: git
      url: https://github.com/heinekmadsen/esphome_components
      ref: "main"
      #ref: "develop"
      #ref: "v1.0.1"
    components: [ balboa_spa ]
    refresh: 0s

# Enable Home Assistant API
api:
  services:
    - service: set_spa_time
      variables:
        hour: int
        minute: int
      then:
        - lambda: 'id(balboa_spa_id).on_set_time(hour, minute);'
    - service: set_heater_enabled
      variables:
        enabled: bool
      then:
        - lambda: 'id(balboa_spa_id).set_heater_enabled(enabled);'
 
# Allow Over-The-Air updates
ota:
- platform: esphome

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

  # Enable fallback hotspot (captive portal) in case wifi connection fails.
  ap:
    ssid: "Esphome-Web-9A464C"
    password: "PPw31wpLkDN3"

captive_portal:

web_server:
  version: 3

time:
- platform: homeassistant
  id: my_time
  timezone: "Europe/Copenhagen"

uart:
  id: spa_uart_bus
  tx_pin: GPIO1
  rx_pin: GPIO3
  data_bits: 8
  parity: NONE
  stop_bits: 1
  baud_rate: 115200
  rx_buffer_size: 128

balboa_spa:
  id: balboa_spa_id
  uart_id: spa_uart_bus
  fault_text_sensor:
    name: "Fault Message"
  time_text_sensor:
    name: "Spa Time"
  heating_status_text_sensor:
    name: "Heating Status"

climate:
  - platform: balboa_spa
    name: "Spa"
    balboa_spa_id: balboa_spa_id    

button:
  - platform: balboa_spa
    name: "Lights Toggle"
    button_datapoint: 0
    id: btn_spa_lights
    internal: true
  - platform: balboa_spa
    name: "Jet1 Toggle"
    button_datapoint: 1
    icon: "mdi:fan"
    id: btn_spa_jet1
    internal: true
  - platform: balboa_spa
    name: "Jet2 Toggle"
    button_datapoint: 2
    icon: "mdi:fan"   
    internal: true

binary_sensor:
  - platform: balboa_spa
    name: "Lights"
    binary_sensor_datapoint: 20
    id: sens_lights
  - platform: balboa_spa
    name: "Circulation Pump"
    binary_sensor_datapoint: 22
  - platform: balboa_spa
    name: "Rest"
    binary_sensor_datapoint: 23
  - platform: balboa_spa
    name: "Jet1"
    binary_sensor_datapoint: 24
    id: sens_jet1
  - platform: balboa_spa
    name: "Jet2"
    binary_sensor_datapoint: 25
  - platform: balboa_spa
    name: "Blower"
    binary_sensor_datapoint: 26     

switch:
  - platform: restart
    name: restart
  - platform: template
    name: Spa Jet
    lambda: |-
      if (id(sens_jet1).state) {
        return true;
      } else {
        return false;
      }
    turn_on_action: 
      then:
        - button.press: btn_spa_jet1
    turn_off_action: 
      then:
        - button.press: btn_spa_jet1
  - platform: template
    name: Spa Light
    lambda: |-
      if (id(sens_lights).state) {
        return true;
      } else {
        return false;
      }
    turn_on_action: 
      then:
        - button.press: btn_spa_lights
    turn_off_action: 
      then:
        - button.press: btn_spa_lights
    
text_sensor:
  - platform: wifi_info
    ip_address:
      name: "IP Address"
    ssid:
      name: "Wi-Fi SSID"
  - platform: version
    name: "ESPHome Version"
    hide_timestamp: true

```