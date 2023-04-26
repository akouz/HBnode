# HEXloader

It is a PC application for Windows. Compiled with Lazarus 2.0.4, uses serial port coimponent CPortLaz. It communicated to programmer (Arduino UNO) running HEXloader.ino sketch.

User selects a hex file compiled for HBnode; HEXloader sends it to the programmer; programmer writes hex file into the I2C EEPROM on HBnode board; after reset HBnode bootloader copies EEPROM into program memory.