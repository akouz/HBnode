/*
 * File     config.c
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

//##############################################################################
// Inc
//##############################################################################

#include "config.h"
#include "flash_access.h"

//##############################################################################
// Func
//##############################################################################

#ifdef DEBUG_BOOT    
// =============================================
// debug - fill descriptor
// =============================================
void fill_descr(uchar* buf)
{
    buf[0] = 0x55;      // pattern
    buf[1] = 0xAA;
    buf[2] = 0xC3;
    buf[3] = 0x3C;
    buf[4] = 1;         // len
    buf[5] = 0;
}
#endif

// =============================================
// Delay 100 us
// =============================================
void delay_100us(void)
{
    uchar i;
    for (i=0; i<80; i++)
    {
        NOP();
        asm("wdr");      // reset watchdog
    }    
}

// =============================================
// Delay 3 us
// =============================================
void delay_3us(void)
{
    NOP();
    asm("wdr");      // reset watchdog
}

// =============================================
// Check pattern
// =============================================
inline uchar check_pattern(uchar* buf)
{
  if (buf[0] != 0x55) return ERR;
  if (buf[1] != 0xAA) return ERR;
  if (buf[2] != 0xC3) return ERR;
  if (buf[3] != 0x3C) return ERR;
  return OK;
}        

// ============================================
// Wait for completion of previous write
// ============================================
void wait_flash(void)
{
    while (NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm | NVMCTRL_FBUSY_bm))
    {
       asm("wdr");      // reset watchdog
    }                 
}

// ============================================
// Erase page and set write to flash command
// ============================================
void start_write_flash(uint adrr)
{
    // erase flash
    wait_flash();
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc);  // clear current command    
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_FLPER_gc); // send erase command     
    pgm_word_write(adrr, 0x00);     // start erase operation
    wait_flash();                   // wait until finished
    
    // write to flash
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc); // clear current command  
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_FLWR_gc); // flash write command          
}

/* EOF */