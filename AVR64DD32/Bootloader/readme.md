# HBnode bootloader

It is compiled in MPLAB-X, compiler AVR-GCC v7.3.0. To fit bootloader into 1K compiler optimisation -s must be used.

Unlike other Arduino bootloaders, this bootloader copies content of the on-board EEPROM into AVR flash memory. EEPROM is supposed to be updated by application via HBus in small chunks.
