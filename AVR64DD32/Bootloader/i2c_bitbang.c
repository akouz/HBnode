/*
 * File     i2c_bitbang.c
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
 * ----------------------------------------------
 * 
 * Read/write I2C EEPROM 24LC512 connected to pins PC2 (SCL) and PD1 (SDA)
 * 
 */

//##############################################################################
// Inc
//##############################################################################

#include "i2c_bitbang.h"

//##############################################################################
// Func
//##############################################################################

// ============================================
// Stop
// ============================================
// for correct operation initial state must be SDA=0, SCL=0
void i2c_stop(void)
{
    SET_SCL;        // set as input
    delay_3us();
    SET_SDA;        // set as input
    NOP();
}
// ============================================
// Init ports
// ============================================
void init_i2c(void)
{
   // PORTC.PIN2CTRL = BIT3;  // PC2 pull-up
   // PORTD.PIN1CTRL = BIT3;  // PD1 pull-up
    i2c_stop();             // ensure initial state SDA=1, SCL=1
    PORTC.OUTCLR = BIT2;    // clear PC2 output latch
    PORTD.OUTCLR = BIT1;    // clear PD1 output latch
}
// ============================================
// Start or restart
// ============================================
void i2c_start(void)
{
    i2c_stop();     // in case or re-start
    CLR_SDA;        // set as output
    delay_3us();
    CLR_SCL;        // set as output
    NOP();
}
// ============================================
// One SCL pulse, also read SDA
// ============================================
uchar SCL_pulse(void)
{
    uchar res;
    NOP();
    SET_SCL;                    // start pulse
    delay_3us();
    res = (SDA_IN)? 1 : 0;      // read SDA
    CLR_SCL;                    // finish pulse
    NOP();
    return res;
}
// ============================================
// Send byte, return Ack
// ============================================
uchar i2c_sbyte(uchar val)
{
    uchar i, res;
    for (i=0;i<8; i++)
    {
        if (val & 0x80)     // check MSB bit
           SET_SDA;
        else
           CLR_SDA;
        val <<= 1;
        SCL_pulse();
    }    
    SET_SDA;                
    res = SCL_pulse();      // read Ack
    CLR_SDA;
    return res;             
}
// ============================================
// Read byte
// ============================================
uchar i2c_rbyte(uchar nack)
{
    uchar i, res=0;
    SET_SDA;
    for (i=0; i<8; i++)
    {
        res <<= 1;
        res |= SCL_pulse();
    }    
    if (nack) 
        SET_SDA;        // last byte, send Nack
    else
        CLR_SDA;        // sequential byte, send Ack
    SCL_pulse();
    CLR_SDA;
    return res;
}
// ============================================
// Read EEPROM to buffer
// ============================================
uchar i2c_read(uchar* buf, uint adrr, uint len) 
{
    uint i;
    uchar res = ERR;                    // in case is there is no EEPROM
    i2c_start();
    // send device addr (A0,A1=low, A2=high), write
    if (i2c_sbyte(EE_WR_ADDR) == 0)     // Ack=0 if EEPROM exists
    {
        i2c_sbyte((uchar)(adrr >> 8));  // byte addr, msb
        i2c_sbyte((uchar)adrr);         // byte addr, lsb
        i2c_start();                    // re-start
        i2c_sbyte(EE_RD_ADDR);          // device addr, read
        for (i=0; i<len-1; i++)
        {
            buf[i] = i2c_rbyte(0);      // read, send Ack
        }            
        buf[len-1] = i2c_rbyte(1);      // last byte, send Nack
        res = OK;
    }    
    i2c_stop();
    return res;
}        
// ============================================
// Write buffer to EEPROM 
// ============================================
// len should not exceed EEPROM buffer size, 128 bytes for 24LC512
void i2c_write(uchar* buf, uint adrr, uchar len) 
{
    uchar i;
    i2c_start();
    // send device addr (A0,A1=low, A2=high), write
    if (i2c_sbyte(EE_WR_ADDR) == 0)    // Ack=0 if EEPROM exists
    {
        i2c_sbyte((uchar)(adrr >> 8)); // byte addr, msb
        i2c_sbyte((uchar)adrr);        // byte addr, lsb
        for (i=0; i<len; i++)
        {
            if (buf)
                i2c_sbyte(buf[i]);      // write data
            else
                i2c_sbyte(0);           // fill by 0;
        }    
    }
    i2c_stop();
    for (i=0; i<100; i++)
    {
        delay_100us();
    }    
//    _delay_ms(10);
}

#ifdef DEBUG_BOOT
// ============================================
// Hex nibble to char
// ============================================
uchar nibble_to_char(uchar nib)
{
    uchar res;
    nib &= 0x0F;
    if (nib < 10)
        res = '0' + nib;
    else
        res = 'A' + nib - 10;
    return res;
}
// ============================================
// Print white space
// ============================================
inline void print_blank(void)
{
    i2c_sbyte(' ');        
}
// ============================================
// Print white space
// ============================================
inline void print_CR_LF(void)
{
    i2c_sbyte(CR);
    i2c_sbyte(LF);    
}
// ============================================
// Send addr
// ============================================
void print_addr(uint addrr)
{
    print_CR_LF();
    i2c_sbyte(nibble_to_char(addrr >> 12));
    i2c_sbyte(nibble_to_char(addrr >> 8));
    i2c_sbyte(nibble_to_char(addrr >> 4));
    i2c_sbyte(nibble_to_char(addrr));
    i2c_sbyte(':');
}
// ============================================
// Send byte
// ============================================
void print_byte(uint val)
{
    i2c_sbyte(nibble_to_char(val >> 4));
    i2c_sbyte(nibble_to_char(val));
    print_blank();
}
// ============================================
// Send buffer to FT200XD, https://ftdichip.com/products/ft200xd/
// ============================================
uchar i2c_send(uint addrr, uchar* buf, uint len) 
{
    uchar ack, i;
    i2c_start();
    // send device addr , write
    ack = i2c_sbyte(FT_WR_ADDR);
    if (ack == 0)    // if FT200XD exists 
    {
        if (buf)
        {
            print_addr(addrr);
            for (i=0; i<len; i++)
            {
                if ((i & 7) == 0)
                    print_blank();
                print_byte(buf[i]); 
            }                
        }    
        else
        {
            print_CR_LF();
        }    
    }
    i2c_stop();
    return ack;
}
#endif

/* EOF */