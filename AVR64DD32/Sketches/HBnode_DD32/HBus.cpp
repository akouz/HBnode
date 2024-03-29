/*
 * File     HBus.cpp
 
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

#include "HBus.h"
#include "HBcipher.h"
#include "HBrxtx.h" 
#include "HBcmd.h"
#include "HBmqtt.h"

//##############################################################################
// Var
//##############################################################################


//##############################################################################
// Func
//##############################################################################

// ========================================
// Clear receiver
// ========================================
void clr_rx(void)
{
    while (Serial1.available())
    {
        node.pause_cnt = 0;
        Serial1.read();  // clear
    }
}
// ========================================
// Dispose tx message
// ========================================
void  finish_tx(hb_msg_t* msg)
{
    /*
    if (CCL.INTFLAGS & 2)
    {
        CCL.INTFLAGS = 0;
        PRINTLN(" collision detected");
    }
    */
    if (msg)
    {
        msg->busy = 0;
        msg->valid = 0;
        Serial1.flush();
    }
}
// =============================================================================
// HBus task: receieve and decode messages, send HBus reply or prompt MQTT data
// =============================================================================
void coos_task_HBus_rxtx(void)
{
    int val;
    uchar buf[8];
    static uchar res;
    static uchar tmout;
    static hb_msg_t* rxmsg;
    static hb_msg_t* txmsg;
    COOS_DELAY(10);
    // ---------------------------
    // loop
    // ---------------------------
    while(1)
    {
        COOS_DELAY(1);
        // -----------------------------------------------
        // Process incoming HBus messages
        // -----------------------------------------------
        while (Serial1.available())
        {
            node.pause_cnt = 0;
            val = Serial1.read();
            if (val >= 0)
            {
                // --------------------------------------
                // if message completed and crc matched
                // --------------------------------------
                rxmsg = HBrxtx.rx((uchar)val);
                if (rxmsg)
                {
                    if (HBrxtx.flag.seed == 0) // first received message
                    {
                        HBrxtx.flag.seed = 1;
                        node.seed = (uint)millis(); // randomise
                        if (node.pup_cnt < (node.seed | 0xF00D))  // EEPROM endurance 100k write cycles
                        {
                            buf[0] = (uchar)(node.seed >> 8);
                            buf[1] = (uchar)node.seed;
                            i2cbb.write_EE(buf, EE_SEED, 2);
                            DBG_PRINTLN(" seed stored");
                        }
                    }
                    // --------------------------------
                    // process HBus message
                    // --------------------------------
                    if (rxmsg->hb)
                    {
                        txmsg = HBcmd.process_rx_cmd(rxmsg);
                        // --------------------------
                        // if reply required
                        // --------------------------
                        if ((txmsg) && (txmsg->encrypt)) // if message to be encrypted
                        {
                            HBcipher.encrypt(txmsg->buf, txmsg->len);
                        }
                        HBrxtx.rtr_cnt = 0;
                        while (txmsg)
                        {
                            // -----------------
                            // postpone transmission in first run
                            // -----------------
                            //DBG_PRINT(" postpone=");
                            //DBG_PRINT(txmsg->postpone);
                            COOS_DELAY(10*txmsg->postpone); // slot is 10 ms
                            txmsg->postpone = 0;
                            clr_rx();
                            if ((HBrxtx.rtr_cnt++ > 2) || (txmsg->len < 8))
                            {
                                finish_tx(txmsg); // bad message
                                txmsg = NULL;
                            }
                            // -----------------
                            // transmit
                            // -----------------
                            if (txmsg)
                            {
                                // ------------
                                // ensure bus is not busy
                                // ------------
                                tmout = 0;
                                HBrxtx.priority = 0xFF;
                                while (node.pause_cnt < 2)
                                {
                                    COOS_DELAY(1);
                                    clr_rx();           // receiver must be empty
                                    if (++tmout > 200)  // time-out 200 ms
                                    {
                                        finish_tx(txmsg);
                                        txmsg = NULL;
                                        break;
                                    }
                                }
                                // ------------
                                // transmit and check echo
                                // ------------
                                if ((txmsg) && (node.pause_cnt >= 2))
                                {
                                    digitalWrite(RLED, LOW);
                                    res = HBrxtx.start_tx(txmsg);
                                    if (OK == res)
                                    {
                                        res = NOT_READY;
                                        tmout = 0;
                                        while (NOT_READY == res)
                                        {
                                            COOS_DELAY(1);
                                            res = HBrxtx.tx(&node.pause_cnt);
                                            if (++tmout > 200)  // time-out 200 ms
                                            {
                                                finish_tx(txmsg);
                                                txmsg = NULL;
                                                break;
                                            }
                                        }
                                        if (READY == res)
                                        {
                                            finish_tx(txmsg); // success
                                            txmsg = NULL;
                                        }
                                        else    // echo mismatch
                                        {
                                            PRINTLN(" HBus collision");
                                            Serial1.end();
                                            COOS_DELAY(random(10));
                                            Serial1.begin(19200);
                                            COOS_DELAY(2);
                                        }
                                    } // if tx started
                                } // if pause on the bus
                            } // if txmsg
                        } // while txmsg
                        digitalWrite(RLED, HIGH);
                    } // if HBus message
                    // --------------------------------
                    // process received MQTT message
                    // --------------------------------
                    else
                    {
                        HBmqtt.rd_msg(rxmsg);           // if topic is in the GW own topic list - process it
                        rxmsg->busy = 0;
                    }
                } // if rxmsg
            }
        } // while Serial1.available
        // -----------------------------------------------
        // Broadcast MQTT-SN messages to the bus
        // -----------------------------------------------
        if (HBmqtt.mqmsg.valid)
        {
            txmsg = &HBmqtt.mqmsg;
            txmsg->busy = 1;
            if (txmsg->encrypt) // if message to be encrypted
            {
                HBcipher.encrypt(txmsg->buf, txmsg->len);
            }
            HBrxtx.rtr_cnt = 0;
            while(txmsg)
            {
                if (HBrxtx.rtr_cnt++ > 2)
                {
                    finish_tx(txmsg);
                    txmsg = NULL;
                }
                if (txmsg)
                {
                    tmout = 0;
                    HBrxtx.priority = 0xFF;
                    while (node.pause_cnt < 2)
                    {
                        COOS_DELAY(1);
                        clr_rx();           // receiver must be empty
                        if (++tmout > 200)  // time-out 200 ms
                        {
                            finish_tx(txmsg);
                            txmsg = NULL;
                            break;
                        }
                    }
                    if ((txmsg) && (node.pause_cnt >= 2))
                    {
                        if (OK == HBrxtx.start_tx(txmsg))
                        {
                            res = NOT_READY;
                            tmout = 0;
                            while (NOT_READY == res)
                            {
                                COOS_DELAY(1);
                                res = HBrxtx.tx(&node.pause_cnt);
                                if (++tmout > 200)
                                {
                                    finish_tx(txmsg);
                                    txmsg = NULL;
                                    break;
                                }
                            }
                            if (READY == res)
                            {
                                finish_tx(txmsg); // success
                                txmsg = NULL;
                            }
                            else    // echo mismatch
                            {
                                PRINTLN(" HBus collision");
                                Serial1.end();
                                COOS_DELAY(random(10));
                                Serial1.begin(19200);
                                COOS_DELAY(2);
                            }
                        } // if tx started
                    }
                }
            }  // while txmsg
            HBmqtt.mqmsg.busy = 0;   // ensure
            HBmqtt.mqmsg.valid = 0;
        } // if mqmsg.valid
    }
}

/* EOF */
