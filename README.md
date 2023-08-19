# HBnode
Microcontroller boards for [HBus](https://github.com/akouz/HBus) based on [AVR64DD32](https://www.microchip.com/en-us/product/avr64dd32). Form factor is the same as Arduino Pro Mini and Arduino Nano, while pinout made as close as possible. Firmware update made using on-board I2C EEPROM; custom bootloader occupies 1K bytes. 

Programmer HEXloader.exe can transfer compiled hex files from PC to EEPROM via Arduino UNO running sketch HEXloader.ino. When a board used in HBus network, firmware can be updated remotely via HBus.

![Pro Mini](https://github.com/akouz/HBnode/blob/main/AVR64DD32/Hardware/Pro_mini/HBnode-rev-1-0.jpg)

![Nano](https://github.com/akouz/HBnode/blob/main/AVR64DD32/Hardware/Nano/HBnode_nano_rev_1_0.jpg)
