/*
 * File     common.cpp
 
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

//##############################################################################
// Inc
//##############################################################################

#include  "HBcommon.h"
#include  "HBconfig.h"

//##############################################################################
// Var
//##############################################################################

struct node_struct node;

StaticJsonBuffer <128> jsonBuf;

Coos <COOS_TASKS, 0> coos;  // declare cooperative operating system

const char str_mini_pro[] = "Mini Pro";
const char str_nano[] = "Nano";

const char* modules_list[] = {str_mini_pro, str_nano};

//##############################################################################
// Func
//##############################################################################

// =============================================
// Blink red LED
// =============================================
void blink(uint dur) // 10ms ticks
{
    node.led_cnt = dur;
    digitalWrite(RLED, HIGH);
}

// ========================================
// Debug: print hex value
// ========================================
uchar print_val(uchar val, uchar i)
{
#ifdef DEBUG
    if (val < 0x10)
        Serial1.print(F("0"));
    Serial1.print(val, HEX);
    Serial1.print(F(" "));
    if ((i & 7) == 7)
        Serial1.print(F(" "));
    if ((i & 0x1F) == 0x1F)
    {
        Serial1.println();
        return 1;
    }
    else
    {
        return 0;
    }
#else
    return 0;
#endif
}

// =============================================
// Validate char
// =============================================
uchar vld_char(char c)
{
    if (((uchar)c < CHAR_TAB) || ((uchar)c == 0xFF))
        return 0;
    else
        return 1;
}

// =====================================
// Debug: print buf
// =====================================
void printbuf(uchar* buf, uchar len)
{
#ifdef DEBUG
    if ((buf) && (len))
    {
        Serial1.print(F(" printing buf, len="));
        Serial1.println(len);
        for (uchar i=0; i<len; i++)
        {
            if (buf[i] < 0x10)
              Serial1.print(F("0"));
            Serial1.print(buf[i], HEX);
            Serial1.print(F(" "));
            if ((i & 7) == 7)
            {
                Serial1.print(F(" "));
            }
            if ((i & 0x0F) == 0x0F)
            {
                Serial1.println();
            }
        }
        Serial1.println();
    }
#endif
}

// ========================================
// Debug: print message buffer
// ========================================
void print_buf(const char* name, hb_msg_t* msg)
{
#ifdef DEBUG
    uchar nl = 1;
    Serial1.println();
    Serial1.print(name);
    Serial1.print(F(":"));
    if (msg->hb)
        Serial1.print(F(" hbus"));
    else
        Serial1.print(F(" mqtt"));
    if (msg->busy)
        Serial1.print(F(" busy"));
    if (msg->valid)
        Serial1.print(F(" valid"));
    if (msg->esc)
        Serial1.print(F(" esc"));
    if (msg->gate)
        Serial1.print(F(" gate"));
    Serial1.println();
    for (uchar i=0; i < msg->len; i++)
    {
        nl = print_val(msg->buf[i], i);
    }
    if (nl == 0)
        Serial1.println();
#endif
}
// =============================================
// Copy buffer
// =============================================
void copy_buf(uchar* src, uchar* dst, uchar len)
{
    if (dst)
    {
        for (uchar i=0; i<len; i++)
        {
            dst[i] = (src)? src[i] : 0; // if src does not exist then fill dst by 0
        }
    }
}
// =============================================
// Reverse byte order
// =============================================
void rev_4_bytes(uchar* buf)
{
    uchar tmp;
    tmp = buf[0];
    buf[0] = buf[3];
    buf[3] = tmp;
    tmp = buf[1];
    buf[1] = buf[2];
    buf[2] = tmp;
}
// =============================================
// Shift buffer down
// =============================================
void shift_buf(uchar* buf, uchar pos, uchar len)
{
    if ((buf) && (pos) && (len))
    {
        for (uchar i=0; i<len; i++)
        {
            buf[i] = buf[i+pos];
        }
    }
}
// =============================================
// Add uchar value to crc
// =============================================
uint crc_add_uchar(uchar b, uint crcx)
{
    crcx ^= (uint)b << 8;
    for (uchar j=0; j<8; j++)
    {
        crcx = (crcx & 0x8000)? ((crcx << 1) ^ 0x1021) : (crcx << 1);
    }
    return crcx;
}
// =============================================
 // Calculate CRC
// =============================================
/*
 * Name  : CRC-16 CCITT
 * Poly  : 0x1021    x^16 + x^12 + x^5 + 1
 * Init  : 0xFFFF
 * Revert: false
 * XorOut: 0x0000
 * Check : 0x3B0A ("123456789" hex)
 */
