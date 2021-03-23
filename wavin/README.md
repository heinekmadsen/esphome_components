# Genvex air circulation modbus interface

A configured uart component is required.

Example:
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
  
sensor:
  - platform: genvex
    address: 1
    update_interval: 60s
    id: my_genvex
    temp_t1:
      id: temp_t1
      name: "temp_t1"
    temp_t3:
      id: temp_t3
      name: "temp_t3"
    temp_t4:
      id: temp_t4
      name: "temp_t4"
    temp_t7:
      id: temp_t7
      name: "temp_t7"
    temp_t2_panel:
      id: temp_t2_panel
      name: "temp_t2_panel"
    measured_humidity:
      id: measured_humidity
      name: "measured_humidity"
    humidity_calculated_setpoint:
      id: humidity_calculated_setpoint
      name: "genvex_humidity_calculated_setpoint"
    alarm_bit:
      id: alarm_bit
      name: "alarm_bit"
    inlet_fan:
      id: inlet_fan
      name: "inlet_fan"
    extract_fan:
      id: extract_fan
      name: "extract_fan"
    bypass:
      id: bypass
      name: "bypass"
    watervalve:
      id: watervalve
      name: "watervalve"
    humidity_fan_control:
      id: humidity_fan_control
      name: "humidity_fan_control"
    bypass_on_off:
      id: bypass_on_off
      name: "bypass_on_off"
    target_temp:
      id: target_temp
      name: "target_temp"
    speed_mode:
      id: speed_mode
      name: "speed_mode"
    heat:
      id: heat
      name: "heat"
    timer:
      id: timer
      name: "timer"
      
binary_sensor:
  - platform: template
    name: "bypass_on_off"
    lambda: |-
      if (id(bypass_on_off).state == 1) {
        // Open
        return true;
      } else {
        return false;
      }
```

All sensors are optional.  The available sensors are:
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
- `target_temp`
- `speed_mode`      
- `heat`
- `timer`


TO-DO:
1. Add a climate platform
  a. Be able to set fan speed
  b. Be able to set target temperature
2. Handle changes to fan speed and target temperature and write new values via modbus
