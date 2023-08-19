HBnode Nano board is similar to HBnode Pro Mini. 

![Nano](https://github.com/akouz/HBnode/blob/main/AVR64DD32/Hardware/Pro_mini/HBnode_nano_rev_1_0.jpg)

Connector P4 for UPDI programmer/debugger. To program bootloader I used MPLAB X rev 6.05 and PicKit4.

Connector P3 for a I2C programmer or for external monitor based on FT200XD. I2C programmer writes application code into serial EEPROM U1. After reset bootloader copies EEPROM into program memory. Arduino UNO can be used as I2C programmer.

![Programmer connected](https://github.com/akouz/HBnode/blob/main/AVR64DD32/Programmer/Programmer_connected.jpg)

