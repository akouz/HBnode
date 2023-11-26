Sketch rev 1.0  matches HBus rev 1.5. It can upload sketches via HBus using NodeTest rev 1.14. 

Sketch implements HBus collision avoidance, CSMA/CA, based on CCL and EVSYS. The TxD output featured CSMA/CA is routed to port PC3, pin 9. Proper operation of that feature requires a hardware link between RX1 (port PC1, pin 7) and D2 (port PD6, pin 16).

Arduino IDE settings as shown:

![settings](https://github.com/akouz/HBnode/blob/main/AVR64DD32/Sketches/HBnode_DD32/Arduino_settings.png)
