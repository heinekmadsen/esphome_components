## Genvexv2

Updated version of Genvex modbus controller for 

Supported Genvex Control units

|Controller     | Support   | Tested |
|---------------|:---------:|:------:|
|Optima 250     | x         | x      |
|Optima 251     |           |        |
|Optima 260     | x         | x      |
|Optima 270     | x         | x      |
|Optima 301     |           |        |
|Optima 310     |           |        |
|Optima 311-312 |           |        |


### Setting up ESP32
Guide on how to setup you ESP32 to Genvex unit via modbus

You will need a ESP32 and a board to convert from UART on RS485 used to communicate wiht Genvex over modbus
- [ESP-32 Development Board](https://www.aliexpress.com/item/32834130422.html?spm=a2g0o.order_list.order_list_main.5.21ef1802NUOVTF)
- [TTL to RS485 Converter 3.3V/5.0V](https://www.aliexpress.com/item/32846149743.html?spm=a2g0o.order_list.order_list_main.28.21ef1802NUOVTF)

<b>Connection diagram</b>
Connect the components as shown on the diagram below.

![Diagram of Genvex and ESP32 connection](doc/diagram.png)

**Note:**
Don't connect the ESP32 to Genvex 10V Power supply on <em>L16.1</em> and a USB device at the samt time!!!
If you do so Genvex 10V and your USB device will be connected and properly just blow a fuse or have a USB port not working :|
