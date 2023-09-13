/*
 * File     HBcommon.h
 
 * (c) 2023 Alex Kouznetsov,  https://github.com/akouz/hbus
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __HB_COMMON_H
#define __HB_COMMON_H

//##############################################################################
// Inc
//##############################################################################

#include <Arduino.h>
#include <stdio.h>
#include <ArduinoJson.h>    // https://github.com/bblanchon/ArduinoJson   rev 5.13.4
#include <coos.h>           // https://github.com/akouz/a_coos  rev 1.6

#include "HBconfig.h"
#include "i2c_bitbang.h"

//##############################################################################
// Def
//##############################################################################

#define DBG_PRINT(x)      i2cbb.print(x)
#define DBG_PRINTLN(x)    i2cbb.println(x)
#define BUF_PRINTLN(x,y)  i2cbb.println(x,y)


#define MAX_BUF       0x90
#define MAX_SSTR      0x20
#define MAX_LSTR      0x40

#ifndef NULL  
    #define NULL 0  
#endif  

#define IOREG   0xD8    // to unlock protected I/O regs
  
#define BIT0    0x01  
#define BIT1    0x02
#define BIT2    0x04  
#define BIT3    0x08  
#define BIT4    0x10  
#define BIT5    0x20  
#define BIT6    0x40  
#define BIT7    0x80  
  
#define NOP()   asm("nop") 

// ==================================
// enum
// ==================================
enum{

    // misc
    DF_STATUS       = 1,    // use JSON in STATUS command
    TIME_TOLERANCE  = 60,  // +/- 1 min

    // byte-stuffing
    _ESC            = 0x1B,
    _ESC_START_HB   = 2,  // HBus
    _ESC_START_MQ   = 3,  // MQTT-SN
    _ESC_START_HBE  = 4,  // HBus encrypted
    _ESC_START_MQE  = 5,  // MQTT-SN encrypted
    _ESC_END        = 7,  // finish frame
    _ESC_ESC        = 8,
    _ESC_2ESC       = 9,

    // chars
    CHAR_BS         = 8,  // backspace
    CHAR_TAB        = 9,
    CHAR_LF         = 10,
    CHAR_CR         = 13,
    CHAR_SPACE      = 0x20,
    CHAR_DEL        = 0x7F,

    // common constants
    READY           = 1,
    NOT_READY       = 0,

    OK              = 0,
    SKIP            = 2,
    ERR             = 0xEE,
    ERR_BUSY        = 0xE0,
    ERR_PARAM       = 0xE1,
    ERR_ECHO        = 0xE2,
    ERR_UNKNOWN     = 0xE3,
    ERR_OVERFLOW    = 0xE4,
    ERR_SECURITY    = 0xE5,
    ERR_TMOUT       = 0xE6,
    ERR_TYPE        = 0xE7,

    // ------------------------------------
    // EEPROM addresses (max size 1024 = 0x400 bytes)
    // ------------------------------------
    EE_BOOT_TABLE   = 0x10,     // 16 bytes used by bootloader
    EE_SEED         = 0x30,     // random seed
    EE_PUP_CNT      = 0x32,     // count power-ups
    EE_SN           = 0x40,     // serial number
    EE_nodeID       = 0x48,     // own NodeId
    EE_TZ           = 0x4C,     // time zone
    EE_SECURITY     = 0x60,     // access control settings
    EE_SECURITY_INV = 0x62,     // inverted access control settings
    EE_XTEA_KEY     = 0x64,     // XTEA cipher key, 16 bytes
    EE_TOPIC_ID     = 0x80,     // own TopicIds, 2-bytes each, up to 32 topics
    EE_NAME_STR     = 0x100,    // name c-string, up to 63 chars
    EE_LOCATION_STR = 0x140,    // location c-string, up to 63 chars
    EE_DESCR_STR    = 0x180,    // description c-string, up to 63 chars

};

//##############################################################################
// Typedef
//##############################################################################

#ifndef __UCHAR_DEFINED__
  #define __UCHAR_DEFINED__
  typedef unsigned char uchar;
  typedef signed   char schar;
  typedef unsigned int  uint;
  typedef unsigned long ulong;
  typedef signed   long slong;
#endif


#ifndef __HB_MSG_T__
#define __HB_MSG_T__

typedef struct{
  uchar buf[MAX_BUF];
  uint  crc;
  uchar len;
  uchar postpone;
  union{
    uchar all;
    struct{
      unsigned gate     : 1;
      unsigned esc      : 1;
      unsigned hb       : 1;
      unsigned encrypt  : 1;
      unsigned ts_ok    : 1;
      unsigned valid    : 1;
      unsigned busy     : 1;
    };
  };
}hb_msg_t;
#endif


//##############################################################################
// Inc
//##############################################################################

#include "i2c_bitbang.h"

//##############################################################################
// Var
//##############################################################################

struct node_struct{
  uint seed;                // random seed
  uint pup_cnt;             // power-up count
  union{
      ulong SN;             // consists of two parts: batch No (msw) and number in batch (lsw)
      int   sn[2];
  };
  union{
      uint  ID;             // own ID
      uchar id[2];
  };  
  int  tz;                  // time zone, minutes to UTC
  char name_str[MAX_SSTR];  
  char location_str[MAX_LSTR];  
  char descr_str[MAX_LSTR];     // description
};
extern struct node_struct node;

extern uint led_cnt;

extern StaticJsonBuffer<128> jsonBuf;
extern Coos <COOS_TASKS, 0> coos;    // 1 ms ticks


//##############################################################################
// Func
//##############################################################################


void blink(uint dur);

uchar print_val(uchar val, uchar i);
void print_buf(const char* name, hb_msg_t* msg);
void printbuf(uchar* buf, uchar len);

void copy_buf(uchar* src, uchar* dst, uchar len);
void rev_4_bytes(uchar* buf);

void shift_buf(uchar* buf, uchar pos, uchar len);
uint crc_add_uchar(uchar b, uint crcx);
uint calc_crc(uchar* buf, uchar len);
void crc_to_msg(hb_msg_t* msg);

uchar begin_txmsg(hb_msg_t* txmsg, uchar hb);
uchar add_txmsg_uchar(hb_msg_t* txmsg, uchar c);
uchar add_txmsg_z_str(hb_msg_t* txmsg, char* str);
void copy_msg_hdr(hb_msg_t* src, uchar first, uchar last, hb_msg_t* txmsg);
void add_ts(hb_msg_t* txmsg);
uchar finish_txmsg(hb_msg_t* txmsg);

uchar ts_valid(hb_msg_t* rxmsg);
uchar sort(uint* arr, uint len);




#endif /* __HB_COMMON_H */
