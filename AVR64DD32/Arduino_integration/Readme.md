[DxCore](https://github.com/SpenceKonde/DxCore) project provides integration of AVR Dx microcontrollers with Arduino.  To use HBnode with Arduino, do the following:

  * Install DxCore to Arduino. Start Arduino IDE, in Tools/Board/Boards Manager find DxCore and install it. Current DxCore version is 1.5.6.
  * Open folder C:\Users\User\AppData\Local\Arduino15\packages\DxCore\hardware\megaavr\1.5.6
  * Replace file "boards.txt" with the file provided here
Use a demo sketch "Blink.ino" to test Arduino integration:
  * Copy sketch "Blink.ino" into a folder "Blink"
  * Start Arduino IDE
  * Select "Blink.ino" sketch
  * Select Tools/Board/DxCore/HBnode board
  * Compile sketch
  * Select Sketch/Export compiled Binary
  * Start HEXloader and navigate to the folder where "Blink.ino" sketch was placed
  * Select compiled hex file and send it to targed board EEPROM
