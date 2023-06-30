/*
 * File     i2c_bitbang.h
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

#ifndef __I2C_BITBANG_H
#define	__I2C_BITBANG_H

//##############################################################################
// Inc
//##############################################################################

#include "config.h"

//##############################################################################
// Var
//##############################################################################

class I2Cbb{
    public:
        I2Cbb(void);
        uchar EE_busy;
        uchar read_EE(uchar* buf, uint adrr, uint len);
        uchar write_EE(uchar* buf, uint adrr, uchar len);
        uchar rd_char(char* val);
        uchar println(void);
        uchar print(char* str);
        uchar println(char* str);
        uchar print(const char* str); 
        uchar println(const char* str); 
        uchar print(char val);
        uchar println(char val);
        uchar print(uchar val); 
        uchar println(uchar val); 
        uchar print(uint val);
        uchar println(uint val);
        uchar print(ulong val);
        uchar println(ulong val);
        uchar print(int val);
        uchar println(int val);
    private:    
        void stop(void);
        void start(void);
        uchar pulse(void);
        uchar sbyte(uchar val);
        uchar rbyte(uchar nack);
        void send_byte(uchar val);
        uchar print_str(char* buf, uchar crlf);
        uchar print_char(char val, uchar crlf);
        uchar print_uchar(uchar val, uchar crlf);
        uchar print_uint(uint val, uchar crlf);
        uchar print_ulong(ulong val, uchar crlf);
        uchar print_int(int val, uchar crlf);
};


#endif	/* __I2C_BITBANG_H */
