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
};

//##############################################################################
// Var
//##############################################################################

// char buf[0x20];

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
    {""},    
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
    i2cbb.read_EE((uchar*)node.name_str, EE_NAME_STR, MAX_SSTR);    // string
    node.name_str[MAX_SSTR-1] = 0;                                  // ensure
    if ((node.name_str[0] < ' ') || (node.name_str[0] >= 0x80))     // if first char is not printable ASCII
    {
        node.name_str[0] = 0;       // then make string blank
    }
    i2cbb.read_EE((uchar*)node.location_str, EE_LOCATION_STR, MAX_LSTR);    // string
    node.location_str[MAX_LSTR-1] = 0;                                      // ensure
    if ((node.location_str[0] < ' ') || (node.location_str[0] >= 0x80))     // if first char is not printable ASCII
    {
        node.location_str[0] = 0;   // then make string blank
    }
    i2cbb.read_EE((uchar*)node.descr_str, EE_DESCR_STR, MAX_LSTR);      // string
    node.descr_str[MAX_LSTR-1] = 0;                                     // ensure
    if ((node.descr_str[0] < ' ') || (node.descr_str[0] >= 0x80))       // if first char is not printable ASCII
    {
        node.descr_str[0] = 0;       // then make string blank
    }
}
// =============================================
// Print project name, sketch file name and revision
// =============================================
void Mon::print_rev(void)
{
    i2cbb.print("Project ");
    i2cbb.print_in_quotes(PROJECT_NAME);
    i2cbb.print(", module ");
    i2cbb.print_in_quotes(ARDUINO_MODULE);
    i2cbb.print(", hw rev ");
    i2cbb.print(HW_REV_MAJ);
    i2cbb.print('.');
    i2cbb.println(HW_REV_MIN);
    // -----------------------
    i2cbb.print("Sketch ");
    i2cbb.print_in_quotes(SKETCH_NAME);
    i2cbb.print(", rev ");
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
    if ((node.sn[1] < 0 ) || (node.sn[0] < 0 ) || (node.SN == 0))
    {
        i2cbb.println("S/N not assigned");    
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
        i2cbb.println(str);
    }
}
// =============================================
// Print nodeID
// =============================================
void Mon::print_id(void)
{
    char str[0x20];
    sprintf(str,"nodeID 0x%04X", node.ID);        
    i2cbb.println(str);
}    
// =============================================
// Print strings
// =============================================
uchar Mon::print_node_name(void)
{
    if ((node.name_str[0] >= ' ') && ((uchar)node.name_str[0] < 0x80)) // first char must be printable ASCII
    {
        i2cbb.print("Node name: ");
        i2cbb.println(node.name_str); 
        return OK;
    }    
    else
    {
        i2cbb.println("Node name not assigned");   
        return ERR;
    }
}
// =============================================
uchar Mon::print_node_location(void)
{
    if ((node.location_str[0] >= ' ') && ((uchar)node.location_str[0] < 0x80)) // first char must be printable ASCII
    {
        i2cbb.print("Location: ");
        i2cbb.println(node.location_str); 
        return OK;
    }    
    else
    {
        i2cbb.println("Location not specified");        
        return ERR;
    }
}
// =============================================
uchar Mon::print_node_descr(void)
{
    if ((node.descr_str[0] >= ' ') && ((uchar)node.descr_str[0] < 0x80)) // first char must be printable ASCII
    {
        i2cbb.print("Description: ");
        i2cbb.println(node.descr_str); 
        return OK;
    }    
    else
    {
        i2cbb.println("No description");        
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
                    i2cbb.print(cc);
                }
                else if (cc == CHAR_BS)
                {
                    i2cbb.print(' ');
                }
            }
            else if ((cc >= ' ') && (this->rxbuf_len < MON_RX_BUF))
            {
                this->rxbuf[this->rxbuf_len++] = cc;
                i2cbb.print(cc);
            }
            if ((this->rxbuf_len >= MON_RX_BUF-2) || (cc == CHAR_CR) || (cc == CHAR_LF) || (cc == 0))
            {
                this->rxbuf[this->rxbuf_len++] = ' ';
                this->rxbuf[this->rxbuf_len] = 0;
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
    i2cbb.print(res);
    i2cbb.print(" words,");
    if (this->param.str)
    {        
        i2cbb.print((uint)this->param.str);
        i2cbb.print(" str='");
        i2cbb.print((char*)this->param.str);
        i2cbb.print("', ");
    }
    else
    {
        i2cbb.print(" str='' ");
    }
    if (this->param.cmd)
    {
        i2cbb.print((uint)this->param.cmd);
        i2cbb.print(" cmd='");
        i2cbb.print((char*)this->param.cmd);
        i2cbb.println("'");
    }        
    else
    {
        i2cbb.print((uint)this->rxbuf);
        i2cbb.print(", rxbuf_len=");
        i2cbb.print(this->rxbuf_len);
        i2cbb.println(", cmd=''");
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
        if ((node.sn[1] == -1) || (node.SN == 0) || (this->param.val[0] == 0)) // if SN is blank or if SN should be erased
        {
            if (i2cbb.EE_busy)
            {
                print_EE_busy();   
                return;
            }
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
            i2cbb.print("S/N ");
            if (node.SN)
            {
                if (node.sn[1])
                {
                    i2cbb.print((int)node.sn[1]);
                    i2cbb.print('.');
                }                
                i2cbb.print((int)node.sn[0]);
                i2cbb.println(" stored"); 
            }
            else
            {
                i2cbb.println("erased"); 
            }
        }
        else
        {
            i2cbb.println("Ignored, current S/N is not blank");
            i2cbb.println("To erase S/N set it to 0");
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
            if (i2cbb.EE_busy)
            {
                print_EE_busy();   
                return;
            }
            node.ID = (uint)this->param.val[0];         // use first param as nodeID
            buf[0] = (uchar)(this->param.val[0] >> 8);  // MSB goes first
            buf[1] = (uchar)this->param.val[0];         // LSB
            i2cbb.write_EE(buf, EE_nodeID, 2);          // store nodeID           
            i2cbb.print("nodeID ");
            if (node.ID)
            {
                i2cbb.print("0x");
                i2cbb.print(node.ID);
                i2cbb.println(" stored"); 
            }
            else
            {
                i2cbb.println("erased"); 
            }
        }
        else
        {
            i2cbb.println("Ignored, current nodeID is not blank");
            i2cbb.println("To erase nodeID set it to 0");
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
        i2cbb.print("==> ERROR: wrong params");
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
        if ((str[0] == 0) || ((this->param.str[0] == '-') && (this->param.str[1] == 0))) 
        {
            if (i2cbb.EE_busy)
            {
                print_EE_busy();   
                return;
            }
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
                i2cbb.println("String erased");
            }
            else
            {
                i2cbb.print("String '");
                i2cbb.print(this->param.str);
                i2cbb.println("' stored");
            }
        }
        else // string not erased
        {
            i2cbb.println("Ignored, current string is not blank");
            i2cbb.print("To erase string type '-");
            switch(ii)
            {
            case NAME_: 
                i2cbb.println(" name'"); 
                break;
            case LOCATION_: 
                i2cbb.println(" location'"); 
                break;
            case DESCR_: 
                i2cbb.println(" descr'"); 
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
// Find command
// =============================================
char Mon::find_cmd(void)
{
    if (this->param.cmd)
    {
        for (uchar i=0; i<MON_CMD; i++)
        {
            if (strcmp(this->param.cmd, cmd_list[i]) == 0)
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
            case ID_:
                this->rdwr_id();
                break;
            case NAME_:
            case LOCATION_:
            case DESCR_:
                this->rdwr_str(cmd);
                break;
            default:
                i2cbb.println("REV, [n] SN,  [n] nodeID, [txt] NAME, [txt] LOCATION, [txt] DESCR"); // list of commands
                break;
        }
    }
    else
    {
        i2cbb.print("==> ERROR: unknown command '");
        if (this->param.cmd)
        {
            i2cbb.print(this->param.cmd);
        }
        i2cbb.println("'");
    }
    i2cbb.print("MON> ");
    this->reset_param();
    return res;
}    

/* EOF */