
# HBnode demo application

It is a simple demo application for HBnode, it flashes on-board red and green LEDs. It can be downloaded to HBnode using HEXloader.

Demo application is written in MPLAB-X using AVR-GCC compiler v7.3.0. In "Project Properties" select "Avr GCC/avr-ld/Memory Settings" and in the row "FLASH segment" enter ".text=0x200". It tells linker to place application after 1K because the lower 1K of program memory occupied by HBnode bootloader.

Later on Arduino with DxCore might be used to write applications for HBnode. So far all my attempts to comppile a working application using DxCore were not successfull.
