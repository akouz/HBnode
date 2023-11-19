Sketch rev 0.19 more or less matches current rev 1.5 of HBus. It can upload sketches via HBus using NodeTest rev 1.14, but it is not finished yet. 

The latest changes implement HBus collision avoidance, CSMA/CA, based on CCL and EVSYS. The TxD output featured CSMA/CA is routed to port PC3, pin 9. Proper operation of that feature requires a hardware link between RX1 (port PC1, pin 7) and D2 (port PD6, pin 16).

Arduino IDE settings as shown:

![settings](https://github.com/akouz/HBnode/blob/main/AVR64DD32/Sketches/HBnode_DD32/Settings.jpg)