uint calc_crc(uchar* buf, uchar len)
{
    uint crcx = 0xFFFF;
    if ((buf) && (len))
    {
        for (uchar i=0; i<len; i++)
        {
            crcx = crc_add_uchar(buf[i], crcx);
        }
    }
    return crcx;
}
// =============================================
// Append txmsg with calculated crc
// =============================================
void crc_to_msg(hb_msg_t* msg)
{
    if (msg)
    {
        msg->buf[msg->len++] = (uchar)(msg->crc >> 8);
        msg->buf[msg->len++] = (uchar)msg->crc;
    }
}
// =============================================
// Reset Tx buffer and start a new message
// =============================================
uchar begin_txmsg(hb_msg_t* txmsg, uchar hb)
{
    if (txmsg->busy)
    {
        return NOT_READY;
    }
    else
    {
        txmsg->hb = (hb)? 1 : 0;
        txmsg->crc = 0xFFFF;    // init crc
        txmsg->len = 0;
        return READY;
    }
}
// =============================================
// Add char to Tx message
// =============================================
uchar add_txmsg_uchar(hb_msg_t* txmsg, uchar c)
{
    if ((txmsg->len >= MAX_BUF) || (txmsg->busy))
    {
        return ERR;
    }
    txmsg->crc = crc_add_uchar(c, txmsg->crc);   // calculate crc
    txmsg->buf[txmsg->len++] = c;
    return OK;
}
// =============================================
// Add 0-terminated string to Tx message
// =============================================
uchar add_txmsg_z_str(hb_msg_t* txmsg, char* str)
{
    uchar res = 0;
    if (txmsg->busy == 0)
    {
        while (*str)
        {
            if (txmsg->len < MAX_BUF)
            {
                add_txmsg_uchar(txmsg, (uchar)*str);
                str++;
                res++;
            }
            else
            {
                break;
            }
        }
    }
    return res;
}
// =============================================
// Copy message header
// =============================================
void copy_msg_hdr_0_6(hb_msg_t* src, hb_msg_t* txmsg)
{
    for (uchar i=0; i<6; i++)
    {
        if (i == 0)
        {
            add_txmsg_uchar(txmsg, src->buf[0] | 0x80); // set "reply" flag
        }
        else
        {
            add_txmsg_uchar(txmsg, src->buf[i]);
        }
    }
}
void copy_msg_hdr_0_3(hb_msg_t* src, hb_msg_t* txmsg)
{
    for (uchar i=0; i<3; i++)
    {
        if (i == 0)
        {
            add_txmsg_uchar(txmsg, src->buf[0] | 0x80); // set "reply" flag
        }
        else
        {
            add_txmsg_uchar(txmsg, src->buf[i]);
        }
    }
}
void copy_msg_hdr_5_6(hb_msg_t* src, hb_msg_t* txmsg)
{
    for (uchar i=5; i<6; i++)
    {
        if (i == 0)
        {
            add_txmsg_uchar(txmsg, src->buf[0] | 0x80); // set "reply" flag
        }
        else
        {
            add_txmsg_uchar(txmsg, src->buf[i]);
        }
    }
}

// =============================================
// Add timestamp
// =============================================
void add_ts(hb_msg_t* txmsg)
{
    union ulo_uni tmp;
    tmp.ulo = coos.uptime;
    add_txmsg_uchar(txmsg, tmp.uch[3]);
    add_txmsg_uchar(txmsg, tmp.uch[2]);
    add_txmsg_uchar(txmsg, tmp.uch[1]);
    add_txmsg_uchar(txmsg, tmp.uch[0]);
}
// =============================================
// Finish Tx message
// =============================================
uchar finish_txmsg(hb_msg_t* txmsg)
{
    if ((txmsg) && (txmsg->len < MAX_BUF-1))
    {
        crc_to_msg(txmsg);
        txmsg->busy = 1;
        return OK;
    }
    return ERR;
}
// =============================================
// Bubble sort
// =============================================
uchar sort(uint* arr, uint len)
{
    uchar res = 0;
    for (uint i=len-1; i>0; i--)
    {
        for (uint j=0; j<i; j++)
        {
            if ((0xFFFF & arr[j]) > (0xFFFF & arr[j+1]))
            {
                uint tmp = arr[j] & 0xFFFF;
                arr[j] = arr[j+1] & 0xFFFF;
                arr[j+1] = tmp;
                res = 1;    // changes made
            }
        }
    }
    return res;
}
// =============================================
// Limit string
// =============================================
void str_limit(char* str, uchar len)
{
    if ((uchar)str[0] == 0xFF)
    {
        str[0] = 0;
    }    
    if (len)
    {
        str[len-1] = 0;
    }    
}

