/*
 * File     monitor.cpp
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
 * ----------------------------------------------
 * 
 * Monitor communicates to hyperterminal via testjig based on FT200XD
 * 
 */

//##############################################################################
// Inc
//##############################################################################

#include "monitor.h"

//##############################################################################
// Var
//##############################################################################

I2Cbb i2cbb;          // I2C bit-bang to access FT200XD test jig and EEPROM
Mon mon;

struct{
    uchar cnt;    
    char dig;
    char hex;
    union{
        uchar all;
        struct{
            unsigned    not_decimal : 1;
            unsigned    not_hex     : 1;
            unsigned    minus       : 1;
            unsigned                : 5;        
        };    
    }flag;
}pstate;

const char cmd_list[MON_CMD][10] = {
    {"?"},    
    {"REV"},
    {"SN"},    
    {"NAME"},    
    {"LOCATION"},    
    {"COMMENT"},    
    {"GUID"},    
    {""},    
    {""},    
    {""},    
}; 


//##############################################################################
// Func
//##############################################################################

// =============================================
// Constructor
// =============================================
Mon::Mon(void)
{
    uchar buf[4];
    uint addr = 0x20; // strings offset 0x20
    this->reset_param();
    this->rxbuf_len = 0;
    this->txbuf_len = 0;
    this->cr_lf = 0;
    i2cbb.read_EE(buf, 4, 4);
    for (uchar i=0; i<4; i++)
    {
        this->sn <<= 8;
        this->sn |= buf[i];
    }
    for (uchar i=0; i<2; i++)
    {
        i2cbb.read_EE((uchar*)this->str[i], addr, 0x20);     
        this->str[i][0x20] = 0;
        addr += 0x20;
    }
}
// =============================================
// Print sketch file name and revision
// =============================================
void Mon::print_rev(void)
{
    i2cbb.print("Sketch ");
    i2cbb.print('"');
    i2cbb.print(SKETCH_NAME);
    i2cbb.print('"');
    i2cbb.print(" rev ");
    i2cbb.print(SW_REV_MAJ);
    i2cbb.print('.');
    i2cbb.println(SW_REV_MIN);
}
// =============================================
// Print S/N
// =============================================
void Mon::print_sn(void)
{
    char str[0x20];
    if ((this->sn == 0x7FFF) || (this->sn == 0))
    {
        i2cbb.println("No S/N");    
    }
    else
    {
        sprintf(str,"Device S/N %d",this->sn);        
        i2cbb.println(str);
    }
}
// =============================================
// Print unit strings
// =============================================
uchar Mon::print_unit_name(void)
{
    if ((this->str[0][0]) && ((uchar)this->str[0][0] < 0xFF))
    {
        i2cbb.print("Unit name: ");
        i2cbb.println(this->str[0]); 
        return OK;
    }    
    return ERR;
}
// =============================================
uchar Mon::print_unit_location(void)
{
    if ((this->str[1][0]) && ((uchar)this->str[1][0] < 0xFF))
    {
        i2cbb.print("Unit location: ");
        i2cbb.println(this->str[1]); 
        return OK;
    }    
    return ERR;
}
// =============================================
// Read FT200XD to rx buffer
// =============================================
uchar Mon::Rx(void)
{
    static char cc;
    uchar res = OK;
    while (OK == res)
    {
        res = i2cbb.rd_char(&cc);
        if (OK == res)
        {
            if (cc == 9) // TAB
                cc = ' ';
            if ((cc >= ' ') && (this->rxbuf_len < MON_RX_BUF))
            {
                this->rxbuf[this->rxbuf_len++] = cc;
                i2cbb.print(cc);
            }
            if ((this->rxbuf_len >= MON_RX_BUF) || (cc == CR) || (cc == LF) || (cc == 0))
            {
                this->cr_lf = 1;
                i2cbb.println();
                break;
            }
        }
    }
    return res; 
}
// =============================================
// Reset command parameters
// =============================================
void Mon::reset_param(void)
{
    this->param.val_cnt = 0;
    this->param.str[0] = 0;
    this->param.cmd[0] = 0;
}
// =============================================
// Store parameter
// =============================================
uchar Mon::push_param(ulong val, uchar val_t)
{
    uchar res = ERR;
    if (this->param.val_cnt < 4)
    {
        this->param.val_t[this->param.val_cnt] = val_t;
        this->param.val[this->param.val_cnt++] = val;
        res = OK;
    }
    return res;
}
// =============================================
// Char utils
// =============================================
char is_digit(char cc)
{
    return ((cc < '0') || (cc > '9'))? -1 : (cc - '0');
}
// =============================================
char is_hex(char cc)
{
    schar res = is_digit(cc);
    if (res >= 0)
        return res;
    else  if ((cc >= 'A') && (cc <= 'F'))
        return (cc - 'A' + 10);
    else
        return -1;    
}
// =============================================
char upper_case(char cc)
{
    if ((cc>='a') && (cc<='z'))
    {
        cc = cc - 'a' + 'A';
    }
    return cc;
}
// =============================================
void upper_str(char* str)
{
    for (uchar i=0; i<0x12; i++)
    {
        if (str[i] == 0)
            break;
        else if ((str[i]>='a') && (str[i]<='z'))
            str[i] = str[i] - 'a' + 'A';
    }
}
// =============================================
// Parse char
// =============================================
void Mon::parse_notblank_char(char cc, char* buf)
{
    if (pstate.cnt < 0x20)
    {
        buf[pstate.cnt++] = cc;
        buf[pstate.cnt] = 0;
    }    
    cc = upper_case(cc);
    pstate.dig = is_digit(cc);
    pstate.hex = is_hex(cc);
    if ((cc == '-') && (pstate.cnt == 1))
    {
        pstate.flag.minus = 1;
        pstate.flag.not_hex = 1;
    }
    else if ((pstate.flag.not_decimal == 0) && (pstate.dig >= 0))
    {
        this->dval = this->dval*10 + pstate.dig;
        if (pstate.flag.not_hex == 0)        
        {
            this->hval = (this->hval << 4) | pstate.dig;
        }
    }
    else if ((pstate.flag.not_hex == 0) && (pstate.hex >= 0))
    {
        pstate.flag.not_decimal = 1;
        this->hval = (this->hval << 4) | pstate.hex;
    }
    else if ((pstate.flag.not_hex == 0) && (buf[0] == '0') && (cc == 'X') && (pstate.cnt == 2)) // hex prefix 0X
    {
        pstate.flag.not_decimal = 1;
    }
    else
    {
        pstate.flag.not_decimal = 1;
        pstate.flag.not_hex = 1;
    }
}
// =============================================
void Mon::parse_blank_char(char* buf)
{
    strcpy(this->param.str, this->param.cmd);
    strcpy(this->param.cmd, buf);
    if ((pstate.flag.not_decimal == 0) && (pstate.flag.minus))
    {
        this->dval = -this->dval;
    }
    if (pstate.flag.not_decimal == 0)
    {
        this->push_param((ulong)this->dval, VAL_T_INT);
    }
    else if (pstate.flag.not_hex == 0)
    {
        this->push_param(this->hval, VAL_T_UINT);
    }
    this->dval = 0;
    this->hval = 0;
    pstate.flag.all = 0;     // word ends
    pstate.cnt = 0;    
}
// =============================================
// Parse rx buffer
// =============================================
uchar Mon::parse(void)
{
    static uchar res = 0;
    char cc; 
    uint i;
    pstate.flag.all = 0;
    pstate.cnt = 0;
    char buf[0x22];
    buf[0] = 0;
    if (this->cr_lf)
    {
        this->cr_lf = 0;
        if (this->rxbuf_len == 0)
        {
            return res;
        }
        this->reset_param();
        res = 0;
        for (i=0; i<this->rxbuf_len; i++)
        {            
            cc = this->rxbuf[i];
            if (cc > ' ') 
            {
                if (pstate.cnt == 0)
                    res++;              // a new word starts
                this->parse_notblank_char(cc, buf);
            }
            else if (pstate.cnt)
            {
                this->parse_blank_char(buf);
            }
        } // this->rxbuf_len
        if (pstate.cnt) // last word is always command
        {
            strcpy(this->param.str, this->param.cmd);
            strcpy(this->param.cmd, buf);
        }
        upper_str(this->param.cmd);     // command is not case sensitive    
        this->param.cmd[0x10] = 0;      // limit command length
        this->rxbuf_len = 0;            // reset Rx buffer
    }
    return res;     // number of words in the string
}
// =============================================
// If EEPROM is busy
// =============================================
void print_EE_busy(void)
{
    i2cbb.println("==> ERROR: EEPROM busy, try again later");
}
// =============================================
// Set or read S/N
// =============================================
void Mon::rdwr_sn(void)
{
    uchar buf[4], i;
    // ----------------------
    // write S/N
    // ----------------------
    if (this->param.val_cnt > 0) 
    {
        if (i2cbb.EE_busy)
        {
            print_EE_busy();   
            return;
        }
        this->sn = this->param.val[0]; // use first param as S/N
        for (i=0; i<4; i++)
        {
            buf[i] = (uchar)(this->param.val[0] >> 8*(3-i)); // MSB goes first
        }
        i2cbb.write_EE(buf, 4, 4); // S/N stored at addr 4
        this->sn &= 0x7FFF;        
        i2cbb.print("S/N ");
        i2cbb.print((int)this->sn);
        i2cbb.println(" stored"); 
    }
    // ----------------------
    // print S/N
    // ----------------------
    else 
    {
        this->print_sn();
    }    
}    
// =============================================
// Set or read name, location, comment
// =============================================
void Mon::rdwr_str(uchar ii)
{
    char res;
    uint addr;
    uchar len, idx;
    if ((ii < NAME_) || (ii > LOCATION_))
    {
        i2cbb.print("==> ERROR: wrong params");
        return;
    }
    idx = ii - NAME_;           // string index
    addr = 0x20 + 0x20*idx;     // strings offset 0x20, strings length 0x20
    // ----------------------
    // write string
    // ----------------------
    if (this->param.str[0])    
    {        
        if (i2cbb.EE_busy)
        {
            print_EE_busy();   
            return;
        }
        len = (uchar)strlen(this->param.str);   
        len = (len > 0x1F)? 0x1F : len; 
        if ((this->param.str[0] == '-') && (this->param.str[1] == 0)) // '-' clears string
        {
            len = 2;
            this->param.str[0] = 0; // string is blank
        }
        this->param.str[len] = 0;
        i2cbb.write_EE((uchar*)this->param.str, addr, len+1); 
        strcpy(this->str[idx], this->param.str);
        if (this->param.str[0] == 0)
        {
            i2cbb.println("String erased");
        }
        else
        {
            i2cbb.print("String ");
            i2cbb.print('"');
            i2cbb.print(this->param.str);
            i2cbb.print('"');
            i2cbb.println(" stored");
        }
    }  
    // ----------------------
    // read string
    // ----------------------
    else
    {
        i2cbb.read_EE((uchar *)this->str[idx], addr, 0x20); 
        this->str[idx][0x20] = 0; // ensure
        switch(ii)
        {
        case NAME_: 
            res = this->print_unit_name();
            break;
        case LOCATION_: 
            res = this->print_unit_location();
            break;
        default:
            break;                              
        }
        if (OK != res)
        {
            i2cbb.println("not defined or invalid");
        }
    }  
}
// =============================================
// Find command
// =============================================
char Mon::find_cmd(void)
{
    for (uchar i=0; i<MON_CMD; i++)
    {
        if (strcmp(this->param.cmd, cmd_list[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}    
// =============================================
// Execute command
// =============================================
uchar Mon::exe(void)
{
    uchar res = ERR;
    char cmd = this->find_cmd();
    if (cmd >= 0)
    {
        res = OK;
        switch (cmd)
        {
            case REV_:
                this->print_rev();
                break;
            case SN_:
                this->rdwr_sn();
                break;
            case NAME_:
            case LOCATION_:
                this->rdwr_str(cmd);
                break;
            default:
                i2cbb.println("REV, [n] SN, [txt] NAME, [txt] LOCATION"); // list of commands
                break;
        }
    }
    else
    {
        i2cbb.println("==> ERROR: unknown command");
    }
    i2cbb.print("MON> ");
    return res;
}    

/* EOF */