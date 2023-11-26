/*
 * File     HBmonitor.h
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

#ifndef __HB_MONITOR_H
#define __HB_MONITOR_H

//##############################################################################
// Inc
//##############################################################################

#include "HBcommon.h"

//##############################################################################
// Def
//##############################################################################

#define MON_RX_BUF  0x60
#define MON_TX_BUF  0x80
#define MON_CMD     0x10


//##############################################################################
// Var
//##############################################################################

struct param_struct{
    ulong val[4];       // entered values
    uchar val_t[4];     // type: 1 - decimal, 2 - hex
    uchar val_cnt;
    uchar str_end;
    uchar cmd_start;
    uchar cmd_end;
    char* cmd;          // entered command
    char* str;          // parameter string
};

class Mon{
    public:
        Mon(void);
        void    begin(void);
        void    print_rev(void);
        void    print_sn(void);
        void    print_id(void);
        uchar   print_node_name(void);
        uchar   print_node_location(void);
        uchar   print_node_descr(void);
        void    HBallow(void);      // set or read node.allow 
        uchar   Rx(void);           // receive chars to buffer
        uchar   parse(void);        // parse buffer
        uchar   exe(void);          // execute command
        struct  param_struct param;    
    private:
        void  reset_param(void);
        uchar push_param(ulong val, uchar val_t); 
        char  find_cmd(void);  
        void  rdwr_sn(void);        // set or read S/N
        void  rdwr_id(void);        // set or read nodeID
        void  rdwr_str(uchar ii);   // set or read device name or location
        void  EErd(void);
        void  EEwr(void);
        void  EEclr(void);
        void  EEcrc(void);
        void  reset(void);
        void  topic(void);
        void  parse_notblank_char(char cc, char* buf);
        void  parse_blank_char(char* buf);
        void  rd_name(void);
        void  rd_location(void);
        void  rd_descr(void);
        slong dval = 0;             // decimal number
        ulong hval = 0;             // hex number
        char  rxbuf[MON_RX_BUF];
        uchar rxbuf_len;
        char  txbuf[MON_TX_BUF];
        uchar txbuf_len;
        uchar cr_lf;
};        

extern Mon mon;

#endif /* __MONITOR_H */