// =============================================
// CSMA/CA - HBus collision avoidance
// =============================================
/*
                                                       +---.
                            +--------------------------|    \
                            |    +------+   +-----+    | AND |-------Tx 
    TXD --------------------*----| D    |---| inv |----|    /
                                 |      |   +-----+    +---'
           +-----+   +------+    |      |               
    RXD ---| inv |---| edge |----| G  R |----+    
           +-----+   +------+    +------+    |
                                            reset                         

At every RXD falling edge the circuit checks UART1 TXD output.  If it is high then it 
means that an external driver sets dominand state at HBus. Once flip-flop is set, it 
locks itself and can be reset only via R input (see reset_CSMA_CA()).

This mechanism does not detect all variants of clashes, but it is considered to be 
sufficient to avoid most collisions.
*/
// =============================================
// Configure event system
// =============================================
void EVSYS_config(void)
{
    EVSYS.CHANNEL2 = 0x4E;          // channel 2 source is PD6, it is connected to 
                                    // PC1 (USART1 RxD) by h/w link
    EVSYS.USERCCLLUT2A = 0x03;      // CCL LUT2 event A connected to channel 2
    //EVSYS.USEREVSYSEVOUTA = 0x03;   // EVSYS EVOUTA pin PA2 (D9) connected to channel 2, debug
}
// =============================================
// Configure CCL
// =============================================
void CCL_config(void)
{
    CCL.CTRLA = 0;      // disable CCL
    CCL.LUT0CTRLA = 0;
    CCL.LUT1CTRLA = 0;
    CCL.LUT2CTRLA = 0;
    CCL.LUT3CTRLA = 0;
    CCL.SEQCTRL0 = 0;       // sequencer 0 disabled
    CCL.SEQCTRL1 = 1;       // sequencer 1 is D flip-flop
    CCL.INTCTRL0 = 0;       // no CCL interrupts
    CCL.INTFLAGS = 0;
    // -----------
    // LUT0 - not used
    // -----------
    CCL.LUT0CTRLB = 0x00;   
    CCL.LUT0CTRLC = 0x08;   // INSEL2 to USART1 TxD    
    CCL.TRUTH0    = 0xF0;      
    // CCL.LUT0CTRLA = 0x41;   // OUTEN to PA3,  debug 

    // -----------
    // LUT1 - combines D flip-flop output and USART1 TxD
    // -----------
    CCL.LUT1CTRLB = 0x02;   // INSEL0 to LUT2 output, eg to D flip-flop output
    CCL.LUT1CTRLC = 0x08;   // INSEL2 to USART1 TxD   
    CCL.TRUTH1    = 0xFA;   // when D flip-flop is set then output is high, eg recessive state on HBus
    CCL.LUT1CTRLA = 0x61;   // OUTEN to PC3, filter, clock CLK_PER 

    // -----------
    // LUT2 - D input of flip-flop and flip-flop output
    // -----------
    CCL.LUT2CTRLB = 0x01;   // INSEL0 is feedback from D flip-flop output
    CCL.LUT2CTRLC = 0x08;   // INSEL2 to USART1 TxD
    CCL.TRUTH2    = 0xFA;   // direct, high at INSEL0 sets it high
    CCL.LUT2CTRLA = 0x01;   // clock CLK_PER
 
    // -----------
    // LUT3 - G input of flip-flop
    // -----------
    CCL.LUT3CTRLB = 0x33;   // source: event A, it is UART1 RxD
    CCL.LUT3CTRLC = 0x03;
    CCL.TRUTH3    = 0xFB;   // inverted IN1 to make falling edge
    CCL.LUT3CTRLA = 0xA1;   // edge detector, filter, clock CLK_PER

    CCL.CTRLA = 1;      // enable CCL
}
// =============================================
// Reset CSMA/CA logic
// =============================================
void reset_CSMA_CA(void)
{
    CCL.LUT2CTRLA = 0;      // When the even LUT is disabled, the latch is cleared asynchronously 
    CCL.LUT2CTRLA = 0x21;   
}

/* EOF */
