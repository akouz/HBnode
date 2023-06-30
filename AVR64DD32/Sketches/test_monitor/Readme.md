A simple monitor works via FT200XD and a hyperterminal. It uses Fort-like reverse polish notation, eg command goes last. Commands are not case sensitive.
The following commands implemented:
  * REV - prints sketch name and revision
  * SN - sets or reads serial number
  * NAME - sets or reads device name
  * LOCATION - sets oe reads device location

Serial number and text strings are stored in EEPROM.

![PuTTY session](https://github.com/akouz/HBnode/blob/main/AVR64DD32/Sketches/test_monitor/PuTTY_session.png)
