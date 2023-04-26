/*
 * Project  HBus bootloader  
 * File     boot.c
 * Target   AVRxxDD32
 * Compiler AVR-GCC v7.3.0  pack AVR-Rx_DFP v1.10.124
 *
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
 * 
 * ----------------------------------------------
 * 
 * After power-up bootloader tries to access external I2C EEPROM 24LC512.
 * If EEPROM stores a valid code, bootloader copies it into application code area. 
 * 
 * I2C access by bit-bang on pins PC2 (SCL) and PD1 (SDA).
 * 
 * A valid code in EEPROM defined by a descriptor, an 8-byte record in EEPROM at
 * address 0x0010. Descriptor fields are as follows:
 * 
 * ----------------------------------
 * addr     content
 * ----------------------------------
 * 0x0010   0x55
 * 0x0011   0xAA
 * 0x0012   0xC3
 * 0x0013   0x3C
 * 0x0014   MSB byte of code length
 * 0x0015   LSB byte of code length
 * 0x0016   MSB byte of CRC
 * 0x0017   LSB byte of CRC
 * ----------------------------------
 * 
 * Bootloader does not use CRC. It is duty of application code to calculate
 * CRC of the supplied code starting from address $400 and to write descriptor
 * if CRC matches.
 * 
 * Bootloader copies specified number of bytes from EEPROM address 0x0400 into
 * program memory starting from addr 0x0400. When code copied, bootloader clears
 * descriptor and jumps to address 0x0400.
 * 
 */

//##############################################################################
// Inc
//##############################################################################

#include "config.h"
#include "flash_access.h"
#include "i2c_bitbang.h"

//##############################################################################
// Fuses
//##############################################################################

FUSES = {
	.WDTCFG = 0x00,     // WDTCFG {PERIOD=OFF, WINDOW=OFF}
	.BODCFG = 0x00,     // BODCFG {SLEEP=DISABLE, ACTIVE=DISABLE, SAMPFREQ=128Hz, LVL=BODLEVEL0}
	.OSCCFG = 0x00,     // OSCCFG {CLKSEL=OSCHF}
	.SYSCFG0 = 0xD9,    // SYSCFG0 {EESAVE=keep, RSTPINCFG=RST, UPDIPINCFG=UPDI, CRCSEL=CRC16, CRCSRC=NOCRC}
	.SYSCFG1 = 0x14,    // SYSCFG1 {SUT=8MS, MVSYSCFG=SINGLE}
	.CODESIZE = 0x00,   // CODESIZE no appdata, only boot+appcode
	.BOOTSIZE = BOOT_BLOCKS, 
};

//##############################################################################
// Var
//##############################################################################

uchar eebuf[BLOCK_SIZE];

uint adr, len, ii, cnt, dat;
uchar bb;

//##############################################################################
// Func
//##############################################################################

// ============================================
// Boot
// ============================================
__attribute__((naked)) __attribute__((section(".ctors"))) void boot(void)
{
    // Initialize system for AVR GCC support, expects r1 = 0 
    asm volatile("clr r1");

    //disable interrupts
    asm("cli");    
    init_i2c();
    if (OK == i2c_read(eebuf, 0x10, 8))     // read descriptor: 8 bytes starting from addr 0x10
    {
#ifdef DEBUG_BOOT    
        fill_descr(eebuf);      // pre-defined descriptor
#endif    
        len = ((uint)eebuf[4] << 8) + eebuf[5];     // length
        adr = APPCODE_ADDR;                 // byte addr
        if (OK == check_pattern(eebuf))     // if pattern matched
        {
            SET_LED;
            cnt = 0;
            dat = 0;
            while (cnt <= len)
            {
                asm("wdr");     // reset watchdog
                // -----------------------
                // read block from EEPROM
                // -----------------------
                i2c_read(eebuf, adr, BLOCK_SIZE);

                // -----------------------
                // erase block in flash
                // -----------------------
                start_write_flash(adr & 0xFE00); // block addr

                // -----------------------
                // write block to flash
                // -----------------------
                for (ii=0; ii<BLOCK_SIZE; ii++)
                {
                    if (adr & 1) // odd address
                    {
                        dat |= (uint)eebuf[ii] << 8;            // MSB byte
                        pgm_word_write((adr & 0xFFFE), dat);    // address must be even
                    }    
                    else // even address
                    {
                        dat = eebuf[ii];    // LSB byte
                    }    
                    adr++;
                    if (++cnt > len)
                        break;
                }    
                wait_flash();
            }    
            // -----------------------
            // clear descriptor to prevent repeated writes
            // -----------------------
            i2c_write(NULL, 0x10, 4);      // clear descriptor in EEPROM
            CLR_LED;
        } // if pattern matches    
    }
#ifdef DEBUG_BOOT    
    i2c_send(0,NULL,0); // CR+LF
    cnt = 0;
    adr = APPCODE_ADDR; 
    while (cnt < len)
    {
        for (ii=0; ii<0x10; ii++)
        {
            eebuf[ii] = pgm_byte_read(adr+ii);
        }    
        if (i2c_send(adr, eebuf, 0x10))
        {    
            break;  // FT200XD not connected
        }    
        cnt += 0x10;
        adr += 0x10;
        if ((adr & 0x007F) == 0)
        {
            i2c_send(0,NULL,0); // CR+LF            
        }
    }    
    while(1)
    {    
        NOP();
    }    
#else    
    asm("jmp 0x0400");  
#endif    
}

/* EOF */