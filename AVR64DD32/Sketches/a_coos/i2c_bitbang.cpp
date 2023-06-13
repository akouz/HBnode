/*
 * File     i2c_bitbang.c
 * Target   HBnode (AVRxxDD32)
 * Compiler Arduino with DxCore

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

#include "config.h"

//##############################################################################
// Func
//##############################################################################


// =============================================
// Constructor
// =============================================
I2Cbb::I2Cbb(void)
{
    this->stop();           // ensure initial state SDA=1, SCL=1
    PORTC.OUTCLR = BIT2;    // clear PC2 output latch
    PORTD.OUTCLR = BIT1;    // clear PD1 output latch
}    

// ============================================
// Stop
// ============================================
// for correct operation initial state must be SDA=0, SCL=0
void I2Cbb::stop(void)
{
    SET_SCL;        // set as input
    delayMicroseconds(3);
    SET_SDA;        // set as input
    NOP();
}
// ============================================
// Start or restart
// ============================================
void I2Cbb::start(void)
{
    this->stop();    // in case or re-start
    CLR_SDA;        // set as output
    delayMicroseconds(3);
    CLR_SCL;        // set as output
    NOP();
}
// ============================================
// One SCL pulse, also read SDA
// ============================================
uchar I2Cbb::pulse(void)
{
    uchar res;
    NOP();
    SET_SCL;                    // start pulse
    delayMicroseconds(3);
    res = (SDA_IN)? 1 : 0;      // read SDA
    CLR_SCL;                    // finish pulse
    NOP();
    return res;
}
// ============================================
// Send byte, return Ack
// ============================================
uchar I2Cbb::sbyte(uchar val)
{
    uchar i, res;
    for (i=0; i<8; i++)
    {
        if (val & 0x80)     // check MSB bit
           SET_SDA;
        else
           CLR_SDA;
        val <<= 1;
        this->pulse();
    }    
    SET_SDA;                
    res = this->pulse();      // read Ack
    CLR_SDA;
    return res;             
}
// ============================================
// Read byte
// ============================================
uchar I2Cbb::rbyte(uchar nack)
{
    uchar i, res=0;
    SET_SDA;
    for (i=0; i<8; i++)
    {
        res <<= 1;
        res |= this->pulse();
    }    
    if (nack) 
        SET_SDA;        // last byte, send Nack
    else
        CLR_SDA;        // sequential byte, send Ack
    this->pulse();
    CLR_SDA;
    return res;
}
// ============================================
// Read EEPROM to buffer
// ============================================
uchar I2Cbb::read_EE(uchar* buf, uint adrr, uint len) 
{
    uint i;
    uchar res = ERR; 
    if ((len) && (this->EE_busy == 0))
    {
        this->start();
        // send device addr (A0,A1=low, A2=high), write
        if (this->sbyte(EE_WR_ADDR) == 0)     // Ack=0 if EEPROM exists
        {
            this->sbyte((uchar)(adrr >> 8));  // byte addr, msb
            this->sbyte((uchar)adrr);         // byte addr, lsb
            this->start();                    // re-start
            this->sbyte(EE_RD_ADDR);          // device addr, read
            for (i=0; i<len-1; i++)
            {
                buf[i] = this->rbyte(0);      // read, send Ack
            }            
            buf[len-1] = this->rbyte(1);      // last byte, send Nack
            res = OK;
        }    
        this->stop();
    }
    return res;
}        
// ============================================
// Write buffer to EEPROM 
// ============================================
uchar I2Cbb::write_EE(uchar* buf, uint adrr, uint len) 
{
    uint i;
    uchar res = ERR; 
    if ((len) && (this->EE_busy == 0))
    {
        this->start();
        // send device addr (A0,A1=low, A2=high), write
        if (this->sbyte(EE_WR_ADDR) == 0)     // Ack=0 if EEPROM exists
        {
            res = OK;
            this->sbyte((uchar)(adrr >> 8));  // byte addr, msb
            this->sbyte((uchar)adrr);         // byte addr, lsb
            for (i=0; i<len-1; i++)
            {
                if (0 != this->sbyte(buf[i]))
                {
                    res = ERR;
                    break;
                } 
            }            
        }    
        this->stop();
        if (0 == res)
           this->EE_busy = 10;      // wait 10 ma until write finished
    }
    return res;
}        

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
// Send byte
// ============================================
void I2Cbb::send_byte(uchar val)
{
    this->sbyte(nibble_to_char(val >> 4));
    this->sbyte(nibble_to_char(val));
}

// ============================================
// CR+LF
// ============================================
uchar I2Cbb::println(void) 
{
    uchar ack;
    this->start();
    ack = this->sbyte(FT_WR_ADDR);
    if (ack == 0)    // if FT200XD exists 
    {
        this->sbyte(CR);
        this->sbyte(LF);    
    }    
    this->stop();
    return ack;
}
// ============================================
// Print string to FT200XD, https://ftdichip.com/products/ft200xd/
// ============================================
uchar I2Cbb::print_str(char* buf, uchar crlf) 
{
    uchar ack, i;
    this->start();
    ack = this->sbyte(FT_WR_ADDR);
    if (ack == 0)    // if FT200XD exists 
    {
        if (buf)
        {
            for (i=0; i<0xFF; i++)
            {
                if (buf[i])
                    this->sbyte((uchar)buf[i]); 
                else
                    break;  
            }                
            if (crlf)
            {
                this->sbyte(CR);
                this->sbyte(LF);    
            }    
        }    
    }
    this->stop();
    return ack;
}
uchar I2Cbb::print(char* str) 
{
    return this->print_str(str, 0);
}    
uchar I2Cbb::println(char* str) 
{
    return this->print_str(str, 1);
}    
uchar I2Cbb::print(const char* str) 
{
    return this->print_str((char*)str, 0);
}    
uchar I2Cbb::println(const char* str) 
{
    return this->print_str((char*)str, 1);
}    
// ============================================
// Print char
// ============================================
uchar I2Cbb::print_char(char val, uchar crlf) 
{
    uchar ack;
    this->start();
    ack = this->sbyte(FT_WR_ADDR);
    if (ack == 0)    // if FT200XD exists 
    {
        this->sbyte((uchar)val);
        if (crlf)
        {
            this->sbyte(CR);
            this->sbyte(LF);    
        }    
    }    
    this->stop();
    return ack;
}
uchar I2Cbb::print(char val) 
{
    return this->print_char(val, 0);
}
uchar I2Cbb::println(char val) 
{
    return this->print_char(val, 1);
}
// ============================================
// Print byte
// ============================================
uchar I2Cbb::print_uchar(uchar val, uchar crlf) 
{
    uchar ack;
    this->start();
    ack = this->sbyte(FT_WR_ADDR);
    if (ack == 0)    // if FT200XD exists 
    {
        this->send_byte(val);
        if (crlf)
        {
            this->sbyte(CR);
            this->sbyte(LF);    
        }    
    }    
    this->stop();
    return ack;
}
uchar I2Cbb::print(uchar val) 
{
    return this->print_uchar(val, 0);
}
uchar I2Cbb::println(uchar val) 
{
    return this->print_uchar(val, 1);
}
// ============================================
// Print word
// ============================================
uchar I2Cbb::print_uint(uint val, uchar crlf) 
{
    uchar ack;
    this->start();
    ack = this->sbyte(FT_WR_ADDR);
    if (ack == 0)    // if FT200XD exists 
    {
        this->send_byte((uchar)(val >> 8));
        this->send_byte((uchar)val);
        if (crlf)
        {
            this->sbyte(CR);
            this->sbyte(LF);    
        }    
    }    
    this->stop();
    return ack;
}
uchar I2Cbb::print(uint val) 
{
    return this->print_uint(val, 0);
}
uchar I2Cbb::println(uint val) 
{
    return this->print_uint(val, 1);
}
// ============================================
// Print long word
// ============================================
uchar I2Cbb::print_ulong(ulong val, uchar crlf) 
{
    uchar ack;
    this->start();
    ack = this->sbyte(FT_WR_ADDR);
    if (ack == 0)    // if FT200XD exists 
    {
        this->send_byte((uchar)(val >> 24));
        this->send_byte((uchar)(val >> 16));
        this->send_byte((uchar)(val >> 8));
        this->send_byte((uchar)val);
        if (crlf)
        {
            this->sbyte(CR);
            this->sbyte(LF);    
        }    
    }    
    this->stop();
    return ack;
}
uchar I2Cbb::print(ulong val) 
{
    return this->print_ulong(val, 0);
}
uchar I2Cbb::println(ulong val) 
{
    return this->print_ulong(val, 1);
}
// ============================================
// Print integer
// ============================================
uchar I2Cbb::print_int(int val, uchar crlf) 
{
    uchar ack, len, i;
    char buf[0x10];
    this->start();
    ack = this->sbyte(FT_WR_ADDR);
    if (ack == 0)    // if FT200XD exists 
    {
        len = sprintf(buf,"%d", val);
        for (i=0; i<len; i++)
        {
            this->sbyte((uchar)buf[i]);
        }        
        if (crlf)
        {
            this->sbyte(CR);
            this->sbyte(LF);    
        }    
    }    
    this->stop();
    return ack;
}
uchar I2Cbb::print(int val) 
{
    return this->print_int(val, 0);
}
uchar I2Cbb::println(int val) 
{
    return this->print_int(val, 1);
}

/* EOF */