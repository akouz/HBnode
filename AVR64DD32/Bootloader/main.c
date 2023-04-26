/*
 * File     main.c
 * Target   AVRxxDD32
 * Compiler XC8 v2.40

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
 * After power-up bootloader tries to access external EEPROM. 
 * If EEPROM stores a valid code, bootloader copies it into application code area. 
 * 
 * I2C access by bit-bang on pins PC2 (SCL) and PD1 (SDA).
 */

//##############################################################################
// Inc
//##############################################################################

#include "config.h"
#include "i2c_bitbang.h"

//##############################################################################
// Fuses
//##############################################################################

#define BLOCK_SIZE  0x200   // 512 bytes
#define BOOT_BLOCKS 3       // BOOTSIZE boot = 512*3 = 1.5K

FUSES = {
	.WDTCFG = 0x00, // WDTCFG {PERIOD=OFF, WINDOW=OFF}
	.BODCFG = 0x00, // BODCFG {SLEEP=DISABLE, ACTIVE=DISABLE, SAMPFREQ=128Hz, LVL=BODLEVEL0}
	.OSCCFG = 0x00, // OSCCFG {CLKSEL=OSCHF}
	.SYSCFG0 = 0xD9, // SYSCFG0 {EESAVE=keep, RSTPINCFG=RST, UPDIPINCFG=UPDI, CRCSEL=CRC16, CRCSRC=NOCRC}
	.SYSCFG1 = 0x14, // SYSCFG1 {SUT=8MS, MVSYSCFG=SINGLE}
	.CODESIZE = 0x00, // CODESIZE no appdata, only boot+appcode
	.BOOTSIZE = BOOT_BLOCKS, 
};

LOCKBITS = 0x5CC5C55C; // {KEY=NOLOCK}

//##############################################################################
// Var
//##############################################################################

union ee_union{
    uchar buf[8];
    struct{
        uchar   patt[4];
        uchar   addr[2];
        uchar   len[2];
    };
}descr; // 

uchar eebuf[BLOCK_SIZE];

//##############################################################################
// Func
//##############################################################################

void wait_flash(void);
void start_write_flash(uint addr);

// ============================================
// Boot
// ============================================
__attribute__((naked)) __attribute__((section(".ctors"))) void boot(void)
{
    uint addr, len, i, cnt, data;
    //disable interrupts
    asm("cli");
    while(1)
    {    
        asm("nop");
        init_i2c();
        if (OK == i2c_read(descr.buf, 0x08, 0x08))      // read descriptor: 8 bytes starting from addr 8
        {
            if (OK == check_pattern(descr.patt))        // if pattern matched
            {
                addr = 0x100*descr.addr[0] + descr.addr[1]; // address
                len = 0x100*descr.len[0] + descr.len[1];    // length
                if (addr == BLOCK_SIZE*BOOT_BLOCKS)         // address matches
                {
                    cnt = 0;
                    data = 0;
                    while (cnt < len)
                    {
                        asm("wdr");     // reset watchdog
                        // -----------------------
                        // read block from EEPROM
                        // -----------------------
                        i2c_read(eebuf, addr, BLOCK_SIZE);

                        // -----------------------
                        // erase block in flash
                        // -----------------------
                        start_write_flash(addr);

                        // -----------------------
                        // write block to flash
                        // -----------------------
                        for (i=0; i<BLOCK_SIZE; i++)
                        {
                            if (addr & 1) // odd address
                            {
                                data |= eebuf[i];   // LSB byte
                                pgm_word_write((addr & 0xFFFE), data);
                            }    
                            else // even address
                            {
                                data = (uint)eebuf[i] << 8;    // MSB byte, big endian
                            }    
                            addr++;
                            cnt++;
                            if (cnt >= len)
                                break;
                        }    
                    }   
                    wait_flash();
                    // -----------------------
                    // clear descriptor to prevent further writes
                    // -----------------------
                    for (i=0; i<8; i++)
                    {
                        descr.buf[i] = 0; // make 0-descriptor
                    }    
                    i2c_write(descr.buf, 0x08, 0x08);     // write 0-descriptor to EEPROM
                    _delay_ms(10);
                }    
            }    
        }
        asm("nop");
       _delay_ms(10);
        asm("wdr");     // reset watchdog
    }
    //enable interrupts
    asm("sei");
    
    // Jump to application code
    pgm_jmp_far(BLOCK_SIZE*BOOT_BLOCKS);    
}

// ============================================
// Wait for completion of previous write
// ============================================
void wait_flash(void)
{
    while (NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm | NVMCTRL_FBUSY_bm))
    {
       asm("wdr");      // reset WDR
    }                 
}

// ============================================
// Erase page and set write to flash command
// ============================================
void start_write_flash(uint addr)
{
    // erase flash
    wait_flash();
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc);  // clear current command    
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_FLPER_gc); // send erase command     
    pgm_word_write(addr, 0x00);     // start erase operation
    wait_flash();                   // wait until finished
    
    // write to flash
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc); // clear current command  
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_FLWR_gc); // flash write command          
}

// ============================================
// Dummy main
// ============================================
int main(void)
{
    while(1)
    {
        asm("wdr");      // reset WDR
    }    
    return -1;
}

/* EOF */