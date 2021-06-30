# Wavin AHC 9000 floor heating modbus interface

A configured uart component and a modbus component is required.

Example minimal required config:
```yaml
uart:
  rx_pin: GPIO16
  tx_pin: GPIO17
  baud_rate: 38400
  id: uart_modbus
  stop_bits: 1
  
modbus:
  uart_id: uart_modbus
 
wavinAhc9000:
  update_interval: 60s
  rw_pin: 25
  
climate:
  - platform: wavinAhc9000
    channel: 1
    name: "Floor room 1"
    visual:
      temperature_step: 0.5 °C
  - platform: wavinAhc9000
    channel: 2
    name: "Floor room 2"
    visual:
      temperature_step: 0.5 °C
    battery_level:
      name: "Battery room 2"
    current_temp:
      name: "Current temp room 2"
  ....
  ....
  - platform: wavinAhc9000
    channel: 16
    name: "Floor room 16"
    visual:
      temperature_step: 0.5 °C
```

Optional:

To get a battery level sensor of the room sensors add battery_level:
Example with battery level sensor:
```yaml
  - platform: wavinAhc9000
    channel: 1
    name: "Floor room 1"
    visual:
      temperature_step: 0.5 °C
    battery_level:
      name: "Battery room 1"
```

To get a current temperature sensor of the room sensors add current_temp:
Example current temperature sensor:
```yaml
  - platform: wavinAhc9000
    channel: 2
    name: "Floor room 2"
    visual:
      temperature_step: 0.5 °C
    current_temp:
      name: "Current temp room 2"
```


Example with both battery level and current temperature sensor:
```yaml
  - platform: wavinAhc9000
    channel: 3
    name: "Floor room 3"
    visual:
      temperature_step: 0.5 °C
    battery_level:
      name: "Battery room 3"
    current_temp:
      name: "Current temp room 3"
```  



TO-DO:
1. ...

Kudos:
A big thanks to ssieb from the ESPHome Discord community for assisting i making this happen! https://github.com/ssieb
