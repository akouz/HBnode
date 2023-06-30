A simple monitor works via FT200XD and a hyperterminal. Monitor uses Forth-like reverse polish notation, eg command goes last. Commands are not case sensitive.
The following commands implemented:
  * REV - prints sketch name and revision
  * SN - writes or reads serial number
  * NAME - writes or reads device name
  * LOCATION - writes or reads device location

Serial number and text strings are stored in EEPROM.

[PuTTY session](https://github.com/akouz/HBnode/blob/main/AVR64DD32/Sketches/test_monitor/PuTTY_session.png)

