/*  Project:  HEXloader
 *  Author:   A.Kouznetsov
 *  File:     HEXloader.ino
 */

//##############################################################################
// Inc
//##############################################################################

#include <Firmata.h>
#include <Wire.h>
#include "description.h"
#include "RxTx.h"
#include "EEPROM.h"

//##############################################################################
// Def
//##############################################################################

#define MAX_BBUF  4

//##############################################################################
// Var
//##############################################################################

RxTx rxtx;
HB_eeprom ee;
binbuf_t  bbuf;

//##############################################################################
// Func
//##############################################################################

//================================
// Process received buffer
//================================
uchar process_rxbuf(void)
{
  uchar res = ERR;
  char c;
  if (rxtx.rxlen)
  {
    switch(rxtx.rxbuf[0])
    {
      // -------------------------
      case 'H':
      digitalWrite(RST_PIN, HIGH);
      Serial.print('H');
      Serial.print(char(0));
      break;
      // -------------------------
      case 'L':
      digitalWrite(RST_PIN, LOW);
      ee.crc = 0xFFFF;   // be ready to receive hex code
      rxtx.codelen = 0;
      Serial.print('L');
      Serial.print(char(0));
      break;
      // -------------------------
      case 'Q':
        c = (ee.connected())? '-' : '+';
        Serial.print(c);
        Serial.print(char(0));
        res = OK;
        break;
      // -------------------------
      case 'R':
        res = rxtx.reply_rev();
        break;
      // -------------------------
      case ':':
        res = rxtx.HEX_to_bin(&bbuf);
        if (res == DONE)
        {
          if (ee.connected())   // if Ack high
          {
            Serial.print('-');  // EEPROM disconnected
          }
          else
          {
            res = ee.write_buf(&bbuf);
            switch(res)
            {
              case 0:
                Serial.print('.'); break; // success
              case 1:
                Serial.print('-'); break; // EEPROM disconnected
              default:
                Serial.print('x'); break; // other errors
            }
          }
          Serial.print(char(0));
        } // else 'x' sent
        break;
        // -------------------------
        case '!':
          res = rxtx.HEX_to_bin(&bbuf);
          if (res == DONE)
          {
            if (ee.connected())   // if Ack high
            {
              Serial.print('-');  // EEPROM disconnected
            }
            else
            {
              res = ee.write_descr(&bbuf);
              switch(res)
              {
                case 0:
                  Serial.print('!'); break; // success
                case 1:
                  Serial.print('-'); break; // EEPROM disconnected
                case INVALID:
                  Serial.print('C'); break; // CRC mismatch
                default:
                  Serial.print('x'); break; // other errors
              }
            }
            Serial.print(char(0));
          } // else 'x' sent
          break;
      // -------------------------
      default:
        break;
    }
    if (res != NOT_READY)
    {
      rxtx.rxlen = 0;   // clear rx buffer
    }
  }
  return res;
}
//#####################################################
// Setup
//#####################################################
void setup()
{
  Serial.begin(115200);             // serial baud rate 115.2 kbps
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);       // by default appy reset to HBus node
  Serial.println("");
  Serial.println("=================================");
  Serial.println("=== HBus HEXloader programmer ===");
  Serial.println("=================================");
  rxtx.reply_rev();      		// promptly print revisions
  digitalWrite(RST_PIN, HIGH); 		// release target 
}
//#####################################################
// Run
//#####################################################
void loop()
{
  // ----------------------------
  // receive chars
  // ----------------------------
  while (Serial.available())  // If anything comes in Serial (USB),
  {
    if (rxtx.add_rx_char(Serial.read()) == DONE) // string ready
    {
      process_rxbuf();    // execute received command
      break;
    }
  }
}

/* EOF */
