/*  Project:  HEXloader
 *  Author:   A.Kouznetsov
 *  File:     EEPROM.h
 */
#ifndef __EEPROM_H
#define __EEPROM_H

//##############################################################################
// Inc
//##############################################################################

#include "description.h"

//##############################################################################
// Var
//##############################################################################

// HBus EEPROM
class HB_eeprom{
  public:
          HB_eeprom(void);
    uchar connected(void);
    uchar write_buf(binbuf_t* buf);
    uchar write_descr(binbuf_t* buf);
    uchar ack;                // 0 if EEPROM replied Ack
    uint crc;
  private:
};

#endif /* __EEPROM_H */
