/*
 * File     HBmonitor.cpp
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

#include "HBmonitor.h"
#include "HBcmd.h"
#include "HBmqtt.h"
#include "HBcipher.h"
#include "i2c_bitbang.h"

//##############################################################################

//##############################################################################
enum{
    // parameter types
    VAL_T_INT   = 1,
    VAL_T_UINT  = 2,
    // commands
    HELP_       = 0,
    REV_        = 1,
    SN_         = 2,
    ID_         = 3,
    NAME_       = 4,
    LOCATION_   = 5,
    DESCR_      = 6,
    ALLOW_      = 7, 
    EERD_       = 8,
    EEWR_       = 9,
    EECLR_      = 10,
    EECRC_      = 11,
    RESET_      = 12,
    TOPIC_      = 13,
};

//##############################################################################
// Var
//##############################################################################

const uchar ee_pattern[4] PROGMEM = { 0x55, 0xAA, 0xC3, 0x3C };

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
    {"NODEID"},    
    {"NAME"},    
    {"LOCATION"},    
    {"DESCR"},    
    {"ALLOW"},    
    {"EERD"},    
    {"EEWR"},    
    {"EECLR"},    
    {"EECRC"},    
    {"RESET"},    
    {"TOPIC"},    
    {""},    
    {""},    
}; 

Mon mon;

//##############################################################################
// Func
//##############################################################################

// =============================================
// Constructor
// =============================================
Mon::Mon(void)
{
    this->begin();
}
// =============================================
// Init monitor
// =============================================
void Mon::begin(void)
{
    uchar buf[0x20];    
    uchar offs;
    this->reset_param();
    this->rxbuf_len = 0;
    this->txbuf_len = 0;
    this->cr_lf = 0;
    // -----------------------------
    // sketch signature
    // -----------------------------
    i2cbb.read_EE(buf, EE_BOOT_TABLE, 8); 
    node.boot_patt = 0;
    if ((buf[0] == 0x55) && (buf[1] = 0xAA))
    {
        if ((buf[2] == 0xC3) && (buf[3] = 0x3C))
            node.boot_patt = 1;
    }
    node.prog_len = 0x100*buf[4] + buf[5];
    node.prog_crc = 0x100*buf[6] + buf[7];
    // -----------------------------
    // digital values
    // -----------------------------
    i2cbb.read_EE(buf, EE_SEED, 0x20); 
    for (uchar i=0; i<2; i++) // SEED offset 0
    {
        node.seed <<= 8;
        node.seed |= buf[i];
    }
    offs = EE_PUP_CNT - EE_SEED;
    for (uchar i=0; i<2; i++) 
    {
        node.pup_cnt <<= 8;
        node.pup_cnt |= buf[i+offs];
    }
    offs = EE_SN - EE_SEED;
    for (uchar i=0; i<4; i++)
    {
        node.SN <<= 8;
        node.SN |= buf[i+offs];
    }
    offs = EE_nodeID - EE_SEED;
    for (uchar i=0; i<2; i++)
    {
        node.ID <<= 8;
        node.ID |= buf[i+offs];
    }
    offs = EE_TZ - EE_SEED;
    for (uchar i=0; i<2; i++)
    {
        node.tz <<= 8;
        node.tz |= buf[i+offs];
    }
    if (node.tz == -1)
    {
        node.tz = 0;     // UTC
    }
    // -----------------------------
    // strings
    // -----------------------------
    i2cbb.read_EE((uchar*)node.name_str, EE_NAME_STR, MAX_SSTR);        // string
    node.name_str[MAX_SSTR-1] = 0;                                      // ensure
    //-------------------------
    i2cbb.read_EE((uchar*)node.location_str, EE_LOCATION_STR, MAX_LSTR);        // string
    node.location_str[MAX_LSTR-1] = 0;                                          // ensure
    //-------------------------
    i2cbb.read_EE((uchar*)node.descr_str, EE_DESCR_STR, MAX_LSTR);      // string
    node.descr_str[MAX_LSTR-1] = 0;                                     // ensure
}
// =============================================
// Print project name, sketch file name and revision
// =============================================
void Mon::print_rev(void)
{
    PRINT("Module ");
    i2cbb.print_in_quotes(ARDUINO_MODULE);
    PRINT(", hw rev ");
    PRINT(HW_REV_MAJ);
    PRINT('.');
    PRINTLN(HW_REV_MIN);
    // -----------------------
    PRINT("Sketch ");
    i2cbb.print_in_quotes(SKETCH_NAME);
    PRINT(", rev ");
    PRINT(SW_REV_MAJ);
    PRINT('.');
    PRINT(SW_REV_MIN);
    PRINT(", signature ");
    PRINT(node.prog_len);
    PRINT('.');
    PRINTLN(node.prog_crc);
}
// =============================================
// Print S/N
// =============================================
void Mon::print_sn(void)
{
    char str[0x20];
    if ((node.sn[1] < 0 ) || (node.sn[0] < 0 ) || (node.SN == 0))
    {
        PRINTLN("S/N not assigned");    
    }
    else
    {
        if (node.sn[1] == 0)
        {
            sprintf(str,"Node S/N %d", node.sn[0]);        
        }
        else
        {
            sprintf(str,"Node S/N %d.%04d", node.sn[1],  node.sn[0]);        
        }
        PRINTLN(str);
    }
}
// =============================================
// Print nodeID
// =============================================
void Mon::print_id(void)
{
    char str[0x20];
    sprintf(str,"nodeID 0x%04X", node.ID);        
    PRINTLN(str);
}    
// =============================================
// Print strings
// =============================================
uchar Mon::print_node_name(void)
{
    if (vld_char(node.name_str[0]))   
    {
        PRINT("Node name: ");
        PRINTLN(node.name_str); 
        return OK;
    }    
    else
    {
        PRINTLN("Node name not assigned");   
        return ERR;
    }
}
// =============================================
uchar Mon::print_node_location(void)
{
    if (vld_char(node.location_str[0])) // first char must be printable ASCII
    {
        PRINT("Location: ");
        PRINTLN(node.location_str); 
        return OK;
    }    
    else
    {
        PRINTLN("Location not specified");        
        return ERR;
    }
}
// =============================================
uchar Mon::print_node_descr(void)
{
    if (vld_char(node.descr_str[0])) // first char must be printable ASCII
    {
        PRINT("Description: ");
        PRINTLN(node.descr_str); 
        return OK;
    }    
    else
    {
        PRINT((uchar)node.name_str[0]);
        PRINT('-');
        PRINTLN("No description");        
        return ERR;
    }
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
        if (OK == res) // char received
        {
            if (cc == CHAR_TAB) 
            {
                cc = ' ';
            }
            if ((cc == CHAR_BS) || (cc == CHAR_DEL)) // backspace
            {
                if (this->rxbuf_len)
                {
                    this->rxbuf[this->rxbuf_len--] = 0; 
                    PRINT(cc);
                }
                else if (cc == CHAR_BS)
                {
                    PRINT(' ');
                }
            }
            else if ((cc >= ' ') && (this->rxbuf_len < MON_RX_BUF))
            {
                this->rxbuf[this->rxbuf_len++] = cc;
                PRINT(cc);
            }
            if ((this->rxbuf_len >= MON_RX_BUF-2) || (cc == CHAR_CR) || (cc == CHAR_LF) || (cc == 0))
            {
                this->rxbuf[this->rxbuf_len++] = ' ';
                this->rxbuf[this->rxbuf_len] = 0;
                this->cr_lf = 1;                
                PRINTLN();
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
    //this->param.cmd[0] = 0;
    //this->param.str[0] = 0;
    this->param.cmd = NULL;
    this->param.str = NULL;
    this->param.str_end = 0;
    this->param.cmd_start = 0;
    this->param.cmd_end = 0;
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
    
    if (pstate.cnt < 0x1F)
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
    uchar res = 0; // number of words
    char cc, buf[0x20]; 
    uint i;
    pstate.flag.all = 0;
    pstate.cnt = 0;
    buf[0] = 0;
    if ((this->cr_lf) && (this->rxbuf_len))
    {
        this->cr_lf = 0;
        this->reset_param();
        // -----------------------
        // parse Rx buffer
        // -----------------------
        for (i=0; i<this->rxbuf_len; i++)
        {            
            cc = this->rxbuf[i];
            if (cc > ' ') 
            {
                if (pstate.cnt == 0)
                {
                    res++;                      // a new word starts
                    this->param.str_end = this->param.cmd_end;  
                    this->param.cmd_start = i;  // probably command word starts here
                }
                this->param.cmd_end = i;
                this->parse_notblank_char(cc, buf);
            }
            else if (pstate.cnt)
            {
                this->parse_blank_char(buf);
            }
        } // for i
        // -----------------------
        // prepare cmd and param str
        // -----------------------
        if (res)    // at least 1 word parsed, last word is always a command
        {
            this->param.cmd = (char*)this->rxbuf + this->param.cmd_start;   // command string
            this->rxbuf[this->param.cmd_end+1] = 0;                         // terminate command
            upper_str(this->param.cmd);                 // because command is not case sensitive    
            if (res > 1)                                // if param string exists
            {
                this->param.str = (char*)this->rxbuf;   // then param string starts from the beginning of the Rx buffer
                this->rxbuf[this->param.str_end+1] = 0; // terminate string
            }
        }
    }
/* debug
    PRINT(res);
    PRINT(" words,");
    if (this->param.str)
    {        
        PRINT((uint)this->param.str);
        PRINT(" str='");
        PRINT((char*)this->param.str);
        PRINT("', ");
    }
    else
    {
        PRINT(" str='' ");
    }
    if (this->param.cmd)
    {
        PRINT((uint)this->param.cmd);
        PRINT(" cmd='");
        PRINT((char*)this->param.cmd);
        PRINTLN("'");
    }        
    else
    {
        PRINT((uint)this->rxbuf);
        PRINT(", rxbuf_len=");
        PRINT(this->rxbuf_len);
        PRINTLN(", cmd=''");
    }
  */
    this->rxbuf_len = 0;    // reset Rx buffer
    return res;             // number of words in the string
}
// =============================================
// If EEPROM is busy
// =============================================
void print_EE_busy(void)
{
    PRINTLN("==> ERROR: EEPROM busy, try again later");
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
        if ((node.sn[1] == -1) || (node.SN == 0) || (this->param.val[0] == 0)) // if SN is blank or if SN should be erased
        {
            if ((int)this->param.val[0] < 0)
            {
                this->param.val[0] = 0;
            }
            node.sn[0] = (uint)this->param.val[0];      // S/N LSB 
            node.sn[1] = 0;
            if (param.val_cnt > 1)
            {
                node.sn[1] = (uint)this->param.val[1];  // S/N MSB (batch)
            } 
            for (i=0; i<4; i++) // for each byte
            {
                buf[i] = (uchar)(node.SN >> (8*(3-i))); // MSB goes first
            }
            i2cbb.write_EE(buf, EE_SN, 4); 
            PRINT("S/N ");
            if (node.SN)
            {
                if (node.sn[1])
                {
                    PRINT((int)node.sn[1]);
                    PRINT('.');
                }                
                PRINT((int)node.sn[0]);
                PRINTLN(" stored"); 
            }
            else
            {
                PRINTLN("erased"); 
            }
        }
        else
        {
            PRINTLN("Ignored, current S/N is not blank");
            PRINTLN("To erase S/N set it to 0");
        }
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
// Set or read nodeID
// =============================================
void Mon::rdwr_id(void)
{
    uchar buf[4];
    // ----------------------
    // write nodeID
    // ----------------------
    if (this->param.val_cnt > 0) 
    {
        if ((node.ID == 0) || (node.ID == 0xFFFF) || (this->param.val[0] == 0))  // if ID is blank or if ID should be erased
        {
            node.ID = (uint)this->param.val[0];         // use first param as nodeID
            buf[0] = (uchar)(this->param.val[0] >> 8);  // MSB goes first
            buf[1] = (uchar)this->param.val[0];         // LSB
            i2cbb.write_EE(buf, EE_nodeID, 2);          // store nodeID           
            PRINT("nodeID ");
            if (node.ID)
            {
                PRINT("0x");
                PRINT(node.ID);
                PRINTLN(" stored"); 
            }
            else
            {
                PRINTLN("erased"); 
            }
        }
        else
        {
            PRINTLN("Ignored, current nodeID is not blank");
            PRINTLN("To erase nodeID set it to 0");
        }
    }
    // ----------------------
    // print nodeID
    // ----------------------
    else 
    {
        this->print_id();
    }    
}    
// =============================================
// Set or read name, location or description
// =============================================
void Mon::rdwr_str(uchar ii)
{
    uint EEaddr;
    char* str;
    uchar len, lim, idx;
    switch (ii)
    {
    case NAME_:
        lim = MAX_SSTR;
        str = node.name_str; 
        break;
    case LOCATION_:
        lim = MAX_LSTR;
        str = node.location_str; 
        break;
    case DESCR_:
        lim = MAX_LSTR;
        str = node.descr_str; 
        break;
    default:
        PRINT("==> ERROR: wrong params");
        return;             // error
        break;
    }
    idx = ii - NAME_;                   // string index
    EEaddr =  EE_NAME_STR + 0x40*idx;   // EEPROM address
    // ----------------------
    // write string
    // ----------------------
    if (this->param.str)    
    {       
        if (str[0] == 0xFF) 
        {
            str[0] = 0;
        }
        if ((str[0] == 0) || ((this->param.str[0] == '-') && (this->param.str[1] == 0))) 
        {
            len = (uchar)strlen(this->param.str);  
            len = (len >= lim)? lim - 1 : len;
            if (this->param.str[0] == '-')  // '-' clears string
            {
                len = 0;
                this->param.str[0] = 0; // string is blank
            }
            this->param.str[len+1] = 0;
            i2cbb.write_EE((uchar*)this->param.str, EEaddr, len+1); 
            strcpy(str, this->param.str);
            if (this->param.str[0] == 0)
            {
                PRINTLN("String erased");
            }
            else
            {
                PRINT("String '");
                PRINT(this->param.str);
                PRINTLN("' stored");
            }
        }
        else // string not erased
        {
            PRINTLN("Ignored, current string is not blank");
            PRINT("To erase string type '-");
            switch(ii)
            {
            case NAME_: 
                PRINTLN(" name'"); 
                break;
            case LOCATION_: 
                PRINTLN(" location'"); 
                break;
            case DESCR_: 
                PRINTLN(" descr'"); 
                break;
            default: 
                break;
            }
        }
    }  
    // ----------------------
    // read string
    // ----------------------
    else
    {
        switch(ii)
        {
        case NAME_: 
            i2cbb.read_EE((uchar *)node.name_str, EE_NAME_STR, MAX_SSTR); 
            node.name_str[MAX_SSTR-1] = 0;
            this->print_node_name();
            break;
        case LOCATION_: 
            i2cbb.read_EE((uchar *)node.location_str, EE_LOCATION_STR, MAX_LSTR); 
            node.location_str[MAX_LSTR-1] = 0;
            this->print_node_location();
            break;
        case DESCR_: 
            i2cbb.read_EE((uchar *)node.descr_str, EE_DESCR_STR, MAX_LSTR); 
            node.descr_str[MAX_LSTR-1] = 0;
            this->print_node_descr();
            break;
        default:
            break;                              
        }
    }  
}
// =============================================
// Read/write node.allow
// =============================================
void Mon::HBallow(void)
{
    // ----------------
    // write
    // ----------------
    if (this->param.val_cnt)
    {
        node.allow.all = (uint)this->param.val[0]; 
    }
    // ----------------
    // read
    // ----------------
    PRINT("node.allow = 0x");
    PRINT(node.allow.all);
    PRINT(", HBcipher.valid = ");
    if (HBcipher.valid) 
    {
        PRINTLN("true");
    }
    else
    {
        PRINTLN("false"); 
    }
}
// =============================================
// Read EEPROM
// =============================================
void Mon::EErd(void)
{
    static uint addr = 0;
    uchar cnt = 1;
    uchar buf[0x80];
    if (this->param.val_cnt)
    {
        addr = (uint)this->param.val[0];
    }
    if (this->param.val_cnt > 1)
    {
        cnt = (uchar)this->param.val[1];
        if (cnt>0x80)
        {
            cnt = 0x80;        
        }            
    }
    i2cbb.read_EE(buf, addr, cnt);
    for (uchar i=0; i<cnt; i++)
    {  
        if (((addr + i) & 0x0F) == 0)
        {
            PRINTLN();
            PRINT((uint)(addr + i));
            PRINT(": ");
        }
        if (((addr + i) & 0xFFF0) == EE_XTEA_KEY) // do not print EEPROM key
        {
            PRINT("XX");
        }
        else
        {
            PRINT(buf[i]);
        }        
        PRINT(' ');
        if ((i & 0x0F) == 7)
        {
            PRINT(' ');
        }
    }
    PRINTLN();
 }    
// =============================================
// Write to EEPROM
// =============================================
void Mon::EEwr(void)
{
    if (this->param.val_cnt > 1)
    {
        uint addr = (uint)this->param.val[0];
        uchar bb = (uchar)this->param.val[1];
        i2cbb.write_EE(&bb, addr, 1);
        PRINTLN("Done");
    }
    else
    {
        PRINTLN(" Error: no data, no address");
    }
}    
// =============================================
// Clear EEPROM
// =============================================
void Mon::EEclr(void)
{
    if (this->param.val_cnt > 1)
    {
        uint addr = (uint)this->param.val[0];
        uchar len = (uchar)this->param.val[1];
        if (len > 0x40)
        {
            len = 0x40;
        }    
        PRINT("Clearing 0x");
        PRINT(len);
        PRINT(" bytes starting at addr 0x");
        PRINT(addr);
        if (len)    
        {
            i2cbb.write_EE(NULL, addr, len);
        }    
        PRINTLN(". Done");
    }
    else
    {
        PRINTLN(" Error: no len, no address");
    }
}    
// =============================================
// Calculate EEPROM crc
// =============================================
void Mon::EEcrc(void)
{
    uint crc = 0xFFFF;
    uint rcrc, codelen;
    uchar buf[0x10];
    i2cbb.read_EE(buf, 0x0010, 8);
    buf[8] = 0;
    codelen = ((uint)buf[4] << 8) + buf[5];
    rcrc = ((uint)buf[6] << 8) + buf[7];
    // i2cbb.println(buf, 8);
    PRINT(codelen);
    PRINT(' ');
    PRINT(rcrc);
    PRINT('-');
    crc = i2cbb.crc_EE(codelen);
    PRINTLN(crc);
}
// =============================================
// Soft reset
// =============================================
void Mon::reset(void)
{
    uchar res = ERR;
    if (this->param.val_cnt == 1)
    {
        switch((uchar)this->param.val[0])
        {
        case 1:
            PRINTLN("OK, wait...");
            res = OK;
            node.rst_cnt = 2;           // reset in 20 ms
            break;
        case 2:
            PRINTLN("OK, wait...");
            res = OK;
            CCP = IOREG;                // unlock
            RSTCTRL.SWRR = 1;           // software reset
            break;
        case 3:
            PRINTLN("Done");
            res = OK;
            i2cbb.write_EE((uchar*)ee_pattern, 0x10, 4);
            break;
        default:
            break;
        }
    }
    if (res != OK)
    {
        PRINTLN("Invalid command");
    }
}  
// =============================================
// Topic
// =============================================
void Mon::topic(void)
{
    char buf[0x40]; 
    uchar done = 0;   
    if (this->param.val_cnt == 0)
    {
        for (uchar i=0; i<MAX_TOPIC; i++)
        {
            if (HBmqtt.flag[i].topic_name_valid)
            {
                done = 1;
                PRINT(i);
                PRINT(": ");
                HBmqtt.copy_topic_name(i, buf);
                PRINT(buf);
                if (HBmqtt.flag[i].topic_valid)    
                {
                    PRINT(", TopicId ");
                    if (ownTopicId[i] < 0x8000)
                        PRINT((int)ownTopicId[i]);
                    else    
                        PRINT(ownTopicId[i]);
                    if (HBmqtt.flag[i].val_type) // if value valid
                    {
                        PRINT(", val ");
                        HBmqtt.print_own_val(i, buf);  
                        PRINTLN(buf);  
                    }                    
                    else
                        PRINTLN();
                }
                else
                    PRINTLN();
            }
        }
        if (done == 0)
        {
            PRINTLN("No valid topics");
        }
        PRINT("MON> ");
    }
    else if (this->param.val_cnt == 1)
    {
        HBmqtt.validate_topics();
    }
}        
     
// =============================================
// Find command
// =============================================
char Mon::find_cmd(void)
{
    char def[2] = {'?', 0};
    if ((this->param.cmd == NULL) || (this->param.cmd[0] == 0))
    {
        this->param.cmd = def; // default
    }
    for (uchar i=0; i<MON_CMD; i++)
    {
        if (strcmp(this->param.cmd, cmd_list[i]) == 0)
            return i;
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
            case ID_:
                this->rdwr_id();
                break;
            case NAME_:
            case LOCATION_:
            case DESCR_:
                this->rdwr_str(cmd);
                break;
            case ALLOW_:
                this->HBallow();
                break;
            case EERD_:
                this->EErd();
                break;
            case EEWR_:
                this->EEwr();
                break;
            case EECLR_:
                this->EEclr();
                break;
            case EECRC_:
                this->EEcrc();
                break;
            case RESET_:
                this->reset();
                break;
            case TOPIC_:
                this->topic();
                break;
            default:
                PRINTLN("REV, [n] SN, [n] nodeID, [txt] NAME, [txt] LOCATION, [txt] DESCR, [addr, len] EERD, [addr, len] EECLR"); // list of commands
                break;
        }
    }
    else
    {
        PRINT("==> ERROR: unknown command '");
        if (this->param.cmd)
        {
            PRINT(this->param.cmd);
        }
        PRINTLN("'");
    }
    PRINT("MON> ");
    this->reset_param();
    return res;
}    

/* EOF */