/*  Project:  HEXloader
 *  Author:   A.Kouznetsov
 *  File:     EEPROM.cpp
 */
 //##############################################################################
 // Inc
 //##############################################################################

#include <Firmata.h>
#include <Wire.h>
#include "EEPROM.h"

//##############################################################################
// Func
//##############################################################################

//================================
// Constructor
//================================
HB_eeprom::HB_eeprom(void)
{
  Wire.begin();
  Wire.setClock(400000);              // I2C clock rate 400 kHz
  Wire.setWireTimeout(15000, true);   // time-out 15 ms
  this->ack = 1;
}

//================================
// For 10 ms check if EEPROM responds
//================================
uchar HB_eeprom::connected(void)
{
  ulong ts = millis();
  while ((millis() - ts) < 12)
  {
    Wire.beginTransmission(EE_ADDR);
    this->ack = Wire.endTransmission(true); // 0 = success
    if (this->ack == 0)
      break;
    else
      delay(1);
  }
  return this->ack;
}
// ===================================================
//  Caculate CRC16-CCITT
// ===================================================
/*
 * Name  : CRC-16 CCITT
 * Poly  : 0x1021    x^16 + x^12 + x^5 + 1
 * Init  : 0xFFFF
 * Revert: false
 * XorOut: 0x0000
 * Check : 0x3B0A ("123456789" hex)
 * MaxLen: 32k
 */
uint crc16(uchar* buf, uint len, uint crc)
{
    uint i;
    while (len--)
    {
        crc ^= (*buf++ << 8);
        for (i=0; i<8; i++)
            crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

//================================
// Write buffer to EEPROM
//================================
uchar HB_eeprom::write_buf(binbuf_t* buf)
{
  if ((buf) && (buf->len) && (this->ack == 0))
  {
    digitalWrite(LED_BUILTIN, HIGH);
    Wire.beginTransmission(EE_ADDR);
    Wire.write(buf->addr_H);
    Wire.write(buf->addr_L);
    for (uchar i=0; i<buf->len; i++)
    {
      Wire.write(buf->buf[i]);
    }
    this->ack = Wire.endTransmission(true);           // 0 = success
    if (this->ack == 0)
    {
      this->crc = crc16(buf->buf, buf->len, this->crc); // update CRC
    }
    digitalWrite(LED_BUILTIN, LOW);
    return this->ack;
  }
  return ERR;
}
//================================
// Write descriptir to EEPROM
//================================
uchar HB_eeprom::write_descr(binbuf_t* buf)
{
  uint dcrc;
  if ((buf) && (buf->len) && (this->ack == 0))
  {
    dcrc = 0x100*buf->buf[2] + buf->buf[3];
    if (dcrc == this->crc) // if CRC matches
    {
      digitalWrite(LED_BUILTIN, HIGH);
      Wire.beginTransmission(EE_ADDR);
      Wire.write(0);
      Wire.write(0x10);   // addr
      Wire.write(0x55);   // pattern
      Wire.write(0xAA);
      Wire.write(0xC3);
      Wire.write(0x3C);
      for (uchar i=0; i<4; i++)
      {
        Wire.write(buf->buf[i]);  // CodeLen and CRC
      }
      this->ack = Wire.endTransmission(true);           // 0 = success
      digitalWrite(LED_BUILTIN, LOW);
      return this->ack;
    }
    return INVALID;
  }
  return ERR;
}

/* EOF */
