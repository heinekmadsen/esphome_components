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

