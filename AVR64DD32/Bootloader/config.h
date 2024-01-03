/*
 * File     config.h
 * Target   AVRxxDD32
 * Compiler AVR-GCC v7.3.0  pack AVR-Rx_DFP v1.10.124

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

/*
 * rev 0.1      - bitbang debugged, bootloader fits in 1K bytes 
 * rev 0.2      - bootloader writes into flash OK
 * rev 0.3      - bootloader fits in 1K
 * rev 0.4      - tune-up
 * rev 1.0      - debugged
 * rev 1.1      - bug fix, added CCP = IOREG
 * rev 1.2      - do not use WDT while running bootloader
 * rev 1.3      - disable MVIO, see .SYSCFG1, it is required for ADC inputs at port C pins 
 */

#ifndef __CONFIG_H
#define	__CONFIG_H

//##############################################################################
// Inc
//##############################################################################

#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>

//##############################################################################
// Config
//##############################################################################

// #define DEBUG_BOOT

#define BLOCK_SIZE  0x200       // 512 bytes
#ifdef DEBUG_BOOT
    #define BOOT_BLOCKS 4       // BOOTSIZE boot = 512*4 = 2K, use compiler optimisation '1'
#else
    #define BOOT_BLOCKS 2       // BOOTSIZE boot = 512*2 = 1K, use compiler optimisation 's'
#endif
#define APPCODE_ADDR BLOCK_SIZE * BOOT_BLOCKS

//##############################################################################
// Def
//##############################################################################

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
  
#define IOREG   0xD8    // unlock key  
  
#define BIT0    0x01  
#define BIT1    0x02
#define BIT2    0x04  
#define BIT3    0x08  
#define BIT4    0x10  
#define BIT5    0x20  
#define BIT6    0x40  
#define BIT7    0x80  
  
#define NOP()   asm("nop") 
  
#define SET_SCL PORTD.DIRCLR = BIT3   // input
#define CLR_SCL PORTD.DIRSET = BIT3   // output
#define SET_SDA PORTD.DIRCLR = BIT4
#define CLR_SDA PORTD.DIRSET = BIT4   
#define SDA_IN (PORTD.IN & BIT4)

#define SET_LED PORTF.DIRSET = BIT5;  PORTF.OUTCLR = BIT5
#define CLR_LED PORTF.OUTSET = BIT5  
  
#define EE_WR_ADDR  0xA8        // A0=0, A1=0, A2=1  
#define EE_RD_ADDR  0xA9        // A0=0, A1=0, A2=1  
#define FT_WR_ADDR  0x44        // 7-bit addr 0x22
#define FT_RD_ADDR  0x45  
  
//##############################################################################
// Func
//##############################################################################

#ifdef DEBUG_BOOT  
    void fill_descr(uchar* buf);
#endif
void delay_100us(void);
void delay_3us(void);
uchar check_pattern(uchar* buf);  
void wait_flash(void);
void start_write_flash(uint addr);

#endif	/* __CONFIG_H */

