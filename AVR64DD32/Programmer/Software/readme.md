# HEXloader

It is a PC application for Windows. Compiled with Lazarus 2.0.4, uses serial port component CPortLaz. It communicated to programmer (Arduino UNO) running HEXloader.ino sketch.

User selects a hex file compiled for HBnode; HEXloader sends it to the programmer; programmer writes hex file into the I2C EEPROM on HBnode board; after reset HBnode bootloader copies EEPROM into program memory.

![HEXloader](https://github.com/akouz/HBnode/blob/main/AVR64DD32/Programmer/Software/HEXloader.png)
