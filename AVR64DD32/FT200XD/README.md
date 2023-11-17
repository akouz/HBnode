Test jig based on [FT200XD](https://ftdichip.com/products/ft200xd/) chip. Jig connected to HBnode 5-pin header P4 via 4-pin jack J2. P4 pin 1 not used; J2 pin 1 connects to P4 pin 2.

![Jig_connected](https://github.com/akouz/HBnode/blob/main/AVR64DD32/FT200XD/FT200XD_jig_connected.jpg)

Use Windows Device Manager to configure FT200XD, it should operate as a virtual com port (VCP). Then use any hyperterminal to talk to HBnode monitor; [PuTTY](https://www.putty.org/) is recommended. 

After power-up HBus sketches usually sends to FT200XD a welcome screen, as shown in the following example:
![power_up](https://github.com/akouz/HBnode/blob/main/AVR64DD32/FT200XD/Power_up.png)

Using FT200XD and HBus sketch it is possible to to do the following:
* REV -- read hardware and sketch revisions
* SN -- read serial number
* xxxx SN -- assign a serial number xxxx to a blank board
* NODEID -- read HBus node ID 
* xxxx NODEID -- assign node ID xxxx to a blank board
* NAME -- read node name (optional)
* TXT NAME -- assign node name TXT
* LOCATION -- read node location  (optional)
* TXT LOCATION -- assign node location TXT
* DESCR -- read node description  (optional)
* TXT DESCR -- assign node description TXT
* addr len EERD -- read len bytes from I2C EEPROM starting at address addr
* addr len EECLR -- clear len bytes in the I2C EEPROM starting at address addr
* TOPIC -- read topic names and topic IDs; if topic value is valid it will be displayed too
