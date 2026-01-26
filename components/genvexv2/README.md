### Installation

## V2 - Genvex
```yaml
packages:
  remote_package:
    url: https://github.com/heinekmadsen/esphome_components
    ref: "v1.0.1"
    #ref: develop
    files: [components/genvexv2/optima250.yaml]
    #files: [components/genvexv2/optima250_develop.yaml]
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