/**
 * Project  HBus bootloader - demo host application
 * File     main.c 
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
 * 
 */

#define F_CPU 4000000UL      // 4 MHz

#include <avr/io.h>
#include <util/delay.h>

//##############################################################################
// Fuses
//##############################################################################

//#define DEBUG_BOOT

#define BLOCK_SIZE  0x200   // 512 bytes

#ifdef DEBUG_BOOT
    #define BOOT_BLOCKS 4 
#else
    #define BOOT_BLOCKS 2       // BOOTSIZE boot = 512*2 = 1K
#endif

FUSES = {
	.WDTCFG = 0x00, // WDTCFG {PERIOD=OFF, WINDOW=OFF}
	.BODCFG = 0x00, // BODCFG {SLEEP=DISABLE, ACTIVE=DISABLE, SAMPFREQ=128Hz, LVL=BODLEVEL0}
	.OSCCFG = 0x00, // OSCCFG {CLKSEL=OSCHF}
	.SYSCFG0 = 0xD9, // SYSCFG0 {EESAVE=keep, RSTPINCFG=RST, UPDIPINCFG=UPDI, CRCSEL=CRC16, CRCSRC=NOCRC}
	.SYSCFG1 = 0x14, // SYSCFG1 {SUT=8MS, MVSYSCFG=SINGLE}
	.CODESIZE = 0,   // CODESIZE no appdata, only boot+appcode
	.BOOTSIZE = BOOT_BLOCKS, 
};


//##############################################################################
// Def
//##############################################################################

#define BIT0    0x01  
#define BIT1    0x02
#define BIT2    0x04  
#define BIT3    0x08  
#define BIT4    0x10  
#define BIT5    0x20  
#define BIT6    0x40  
#define BIT7    0x80  

//##############################################################################
// Func
//##############################################################################

// ======================================
// Init LED1 (PF5)
// ======================================
static inline void RED_LED_init(void)
{    
    VPORTF.DIR = BIT5; // set as output
    VPORTF.OUT = BIT5; // set passive high level
}
// ======================================
// Toggle LED1 (PF5)
// ======================================
static inline void RED_LED_toggle(void)
{
    VPORTF.OUT ^= BIT5;
}
// ======================================
// Init LED2 (PD5)
// ======================================
static inline void GREEN_LED_init(void)
{
    VPORTD.DIR = BIT5; // set as output
    VPORTD.OUT = BIT5; // set passive high level
}
// ======================================
// Toggle LED2 (PD5)
// ======================================
static inline void GREEN_LED_toggle(void)
{    
    VPORTD.OUT ^= BIT5;
}

// =========================================================
// Main loop
// =========================================================
int main(void)
{
    RED_LED_init();    
    GREEN_LED_init();    
    GREEN_LED_toggle();
    while (1) 
    {
        _delay_ms(500);
        GREEN_LED_toggle();
        RED_LED_toggle();
    }
}

/* EOF */
