/*  Project:  HEXloader
 *  Author:   A.Kouznetsov
 *  File:     RxTx.cpp
 */

 //##############################################################################
 // Inc
 //##############################################################################

#include <Firmata.h>
#include "RxTx.h"

 //##############################################################################
 // Class RxTx
 //##############################################################################

 //================================
 // Constructor
 //================================
 RxTx::RxTx(void)
 {
   this->rxlen = 0;
 }

 //================================
 // Add received char
 //================================
uchar  RxTx::add_rx_char(char c)
{
   if (c > ' ') // printable ASCII char
   {
      if (this->rxlen < RX_BUF_LEN-1)
      {
        if ((c >= 'a') && (c <= 'z'))
        {
          c = c - 'a' + 'A';  // make upper case
        }
        this->rxbuf[this->rxlen++] = c;
        return OK;
      }
      else
      {
        Serial.print('|'); // overflow
        Serial.print(char(0));
        return ERR;
      }
   }
   else if (c < ' ') // string terminator
   {
      return DONE;
   }
   return ERR;
}
//================================
// Reply revision
//================================
uchar RxTx::reply_rev(void)
{
  Serial.print("Rev 1.4");
  Serial.print(char(0));
  return OK;
}
//================================
// Char to nibble
//================================
uchar char_to_nibble(char c)
{
  if ((c >= '0') && (c <= '9'))
  {
    return c - '0';
  }
  else if ((c >= 'A') && (c <= 'F'))
  {
     return c - 'A' + 10;
  }
  else
    return 0xFF;
}

//================================
// Convert HEX into binary
//================================
uchar RxTx::HEX_to_bin(binbuf_t*  binbuf)
{
  uchar len, nib, val=0, cs;
  char c;
  if (binbuf)
  {
    if (this->rxlen & 1) // rx buffer must have odd number of chars
    {
      binbuf->len = 0;
      len = 0;
      cs = 0; // cheksum
      for(uint i=1; i<this->rxlen; i++)
      {
        c = this->rxbuf[i];
        nib = char_to_nibble(c);
        if (nib > 0x10) // illegal char
        {
          this->rxlen = 0;
          Serial.print('x'); // ask to re-send
          Serial.print(char(0));
          return ERR;
        }
        if (i & 1)
        {
          val = nib << 4; // MSB nibble
        }
        else
        {
          val |= nib;    // add LSB nibble
          binbuf->buf[len++] = val;
          cs += val;  // checksum
        }
      } // for
      if ((cs == 0) && (len > 5)) // chectsum matched, has at least one byte of data
      {
        if (binbuf->buf[0] == len-5)
        {
          binbuf->len = binbuf->buf[0];       // data length
          binbuf->addr_H = binbuf->buf[1];    // address MSB
          binbuf->addr_L = binbuf->buf[2];    // address LSB
          binbuf->rectype = binbuf->buf[3];   // type
          memcpy(binbuf->buf, binbuf->buf+4, binbuf->len);  // data
          this->rxlen = 0;
          if ((binbuf->rectype == 0) || (binbuf->rectype == 1))  // only those two types are accepted
          {
            return DONE;
          }
        } // len matches
      } // checksum and len
    } // odd
    // any error
    Serial.print('x'); // ask to re-send
    Serial.print(char(0));
    this->rxlen = 0;
    return INVALID;
  } // if binbuf
  return NOT_READY;
}

/* EOF */
