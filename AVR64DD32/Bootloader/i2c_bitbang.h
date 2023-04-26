/*
 * File     i2c_bitbang.h
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

#ifndef __I2C_BITBANG_H
#define	__I2C_BITBANG_H

//##############################################################################
// Inc
//##############################################################################

#include "config.h"

//##############################################################################
// Func
//##############################################################################

void init_i2c(void);
uchar i2c_read(uchar* buf, uint adr, uint len);
void i2c_write(uchar* buf, uint adr, uchar len);

#ifdef DEBUG_BOOT
uchar i2c_send(uint addrr, uchar* buf, uint len);
#endif
         
#endif	/* __I2C_BITBANG_H */

