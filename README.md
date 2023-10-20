# HBnode
Microcontroller boards for [HBus](https://github.com/akouz/HBus) based on [AVR64DD32](https://www.microchip.com/en-us/product/avr64dd32). Form factor is the same as Arduino Pro Mini and Arduino Nano, while pinout made as close as possible. Firmware update made using on-board I2C EEPROM; custom bootloader occupies 1K bytes. 

[Programmer HEXloader.exe](https://github.com/akouz/HBnode/tree/main/AVR64DD32/Programmer/Software) can transfer compiled hex files from PC to EEPROM via Arduino UNO running [sketch HEXloader.ino](https://github.com/akouz/HBnode/tree/main/AVR64DD32/Programmer/Sketch). When a board used in HBus network, firmware can be updated remotely via HBus.

![Pro Mini](https://github.com/akouz/HBnode/blob/main/AVR64DD32/Hardware/Pro_mini/HBnode-rev-1-0.jpg)

![Nano](https://github.com/akouz/HBnode/blob/main/AVR64DD32/Hardware/Nano/HBnode_nano_rev_1_0.jpg)


# How to program HBnode board:
1. Burn bootloader. 
   Plug PicKit4 to 4-pin connector, apply power to the board and burn [HBnode bootloader](https://github.com/akouz/HBnode/blob/main/AVR64DD32/Bootloader/HBnode_bootloader.hex) using MPLAB.X IDE or MPLAB.X IPE.

2. Load HBus sketch.
   Connect HBnode to the [programmer](https://github.com/akouz/HBnode/tree/main/AVR64DD32/Programmer) and transfer selected sketch to the HBnode using [HEXLoader](https://github.com/akouz/HBnode/tree/main/AVR64DD32/Programmer/Software)
   
