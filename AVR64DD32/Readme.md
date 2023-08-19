# HBnode AVR64DD32

Small modules based on AVR64DD32 are supposed to be used in [HBus](https://github.com/akouz/HBus) project instead of Arduino Pro Mini and Arduino Nano. They have the same size and similar pin-out, but differs by the following:

-- Program memory 64K instead of 32K.

-- Clock frequency 24 MHz instead of 16 MHz

-- Custom made bootloader dedicated for operation in HBus environment. HBus device based on HBnode can remotely upload a new firmare via HBus.

-- User monitor via I2C link and FT200XD test jig.
