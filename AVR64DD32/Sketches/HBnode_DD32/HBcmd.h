/*
 * File     HBcmd.h

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

#ifndef __HB_CMD_H
#define __HB_CMD_H

//##############################################################################
// Inc
//##############################################################################

#include  "HBcommon.h"

//##############################################################################
// Def
//##############################################################################

enum{
  CMD_REV            = 1,
  CMD_STATUS         = 2,
  CMD_COLLECT        = 3,
  CMD_PING           = 4,
  CMD_SET_ID         = 5,
  CMD_BOOT           = 6,
  CMD_BEEP           = 7,
  CMD_DESCR          = 8,
  CMD_SECURITY       = 9,
  CMD_CUSTOM         = 10,
  CMD_TOPIC          = 11,
};

//##############################################################################
// Class
//##############################################################################


class HB_cmd{
  public:
        HB_cmd(void);
    hb_msg_t*   process_rx_cmd(hb_msg_t* rxmsg);
    void        set_custom_cmd(hb_msg_t* (*c_cmd)(hb_msg_t* msg));
    void        tick10ms(void);

  private:
    union{
        uint ID;
        uchar id[2];
    }msg;
    uchar       rply_tmout;
    uint        ignore_collect;         // in 10 ms ticks
    hb_msg_t    cmd_reply;
    uchar       read_EE_security(void);
    void        prep_rply_hdr(hb_msg_t* rxmsg, hb_msg_t* rply);
    void        add_to_hdr(hb_msg_t* rply, uchar okerr);
    uchar       rply_unknown(hb_msg_t* rxmsg, hb_msg_t* rply);
    uchar       rply_rev(hb_msg_t* rxmsg, hb_msg_t* rply);
    uchar       rply_status(hb_msg_t* rxmsg, hb_msg_t* rply);
    uchar       rply_collect(hb_msg_t* rxmsg, hb_msg_t* rply);
    uchar       rply_ping(hb_msg_t* rxmsg, hb_msg_t* rply);
    uchar       rply_setID(hb_msg_t* rxmsg, hb_msg_t* rply);
    uchar       rply_boot(hb_msg_t* rxmsg, hb_msg_t* rply);
    uchar       rply_beep(hb_msg_t* rxmsg, hb_msg_t* rply);
    uchar       rply_descr(hb_msg_t* rxmsg, hb_msg_t* rply);
    uchar       rply_security(hb_msg_t* rxmsg, hb_msg_t* rply);
    uchar       rply_custom(hb_msg_t* rxmsg, hb_msg_t* rply);
    uchar       rply_topic(hb_msg_t* rxmsg, hb_msg_t* rply);    // read MQTT topic
    hb_msg_t*   (*custom_cmd)(hb_msg_t* msg);   // user-defined custom command
};
extern HB_cmd HBcmd;

#endif /* #define __HB_CMD_H */
