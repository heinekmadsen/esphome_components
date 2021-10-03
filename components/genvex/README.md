# Genvex air circulation modbus interface

A configured uart component and a modbus component is required.

Example minimal required config:
```yaml
uart:
  rx_pin: GPIO16
  tx_pin: GPIO17
  parity: EVEN
  baud_rate: 19200
  id: uart_modbus
  stop_bits: 1
  
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

List of optional sensors:
- `temp_t1`
- `temp_t3`
- `temp_t4`
- `temp_t7`
- `temp_t2_panel`
- `measured_humidity`
- `humidity_calculated_setpoint`
- `alarm_bit`
- `inlet_fan`
- `extract_fan`
- `bypass`
- `watervalve`
- `humidity_fan_control`
- `bypass_on_off`      
- `heat`
- `timer`


TO-DO:
1. .....

Kudos:
A big thanks to ssieb from the ESPHome Discord community for assisting i making this happen! https://github.com/ssieb
