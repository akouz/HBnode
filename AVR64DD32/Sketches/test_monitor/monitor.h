/*
 * File     monitor.h
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

#ifndef __MONITOR_H
#define __MONITOR_H

//##############################################################################
// Inc
//##############################################################################

#include "config.h"

//##############################################################################
// Def
//##############################################################################

#define MON_RX_BUF  0x80
#define MON_TX_BUF  0x80
#define MON_CMD     10

enum{
    // parameter types
    VAL_T_INT   = 1,
    VAL_T_UINT  = 2,
    // commands
    HELP_       = 0,
    REV_        = 1,
    SN_         = 2,
    NAME_       = 3,
    LOCATION_   = 4,
};

//##############################################################################
// Var
//##############################################################################

struct param_struct{
    ulong val[4];
    uchar val_t[4]; // type: 1 - decimal, 2 - hex
    uchar val_cnt;
    char  str[0x22];
    char  cmd[0x22];
};

class Mon{
    public:
        Mon(void);
        uchar   Rx(void);           // receive chars to buffer
        void    print_rev(void);
        void    print_sn(void);
        uchar   print_unit_name(void);
        uchar   print_unit_location(void);
        uchar   parse(void);        // parse buffer
        uchar   exe(void);          // execute command
        struct  param_struct param;    
        int     sn;                 // device S/N
    private:
        void  reset_param(void);
        uchar push_param(ulong val, uchar val_t); 
        char  find_cmd(void);  
        void  rdwr_sn(void);        // set or read S/N
        void  rdwr_str(uchar ii);   // set or read device name or location
        void  parse_notblank_char(char cc, char* buf);
        void  parse_blank_char(char* buf);
        slong dval = 0;             // decimal number
        ulong hval = 0;             // hex number
        char  str[2][22];           // name, location
        char  rxbuf[MON_RX_BUF];
        uchar rxbuf_len;
        char  txbuf[MON_TX_BUF];
        uchar txbuf_len;
        uchar cr_lf;
};        

extern I2Cbb i2cbb;
extern Mon mon;

#endif /* __MONITOR_H */
