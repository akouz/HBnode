/*
 * File     i2c_bitbang.cpp
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
 * 
 */

//##############################################################################
// Inc
//##############################################################################

#include "i2c_bitbang.h"

//##############################################################################
// Var
//##############################################################################

I2Cbb i2cbb;          // I2C bit-bang to access FT200XD test jig and EEPROM

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
    delayMicroseconds(2);
    SET_SCL;        // set as input
    delayMicroseconds(4);
    SET_SDA;        // set as input
    delayMicroseconds(3);
}
// ============================================
// Start or restart
// ============================================
void I2Cbb::start(void)
{
    this->stop();   // in case or re-start
    CLR_SDA;        // set as output
    delayMicroseconds(4);
    CLR_SCL;        // set as output
    delayMicroseconds(2);
}
// ============================================
// One SCL pulse, also read SDA
// ============================================
uchar I2Cbb::pulse(void)
{
    uchar res;
    delayMicroseconds(2);
    SET_SCL;                    // start pulse
    delayMicroseconds(4);
    res = (SDA_IN)? 1 : 0;      // read SDA
    CLR_SCL;                    // finish pulse
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
    NOP();
    return res;             
}
// ============================================
// Read byte
// ============================================
uchar I2Cbb::rbyte(uchar nack)
{
    uchar i, res = 0;
    SET_SDA;
    NOP();
    for (i=0; i<8; i++)
    {
        res <<= 1;
        res |= this->pulse();
    }    
    if (nack) 
        SET_SDA;        // last byte, send Nack
    else
        CLR_SDA;        // sequential byte, send Ack
    NOP();
    this->pulse();
    CLR_SDA;
    NOP();
    return res;
}
// ============================================
// Calculate CRC for code in EEPROM starting from address 0x0400
// ============================================
uint I2Cbb::crc_EE(uint codelen)
{
    uchar buf[0x40];
    uint crcx = 0xFFFF;
    uint addr = 0x0400;
    uchar chunk;
    for (uint i=0; i<2048; i++) 
    {
        chunk = (codelen >= 0x40)? 0x40 : (uchar)codelen;
        if (chunk)
        {
            this->read_EE(buf, addr, chunk);
            codelen -= chunk;
            addr += chunk;
            for (uchar j=0; j<chunk; j++)
            {
//                crcx = crc_add_uchar(buf[j], crcx);
                crcx ^= (uint)buf[j] << 8;
                for (uchar k=0; k<8; k++)
                {
                    crcx = (crcx & 0x8000)? ((crcx << 1) ^ 0x1021) : (crcx << 1);
                }
            }
        } 
        else
            break;       
    }
    return crcx;
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
uchar I2Cbb::write_EE(uchar* buf, uint adrr, uchar len) 
{
    uint i;
    uchar res = ERR; 
    if (buf == NULL)
    {
        PRINTLN(" buf=NULL");
    }
    if ((len) && (this->EE_busy == 0))
    {
        this->start();
        // send device addr (A0,A1=low, A2=high), write
        if (this->sbyte(EE_WR_ADDR) == 0)     // Ack=0 if EEPROM exists
        {
            res = OK;
            this->sbyte((uchar)(adrr >> 8));  // byte addr, msb
            this->sbyte((uchar)adrr);         // byte addr, lsb
            for (i=0; i<len; i++)
            {
                if (buf == NULL)
                {
                    if (0 != this->sbyte(0xFF))  // erase
                    {
                        res = ERR;
                        break;
                    } 
                }
                else
                {
                    if (0 != this->sbyte(buf[i]))
                    {
                        res = ERR;
                        break;
                    } 
                }   
            }  
        }    
        this->stop();
        if (0 == res)
           this->EE_busy = 10;      // wait 10 ms until write finished
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
// ===================================================
// Read char from FT200XD
// ===================================================
uchar I2Cbb::rd_char(char* val)
{
	uchar res;
	this->start();
    res = this->sbyte(FT_RD_ADDR); 
	if (OK == res)
	{
		*val = (char)this->rbyte(0);  // read last byte
	}
	this->stop();
    return res;
}
// ============================================
// CHAR_CR+CHAR_LF
// ============================================
uchar I2Cbb::println(void) 
{
    uchar ack;
    this->start();
    ack = this->sbyte(FT_WR_ADDR);
    if (ack == 0)    // if FT200XD exists 
    {
        this->sbyte(CHAR_CR);
        this->sbyte(CHAR_LF);    
    }    
    this->stop();
    return ack;
}
// ============================================
// Print string to FT200XD, https://ftdichip.com/products/ft200xd/
// ============================================
uchar I2Cbb::print_str(char* buf, uchar crlf, char quotes) 
{
    uchar ack, i;
    this->start();
    ack = this->sbyte(FT_WR_ADDR);
    if (ack == 0)    // if FT200XD exists 
    {
        if (quotes)
            this->sbyte((uchar)quotes);
        if (buf)
        {
            for (i=0; i<0xFF; i++)
            {
                if (buf[i])
                    this->sbyte((uchar)buf[i]); 
                else
                    break;  
            }                
        }    
        if (quotes)
            this->sbyte((uchar)quotes);
        if (crlf)
        {
            this->sbyte(CHAR_CR);
            this->sbyte(CHAR_LF);    
        }    
    }
    this->stop();
    return ack;
}
uchar I2Cbb::print(char* str) 
{
    return this->print_str(str, 0, 0);
}    
uchar I2Cbb::print_in_quotes(char* str) 
{
    return this->print_str(str, 0, '"');
}    
uchar I2Cbb::print(const char* str) 
{
    return this->print_str((char*)str, 0, 0);
}    
uchar I2Cbb::print_in_quotes(const char* str) 
{
    return this->print_str((char*)str, 0, '"');
}    
uchar I2Cbb::println(char* str) 
{
    return this->print_str(str, 1, 0);
}    
uchar I2Cbb::println_in_quotes(char* str) 
{
    return this->print_str(str, 1, '"');
}    
uchar I2Cbb::println(const char* str) 
{
    return this->print_str((char*)str, 1, 0);
}    
uchar I2Cbb::println_in_quotes(const char* str) 
{
    return this->print_str((char*)str, 1, '"');
}    
// ============================================
// Print str as hex
// ============================================
void I2Cbb::print_str_hex(char* str)
{
    uchar val;
    for (uchar i=0; i<0x80; i++)
    {
        val = (uchar)str[i];
        this->print_uchar(val,0);
        if (val)
        {
            this->print_char(' ',0);
            if ((i & 7) == 7)
              this->print_char(' ',0);
        }
        else
        {
            this->sbyte(CHAR_CR);
            this->sbyte(CHAR_LF);    
            break;
        }
    }    
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
            this->sbyte(CHAR_CR);
            this->sbyte(CHAR_LF);    
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
            this->sbyte(CHAR_CR);
            this->sbyte(CHAR_LF);    
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
            this->sbyte(CHAR_CR);
            this->sbyte(CHAR_LF);    
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
            this->sbyte(CHAR_CR);
            this->sbyte(CHAR_LF);    
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
            this->sbyte(CHAR_CR);
            this->sbyte(CHAR_LF);    
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
// ============================================
// Print buffer
// ============================================
void I2Cbb::print_buf(uchar* buf, uchar len, uchar crlf) 
{
    uchar endline = 0;
    this->println();
    for (uchar i=0; i<len; i++)
    {
        endline = ((i & 0x0F) == 0x0F)? 1 : 0; // end of line
        if ((i & 0x0F) == 8)
        {
            this->print_char(' ', 0);
        }
        this->print_uchar(buf[i], endline);
        if (!endline)
        {
            this->print_char(' ', 0);    
        }                
    }
    if ((!endline) && (crlf))
    {
        this->println();
    }    
}    
void I2Cbb::print(uchar* buf, uchar len) 
{
    this->print_buf(buf, len, 0); 
}
void I2Cbb::println(uchar* buf, uchar len) 
{
    this->print_buf(buf, len, 1); 
}

/* EOF */