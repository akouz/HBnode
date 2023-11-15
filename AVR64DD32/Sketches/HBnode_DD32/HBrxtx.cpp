/*
 * File     HBrxtx.cpp
 * Rev      1.0 dated 20/12/2018
 
 * (c) 2019 Alex Kouznetsov,  https://github.com/akouz/hbus
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

#include "HBrxtx.h"
#include "HBcipher.h"

//##############################################################################
// Var
//##############################################################################

Hb_rxtx HBrxtx;

//##############################################################################
// Func
//##############################################################################

// =============================================
// Constructor
// =============================================
Hb_rxtx::Hb_rxtx(void)
{
    this->flag.all = 0;
    this->txbuf = NULL;
}

// =============================================
// Add a symbol to Rx buffer
// =============================================
uchar Hb_rxtx::add_rx_uchar(uchar val, hb_msg_t* dest)
{
    uchar res = NOT_READY;
    if ((dest) && (dest->len < MAX_BUF))  // if rx buffer exists and not full
    {
        if (dest->esc)  // previous char was ESC
        {
            // --------------------
            // char after ESC
            // --------------------
            dest->esc = 0;
            switch (val)
            {
                // -------------- beginning of a frame
                case _ESC_START_HB:
                case _ESC_START_MQ:
                case _ESC_START_HBE:
                case _ESC_START_MQE:
                    if (dest->gate)
                    {
                        this->err_cnt++;      // START without STOP
                    }
                    dest->gate = 1;
                    dest->len = 0;
                    dest->hb = 0;
                    if ((val == _ESC_START_HB) || (val == _ESC_START_HBE))
                    {
                        dest->hb = 1;   // HBus message
                    }
                    dest->encrypt = 0;
                    if ((val == _ESC_START_HBE) || (val == _ESC_START_MQE))
                    {
                        dest->encrypt = 1;  // encrypted message
                    }
                    break;
                // -------------- end of a frame
                case _ESC_END:
                    if (dest->gate == 0)
                    {
                        this->err_cnt++;      // STOP without START
                    }
                    else
                    {
                        res = READY;    // message completed, rx buffer ready
                        dest->gate = 0;
                        dest->ts_ok = 0;
                        // BUF_PRINTLN((uchar*)dest->buf, (uchar)dest->len);
                    }
                    break;
                // -------------- insert ESC
                case _ESC_ESC:
                    if (dest->gate == 0)
                    {
                        this->err_cnt++;
                    }
                    else
                    {
                        dest->buf[dest->len++] = _ESC;
                    }
                    break;
                // -------------- insert two ESC
                case _ESC_2ESC:
                    if (dest->gate == 0)
                    {
                        this->err_cnt++;
                    }
                    else
                    {
                        dest->buf[dest->len++] = _ESC;
                        if (dest->len < MAX_BUF)
                        {
                            dest->buf[dest->len++] = _ESC;
                        }
                    }
                    break;
                // --------------
                default:
                    this->err_cnt++;  // any other char after ESC is an error
                    break;
            }
        } // if was ESC
        else
        {
            // --------------------
            // regular input
            // --------------------
            if (val == _ESC)
            {
                dest->esc = 1;  // consider next char
            }
            else
            {
                if (dest->gate == 0)
                {
                    this->err_cnt++;
                }
                else
                {
                    dest->buf[dest->len++] = val;
                }
            }
        }
    }
    return res;
}

// =============================================
// Check crc
// =============================================
uchar Hb_rxtx::check_crc(hb_msg_t* msg)
{
    msgcrc = 0;
    calccrc = 0xF1F0;
    if ((msg) && (msg->len >= 2))
    {
        msg->len -= 2;
        msgcrc = 0x100*(uint)msg->buf[msg->len] + msg->buf[msg->len+1]; // supplied CRC
        calccrc = calc_crc(msg->buf, msg->len);
        if (msgcrc == calccrc)
        {
            return OK;
        }
    }
    return ERR;
}
// =============================================
// Check time stamp
// =============================================
uchar Hb_rxtx::check_ts(hb_msg_t* msg)
{
    ulo_uni ts;
    if (node.allow.ignore_ts)
    {
        if (coos.uptime < 0x10000000)
        {
            ts.uch[0] = msg->buf[11];
            ts.uch[1] = msg->buf[10];
            ts.uch[2] = msg->buf[9];
            ts.uch[3] = msg->buf[8];
            if (ts.ulo > 0x10000000)
            {
                DBG_PRINTLN(" TS accepted");
                coos.uptime = ts.ulo; // accept as initial value
            }
        }
        msg->ts_ok = 1;    
        return OK;
    }
    else
    {
        ts.uch[0] = msg->buf[11];
        ts.uch[1] = msg->buf[10];
        ts.uch[2] = msg->buf[9];
        ts.uch[3] = msg->buf[8];
        if ((coos.uptime + TIME_TOLERANCE > ts.ulo) || (coos.uptime < ts.ulo + TIME_TOLERANCE))
        {
            msg->ts_ok = 1;    
            return OK;
        }
        else if ((coos.uptime < 0x10000000) && (ts.ulo > 0x10000000))
        {
            DBG_PRINTLN(" TS accepted");
            coos.uptime = ts.ulo; // accept as initial value
            msg->ts_ok = 1;    
            return OK;
        }
        else
        {
            msg->ts_ok = 0;    
        }
    }
    return ERR;
}
// =============================================
// Receive symbol (while in receive mode)
// =============================================
hb_msg_t* Hb_rxtx::rx(uchar val)
{
    if (this->rxbuf.busy == 0)
    {
        if (READY == this->add_rx_uchar(val, &this->rxbuf))        
        {
            if (this->rxbuf.encrypt)                  // if message encrypted
            {
                HBcipher.decrypt(this->rxbuf.buf, this->rxbuf.len);  // then decrypt it
            }
            if ((this->flag.no_crc) || (OK == check_crc(&this->rxbuf)))  // if crc matches
            {
                // blink(5);   // blink 50 ms
                //DBG_PRINTLN(" rx msg OK");
                if (this->check_ts(&this->rxbuf) == OK)
                {
                    this->rxbuf.busy = 1;
                    return &this->rxbuf;
                }
                else // time stamp mismatch
                {
                    DBG_PRINTLN(" Rx reset");    
                    this->rxbuf.len = 0;     // reset output buffer
                    this->rxbuf.all = 0;
                }
            }
            else // crc mismatch
            {
                //DBG_PRINTLN(" crc mismatch");
                this->rxbuf.len = 0;     // reset output buffer
                this->rxbuf.all = 0;
            }
        }
    }
    return NULL;
}

// =============================================
// Start transmission
// =============================================
uchar Hb_rxtx::start_tx(hb_msg_t* buf)
{
    if ((buf == NULL) || (buf->len < 8))
    {
        return ERR_PARAM;
    }
    this->txbuf = buf;
    Serial1.write(priority);
    this->echobuf[0] = this->priority;
    Serial1.write(_ESC);
    this->echobuf[1] = _ESC;
    buf->esc = 1;
    this->echolen = 2;
    this->txcnt = 2;
    this->txpos = 0;
    buf->gate = 0;
    if (buf->encrypt)
    {
        this->start_symb = (buf->hb) ? _ESC_START_HBE : _ESC_START_MQE; // encrypted HBus or encrypted MQTT
    }
    else
    {
        this->start_symb = (buf->hb) ? _ESC_START_HB : _ESC_START_MQ; // unencrypted HBus or unencrypted MQTT
    }
    return OK;
}

// =============================================
// Transmit, insert byte-stuffing
// =============================================
uchar Hb_rxtx::tx(uchar* pause_cnt)
{
    uchar val;
    // ------------------------------
    // receive echo
    // ------------------------------
    while (Serial1.available())
    {
        *pause_cnt = 0;
        uchar rxchar = (uchar)Serial1.read();
        if ((this->echolen) && (rxchar == this->echobuf[0])) // if echo matches
        {
           this->echobuf[0] = this->echobuf[1];
           this->echobuf[1] = this->echobuf[2];
           this->echolen = (this->echolen)? this->echolen-1 : 0;
        }
        else
        {
            return ERR_ECHO;
        }
        if ((this->txpos >= this->txbuf->len) && (this->echolen == 0)) // txbuf transmitted, echo received
        {
            Serial1.flush();
            return READY;
        }
    }
    // ------------------------------
    // transmit next char(s)
    // ------------------------------
    while (this->echolen < 3)
    {
        if (this->txpos < this->txbuf->len)
        {
            if (this->txbuf->gate)  // if gate opened
            {
                if (this->txbuf->esc)
                {
                    this->txbuf->esc = 0;
                    if (this->txbuf->buf[this->txpos] == _ESC) // second ESC
                    {
                        this->txpos++;
                        val = _ESC_2ESC;
                    }
                    else    // it was a single ESC
                    {
                        val = _ESC_ESC;
                    }
                }
                else
                {
                    val = this->txbuf->buf[this->txpos++];
                    this->txbuf->esc = (val == _ESC) ? 1 : 0;
                }
                Serial1.write(val);
                this->echobuf[this->echolen++] = val;
            }
            else // open the gate - start transmission
            {
                Serial1.write(this->start_symb);
                this->echobuf[this->echolen++] = this->start_symb;
                this->txbuf->esc = 0;
                this->txbuf->gate = 1;
            }
        }
        else  // buffer finished
        {
            if  (this->txbuf->esc)    // last char was ESC
            {
                this->txbuf->esc = 0;
                val = _ESC_ESC;
            }
            else
            {
                switch(this->txpos - this->txbuf->len)
                {
                case 0: 
                    val = _ESC;         
                    break;
                case 1: 
                    val = _ESC_END;     
                    break;
                default:
                    this->txbuf->gate = 0;
                    this->txbuf->busy = 0;
                    return READY;
                    break;
                }
                this->txpos++;
            }
            Serial1.write(val);
            this->echobuf[this->echolen++] = val;
        } // buffer finished
        this->txcnt++;
    }
    return NOT_READY;
}

/* EOF */
