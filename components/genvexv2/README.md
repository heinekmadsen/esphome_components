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

You will need a ESP32 and a modbus level conversation print.
####### Insert links #########

Start setting up you ESP32 using ESPHOME in Home Assistant

Note: Be aware when you connect the ESP32 to Genvex Power supply on <b><i>pin</i></b> and the USB device at the samt time :) 
If you do so you connect the 5V from USB and Genvex 10V output and no one will win you will properly just blow a fuse. :D 