/*
 * File     config.h
 * Target   HBnode (AVRxxDD32)
 * Compiler Arduino with DxCore

 * (c) 2023 Alex Kouznetsov,  https://github.com/akouz/hbnode
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

#ifndef __CONFIG_H
#define	__CONFIG_H

//##############################################################################
// Inc
//##############################################################################

#include <coos.h>

//##############################################################################
// History
//##############################################################################

// rev 1.1 - monitor parser works
// rev 1.2 - monitor processing works, can rd/wr serial No
// rev 1.3 - rd/wr name, location, comment, guid
// rev 1.4 - can clear name, location, comment, guid by entering "-"
// rev 1.5 - bug fix
// rev 1.6 - bug fix

//##############################################################################
// Def
//##############################################################################

// ==================================
// Software revision
// ==================================
#define SKETCH_NAME   "test_monitor.ino"
#define SW_REV_MAJ    1
#define SW_REV_MIN    6

#define SW_REV  (0x100*SW_REV_MAJ + SW_REV_MIN)

#pragma message("rev 1.6")

#ifndef __UCHAR_DEFINED__
  #define __UCHAR_DEFINED__
  typedef unsigned char uchar;
  typedef signed   char schar;
  typedef unsigned int  uint;
  typedef unsigned long ulong;
  typedef signed   long slong;
#endif

#ifndef NULL  
    #define NULL 0  
#endif
  
#define OK      0
#define ERR     0xEE  
#define CR      13
#define LF      10  
  
#define BIT0    0x01  
#define BIT1    0x02
#define BIT2    0x04  
#define BIT3    0x08  
#define BIT4    0x10  
#define BIT5    0x20  
#define BIT6    0x40  
#define BIT7    0x80  
  
#define NOP()   asm("nop") 

enum{
    TICK_1000US   = 0,    // 1 ms in average because of corrections made in millis(),  
                          // see https://www.best-microcontroller-projects.com/arduino-millis.html
    TICK_1024US   = 1     // 1.024 ms
};
#define TICK   TICK_1000US  // select tick
  
#define SET_SCL PORTD.DIRCLR = BIT3   // input
#define CLR_SCL PORTD.DIRSET = BIT3   // output
#define SET_SDA PORTD.DIRCLR = BIT4
#define CLR_SDA PORTD.DIRSET = BIT4   
#define SDA_IN (PORTD.IN & BIT4)

//#define SET_LED PORTF.DIRSET = BIT5;  PORTF.OUTCLR = BIT5
//#define CLR_LED PORTF.OUTSET = BIT5  
  
#define EE_WR_ADDR  0xA8        // A0=0, A1=0, A2=1  
#define EE_RD_ADDR  0xA9        // A0=0, A1=0, A2=1  
#define FT_WR_ADDR  0x44        // 7-bit addr 0x22
#define FT_RD_ADDR  0x45  

#define RED_LED  PIN_PF5
#define GRN_LED  PIN_PD5

#define RLED  RED_LED
#define GLED  GRN_LED

#define TOGGLE_GLED PORTD.OUT ^= BIT5

//##############################################################################
// Var
//##############################################################################

extern ulong sn;

//##############################################################################
// Inc
//##############################################################################

#include "i2c_bitbang.h"


#endif	/* __CONFIG_H */
