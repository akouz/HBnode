# Sketch HEXloader

Sketch HEXloader for Arduino UNO used to download compiled files (sketches) into HBnode using
[HEXloader](https://github.com/akouz/HBnode/tree/main/AVR64DD32/Programmer/Software) application.

## General

HBnode employs AVR64DD32 and a serial EEPROM 24C512. HEXloader sketch writes code
received from the serial port into EEPROM while keeping AVR64DD32 in reset.
When application code copied into EEPROM, HEXloader  sketch releases AVR64DD32. The
HBnode bootloader in AVR64DD32 copies application code from EEPROM into flash
an passed control to the application code.

## Exchange with PC

Hardware serial port is used, baud rate 115200. PC sends ASCII strings, length
2...288 bytes. HEXloader  sketch reads strings and replies with ASCII strings too. In most
cases reply string consists of a single char and a terminator. String terminator
is any char with value less than 0x20. Blank char (' ', code 0x20) is ignored.
Input strings are not case sensitive.

The following commands from PC to HEXloader sketch are defined:
```
'R' - Read revision, HEXloader  sketch should reply "R" followed by its sketch revision.

'Q' - Query HBnode. HEXloader  sketch tries to read HBnode EEPROM. It replies '-' if EEPROM not
      detected, or '+' if EEPROM is present.

':' - Write a block of data in Intel HEX format to EEPROM. This message has the
      following structure:
      +-----+-----+------+------+------+---------+
      | ':' | Len | Addr | Type | Data | Cheksum |
      +-----+-----+------+------+------+---------+
      Example:
      :1001100060F00E94D70048E060E170E090E080E08D
      -- 10   - Len - data length 0x10 bytes
      -- 0110 - Addr - address 0x0110
      -- 00   - Type - data record
      -- 60 F0 0E 94 D7 00 48 E0  60 E1 70 E0 90 E0 80 E0  - Data
      -- 8D   - Checksum - two's complement of checksum LSB byte
      HEXloader  sketch should  reply '.' if writing succeeded, or 'x' in case
      of any error. If EEPROM disconnected it replies '-'

'!' - Write a decriptor for bootloader. This message has Intel HEX format.
      Its structure follows:
      +-----+------+------+------+---------+-----+---------+
      | '!' |  Len | Addr | Type | CodeLen | CRC | Cheksum |
      +-----+------+------+------+---------+-----+---------+
      Where:
      -- CodeLen - code length in EEPROM (starting from addr 0x0400)
      -- CRC - cyclic redundancy check for the valid EEPROM code
      If CRC matches then the following descriptor is written into EEPROM
      starting from address 0x0010:
      +------+------+------+------+-----------+-----------+-------+-------+
      | 0x55 | 0xAA | 0xC3 | 0x3C | CodeLen_H | CodeLen_L | CRC_H | CRC_L |
      +------+------+------+------+-----------+-----------+-------+-------+
      HEXloader  sketch should  reply '!' if writing succeeded, or 'x' in case
      of any error. If EEPROM disconnected it replies '-'

'L' - set active low level to HBnode reset line, reply is 'L'. Also reset crc
      and codelen counter.

'H' - set passive high level to HBnode reset line, reply is 'H'
```

## Writing to EEPROM

Write to EEPROM requires 5..10 ms to complete. At baud rate of 115200 bps, it
takes 86.8 us to transfer one byte, or 3.47 ms to transfer one chunk of 16 bytes
in HEX format.

EEPROM 24LC512 has internal data buffer size of 128 bytes. EEPROM exchange baud
rate is 200 kbps.
