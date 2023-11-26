/*
 * File     HBcmd.cpp
 
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
 *                                                                                                                                                                        F
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

#include  "HBcmd.h"
#include  "HBmqtt.h"
#include  "HBcipher.h"

//##############################################################################
// Var
//##############################################################################

HB_cmd HBcmd;

#ifdef DEVICE_DESCRIPTION
    const char fixed_descr[] = DEVICE_DESCRIPTION;
#endif

//##############################################################################
// Func
//##############################################################################

// =====================================
// Constructor
// =====================================
HB_cmd::HB_cmd(void)
{
    this->ignore_collect = 0;
    this->cmd_reply.len = 0;
    this->cmd_reply.all = 0;
    this->custom_cmd = NULL;
    if (OK != this->read_EE_security())
    {
        PRINTLN(" Security read error");
    }
}
// =====================================
// Read security settings from EEPROM
// =====================================
uchar HB_cmd::read_EE_security(void)
{
    uchar buf[0x20];
    uchar res = ERR;
    node.allow.all = 0xFFFF;    // default: enable unecrypted access to all commands
    if (node.ID < 0xF000)        // if permanent NodeID assigned
    {
        res = i2cbb.read_EE(buf, EE_XTEA_KEY, 0x18);
        if (res == OK)
        {
            uchar offset = EE_SECURITY - EE_XTEA_KEY;
            uint val = 0x100*(uint)buf[offset] + buf[offset+1];
            uint notval = 0x100*(uint)buf[offset+2] + buf[offset+3]; // inverted
            notval = (~notval) & 0xFFFF;
            if (val == notval)              // if direct and inverted values are the same
            {
                node.allow.all = val;      // HBus mode settings
            }
            else // if values not matched but EEPROM key valid then unencrypted access restricted
            {
                uint crc = calc_crc(buf, 16);
                uint bufcrc = 0x100*buf[16] + buf[17];
                if (bufcrc == crc) // key valid
                {
                    node.allow.all = 0; 
                    node.allow.rev = 1;
                    node.allow.status = 1;
                    node.allow.rddescr = 1;
                    node.allow.rdsecurity = 1;
                    node.allow.ignore_ts = 1; 
                    PRINT(" Key CRC valid, use default settings, ");
                }            
                else
                {
                    PRINT(" Key CRC invalid, allow all, ");    
                }
                res = ERR;            
            }
        } 
        else
            PRINT(" EE read failed, allow all");
    }
    else
    {
        PRINT(" Temp NodeID, allow all");
    }
    return res;
}
// =====================================
// Process input commands, form a reply if required
// =====================================
hb_msg_t* HB_cmd::process_rx_cmd(hb_msg_t* rxmsg)
{
    uchar cmd;
    uchar res = 0; // no reply
    if ((rxmsg) && (this->cmd_reply.busy == 0) && (rxmsg->hb) && (rxmsg->len >= 12))
    {
        //PRINT(" HB_command ");
        this->cmd_reply.hb = rxmsg->hb;
        cmd = rxmsg->buf[0];
        // ----------------------------
        // reply to COLLECT command
        // ----------------------------
        if ((cmd == CMD_COLLECT) && (this->ignore_collect == 0))
        {
            if ((rxmsg->encrypt) || (node.allow.collect))
            {
                if ((node.allow.ignore_ts) || (rxmsg->ts_ok) || (!rxmsg->encrypt))  // timestamp
                {
                    res = this->rply_collect(rxmsg, &cmd_reply);
                    rxmsg->busy = 0;
                    return &this->cmd_reply;
                }
            }
        }
        // ----------------------------
        // if ID (eg address) matches
        // ----------------------------
        else if ((rxmsg->buf[3] == node.id[1]) && (rxmsg->buf[4] == node.id[0]))
        {
            if ((node.allow.ignore_ts) || (rxmsg->ts_ok) || (!rxmsg->encrypt))  // timestamp
            {
                this->cmd_reply.hb = rxmsg->hb;
                this->cmd_reply.encrypt = rxmsg->encrypt;
                begin_txmsg(&this->cmd_reply, rxmsg->hb);
                this->cmd_reply.postpone = 0;
                switch(cmd)
                {
                    case CMD_REV:       res = this->rply_rev(rxmsg, &this->cmd_reply);      break;
                    case CMD_STATUS:    res = this->rply_status(rxmsg, &this->cmd_reply);   break;
                    case CMD_PING:      res = this->rply_ping(rxmsg, &this->cmd_reply);     break;
                    case CMD_SET_ID:    res = this->rply_setID(rxmsg, &this->cmd_reply);    break;
                    case CMD_BOOT:      res = this->rply_boot(rxmsg, &this->cmd_reply);     break;
                    case CMD_BEEP:      res = this->rply_beep(rxmsg, &this->cmd_reply);     break;
                    case CMD_DESCR:     res = this->rply_descr(rxmsg, &this->cmd_reply);    break;
                    case CMD_SECURITY:  res = this->rply_security(rxmsg, &this->cmd_reply); break;
                    case CMD_CUSTOM:    res = this->rply_custom(rxmsg, &this->cmd_reply);   break;
                    case CMD_TOPIC:     res = this->rply_topic(rxmsg, &this->cmd_reply);    break;
                    default:            res = this->rply_unknown(rxmsg, &this->cmd_reply);  break;
                }
                if (READY == res)
                {
                    if (OK == finish_txmsg(&cmd_reply))
                    {
                        rxmsg->busy = 0;
                        rxmsg = NULL;
                        return &this->cmd_reply;
                    }
                }
                else
                {
                    PRINT("--> ERR_001: res=");
                    PRINTLN(res);
                }
            }
        }
    }
    rxmsg->busy = 0;
    return NULL;
}

// =====================================
// Prepare reply header
// =====================================
void  HB_cmd::prep_rply_hdr(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    //copy_msg_hdr(rxmsg, 0, 6, rply);
    copy_msg_hdr_0_6(rxmsg, rply);
}
// =====================================
// Add nonce, timestamp and OK/ERR to reply header
// =====================================
void  HB_cmd::add_to_hdr(hb_msg_t* rply, uchar okerr)
{
    add_txmsg_uchar(rply, random(0x100));  // nonce
    add_txmsg_uchar(rply, okerr);
    add_ts(rply);                           // timestamp
}
// =====================================
// Unknown command
// =====================================
uchar HB_cmd::rply_unknown(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    if (!rxmsg->encrypt)
    {
        this->prep_rply_hdr(rxmsg, rply);
        this->add_to_hdr(rply, ERR_UNKNOWN);
        return READY;
    }
    return ERR_SECURITY; // do not reply to unknown encrypted commands
}

// =====================================
// Reply revisions
// =====================================
uchar HB_cmd::rply_rev(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    if ((rxmsg->encrypt) || (node.allow.rev))
    {
        this->prep_rply_hdr(rxmsg, rply);
        this->add_to_hdr(rply, OK);
        add_txmsg_uchar(rply, HB_DEV_TYPE);
        add_txmsg_uchar(rply, HB_DEV_MODEL);
        add_txmsg_uchar(rply, HB_HW_REV_MAJ);
        add_txmsg_uchar(rply, HB_HW_REV_MIN);
        add_txmsg_uchar(rply, HB_BOOT_REV_MAJ);
        add_txmsg_uchar(rply, HB_BOOT_REV_MIN);
        add_txmsg_uchar(rply, HB_SKETCH_REV_MAJ);
        add_txmsg_uchar(rply, HB_SKETCH_REV_MIN);
        add_txmsg_uchar(rply, HB_REV_MAJ);      // HBus revision
        add_txmsg_uchar(rply, HB_REV_MIN);
        return READY;
    }
    return ERR_SECURITY;
}
// =====================================
// Reply status
// =====================================
uchar HB_cmd::rply_status(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    char buf[0x80];
    if ((rxmsg->encrypt) || (node.allow.status))
    {
        uint tpc;
        this->prep_rply_hdr(rxmsg, rply);
        this->add_to_hdr(rply, DF_STATUS);
        if (DF_STATUS == 1) // DF = JSON
        {
            // list all topics
            snprintf(buf, sizeof(buf),"{tid:[");
            add_txmsg_z_str(rply, buf);
            buf[0] = 0;
            for (uchar ti=0; ti< MAX_TOPIC; ti++)
            {
                if ((HBmqtt.flag[ti].topic_name_valid) && (HBmqtt.flag[ti].topic_valid))
                {
                    if (buf[0])
                    {
                        add_txmsg_uchar(rply, uchar(',')); // add comma between items
                    }
                    tpc = ownTopicId[ti];
                    snprintf(buf, sizeof(buf),"%d", tpc);
                    add_txmsg_z_str(rply, buf);
                }
            }
            // list all topics values
            snprintf(buf, sizeof(buf),"], val:[");
            add_txmsg_z_str(rply, buf);
            buf[0] = 0;
            for (uchar ti=0; ti<MAX_TOPIC; ti++)
            {
                if ((HBmqtt.flag[ti].topic_name_valid) && (HBmqtt.flag[ti].topic_valid))
                {
                    if (buf[0])
                    {
                        add_txmsg_uchar(rply, uchar(',')); // add comma between items
                    }
                    if (HBmqtt.flag[ti].val_type)           // value is valid
                    {
                        HBmqtt.print_own_val(ti, buf);
                        add_txmsg_z_str(rply, buf);
                    }
                    else
                    {
                        add_txmsg_uchar(rply,(uchar)'-');   // value is invalid
                        buf[0] = '-';
                    }
                }
            }            
            add_txmsg_uchar(rply, ']');
            add_txmsg_uchar(rply, '}');
        }
        else // DF = binary, other formats not implemented yet
        {
            add_txmsg_uchar(rply, MAX_TOPIC);
            for (uchar ti=0; ti<MAX_TOPIC; ti++)
            {
                tpc = ownTopicId[ti];
                add_txmsg_uchar(rply, (uchar)(tpc >> 8));
                add_txmsg_uchar(rply, (uchar)tpc);
            }
        }
        return READY;
    }
    return ERR_SECURITY;
}

// =====================================
// Reply COLLECT
// =====================================
uchar HB_cmd::rply_collect(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    uchar grp = rxmsg->buf[3];
    uchar res = 0;
    // -----------------------------
    // check group
    // -----------------------------
    switch (grp)
    {
        case 1: // all nodes
            res = READY;
            break;
        case 2: // nodes with tmp ID
            res = ((node.ID & 0xF000) == 0xF000)? READY : NOT_READY;
            break;
        case 3: // nodes with permanent ID
            res = ((node.ID & 0xF000) == 0xF000)? NOT_READY : READY;
            break;
        default:
            break;
    }
    // -----------------------------
    // if reply required
    // -----------------------------
    uchar slots = rxmsg->buf[4];
    if (READY == res)
    {
        rply->postpone = random(slots);
        cmd_reply.hb = rxmsg->hb;
        cmd_reply.encrypt = rxmsg->encrypt;
        begin_txmsg(&cmd_reply, rxmsg->hb);
        // copy_msg_hdr(rxmsg, 0, 3, rply);
        copy_msg_hdr_0_3(rxmsg, rply);
        add_txmsg_uchar(rply, node.id[1]);
        add_txmsg_uchar(rply, node.id[0]);
        // copy_msg_hdr(rxmsg, 5, 6, rply);
        copy_msg_hdr_5_6(rxmsg, rply);
        add_txmsg_uchar(rply, random(0x100)); // nonce
        add_txmsg_uchar(rply, OK);
        add_ts(rply);   // timestamp
        finish_txmsg(&cmd_reply);
    }
    else
    {
        rply->postpone = slots;
        rply->len = 0;
        res = READY;
    }
    return res;
}

// =====================================
// Reply PING
// =====================================
uchar HB_cmd::rply_ping(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    if ((rxmsg->encrypt) || (node.allow.ping))
    {
        this->ignore_collect = (uint)rxmsg->buf[7]*100;  // supplied interval in sec, convert it into 10 ms ticks
        this->prep_rply_hdr(rxmsg, rply);
        this->add_to_hdr(rply, OK);
        return READY;
    }
    return ERR_SECURITY;
}

// =====================================
// Reply SET_ID
// =====================================
#define MAGIC_ID 0x07FA
uchar HB_cmd::rply_setID(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    uint id = ((uint)rxmsg->buf[12] << 8) | rxmsg->buf[13];
    if ((node.ID >= 0xF000) || (id == MAGIC_ID)) // only tmp ID can be set, or if the 'magic' value presented
    {
        if ((HBcipher.valid == 0) || (rxmsg->encrypt))
        {
            uchar res = ERR;
            if (rxmsg->len > 12)
            {
                if (id == MAGIC_ID)
                {
                    node.ID = MAGIC_ID;
                    node.allow.all = 0xFFFF;                    
                }
                else
                {
                    node.ID = 0x100*rxmsg->buf[12] + rxmsg->buf[13];
                }
                i2cbb.write_EE(rxmsg->buf + 12, EE_nodeID, 2);
                res = OK;
            }
            //copy_msg_hdr(rxmsg, 0, 3, rply);
            copy_msg_hdr_0_3(rxmsg, rply);
            add_txmsg_uchar(rply, (uchar)(node.ID >> 8));
            add_txmsg_uchar(rply, (uchar)node.ID);
            //copy_msg_hdr(rxmsg, 5, 6, rply);
            copy_msg_hdr_5_6(rxmsg, rply);
            add_txmsg_uchar(rply, random(0x100));  // nonce
            add_txmsg_uchar(rply,  res);
            add_ts(rply);   // timestamp
            return READY;
        }
    }
    return ERR_SECURITY;
}

// =====================================
// Reply BOOT
// =====================================
uchar HB_cmd::rply_boot(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    uchar res = ERR_SECURITY;
    if ((rxmsg->encrypt) || (node.allow.boot))
    {
        
        uchar bcnt = rxmsg->buf[12];                        // byte count
        uint addr = 0x100*rxmsg->buf[13] + rxmsg->buf[14];  // address
        uchar rectype = rxmsg->buf[15];                     // record type
        res = ERR;
        if ((bcnt) && (rectype <= 1))                       // record types 0 and 1 are OK
        {
            if (addr >= 0x400)                              // it is a chunk of code 
            {
                res = i2cbb.write_EE(rxmsg->buf + 16, addr, bcnt);
                node.boot_in_progr = 100;   // supress Tx for 100 ms
            }
            else if ((addr == 0x0010) && (bcnt == 8))       // it is descriptor for bootloader
            {
                res = ERR_PARAM;
                if ((rxmsg->buf[16] == 0x55) && (rxmsg->buf[17] == 0xAA))       // pattern
                {
                    if ((rxmsg->buf[18] == 0xC3) && (rxmsg->buf[19] == 0x3C))   // pattern
                    {
                        uint codelen = 0x100*rxmsg->buf[20] + rxmsg->buf[21];   
                        uint rxcrc = 0x100*rxmsg->buf[22] + rxmsg->buf[23];    
                        uint eecrc = i2cbb.crc_EE(codelen);
                        PRINT("Codelen=");
                        PRINT(codelen);
                        PRINT(", rxcrc=");
                        PRINT(rxcrc);
                        PRINT(", eecrc=");
                        PRINTLN(eecrc);
                        if (rxcrc == eecrc)                                     // if crc matched
                        {
                            res = i2cbb.write_EE(rxmsg->buf + 16, 0x0010, 8);  // write descriptor
                            if (res == OK)
                            {
                                PRINTLN("Rebooting, please wait...");
                                node.rst_cnt = 15;      // reset in 150 ms
                            }
                            else
                            {
                                PRINTLN(" ==> ERROR: EEPROM error, abort");
                            }
                        }
                        else
                        {
                            PRINT(" ==> ERROR: CRC mismatch, eecrc=");
                            PRINT(eecrc);
                            PRINT(", rxcrc=");
                            PRINT(rxcrc);
                            PRINTLN(", abort");
                            res = ERR_CRC;
                        }
                    }
                }
            }
        }
        this->prep_rply_hdr(rxmsg, rply);
        this->add_to_hdr(rply, res);
        return READY;
    }
    return res;
}

// =====================================
// Reply BEEP
// =====================================
uchar HB_cmd::rply_beep(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    if ((rxmsg->encrypt) || (node.allow.ping))
    {
        blink((uint)rxmsg->buf[7]*100);
        this->prep_rply_hdr(rxmsg, rply);
        this->add_to_hdr(rply, OK);
        return READY;
    }
    return ERR_SECURITY;
}

// =====================================
// Reply DESCR
// =====================================
uchar HB_cmd::rply_descr(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    uchar len, lim, rdwr, str_no, res;
    char* str;
    uint addr;
    uchar buf[0x40];
    rdwr = rxmsg->buf[7] & 1; 
    str_no = rxmsg->buf[7] >> 1;   // 0 - name, 1 - location, 2 - description
    switch (str_no)
    {
    case 0: 
        str = node.name_str;
        addr = EE_NAME_STR;
        lim = MAX_SSTR;
        break;
    case 1: 
        str = node.location_str;
        addr = EE_LOCATION_STR;
        lim = MAX_LSTR;
        break;
    case 2: 
        str = node.descr_str;
        addr = EE_DESCR_STR;
        lim = MAX_LSTR;
        break;
    default:
        this->prep_rply_hdr(rxmsg, rply);
        this->add_to_hdr(rply, ERR_PARAM);
        return READY;
        break;
    }        
    // ----------------------
    // write
    // ----------------------
    if ((rdwr) && ((rxmsg->encrypt) || (node.allow.wrdescr)))
    {
#ifdef DEVICE_DESCRIPTION
        if (str == 2)
        {                
            this->prep_rply_hdr(rxmsg, rply);    // cannot write, description is fixed, it is defined in "HBconfig.h"
            this->add_to_hdr(rply, ERR);
            return READY;
        }    
#else
        res = ERR_PARAM;
        len = rxmsg->buf[12];
        if (str[0]) // string not empty
        {
            if ((rxmsg->buf[13] == '-') && (len = 1) && (rxmsg->len == 14))  // single symbol '-' erases existing string
            {
                rxmsg->buf[13] = 0;  // clear string
                rxmsg->buf[14] = 0;
            }
            else
            {
                len = lim+1;        // ensure error reply
                res = ERR_NOT_EMPTY;
            }
        }
        if ((len <= lim) && (rxmsg->len > 12))
        {
            rxmsg->buf[13+len] = 0;                             // ensure z-string
            res = i2cbb.write_EE(rxmsg->buf+13, addr, len+1);   // write string
            if (res == OK)
            {
                strcpy(str, (char*)rxmsg->buf+13);
            }
        }
        this->prep_rply_hdr(rxmsg, rply);
        this->add_to_hdr(rply, res);
        return READY;
#endif
    }
    // ----------------------
    // read
    // ----------------------
    else if ((rdwr == 0) && ((rxmsg->encrypt) || (node.allow.rddescr)))
    {
#ifdef DEVICE_DESCRIPTION
        if (str_no == 2)
        {
            len = sizeof(fixed_descr);
            lim = len;
            strcpy(buf, fixed_descr);
            res = OK;
        }
#else
        res = i2cbb.read_EE(buf, addr, lim);
#endif
        str_limit((char*)buf, lim);
        len = (uchar)strlen((char*)buf);
        if (buf[0] == 0)
        {
            len = 0;            
        }
        this->prep_rply_hdr(rxmsg, rply);
        this->add_to_hdr(rply, res);
        if (res == OK)
        {
            add_txmsg_uchar(rply,  len);
            if (len)
            {
                for (uchar i=0; i<len; i++)
                {
                    add_txmsg_uchar(rply, buf[i]);
                }
            }
            strcpy(str, (char*)buf); 
        }
        return READY;
    }
    return ERR_SECURITY;
}

// =====================================
// Set EEPROM cipher and access control settings
// =====================================
uchar HB_cmd::rply_security(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    // uchar buf[0x20];
    uchar wrbuf = 0;
    uchar val, res = ERR;
    uint  newval;
    uchar rdwr = rxmsg->buf[7] & 1;
    // ----------------------
    // store 
    // ----------------------
    if ((rdwr) && (rxmsg->len > 9))
    {
        if ((rxmsg->encrypt) || (HBcipher.valid == 0))
        {
            for(uchar i=0x10; i<0x20; i++)
            {
                i2cbb.buf[i] = 0xFF;
            }
            // store unencrypted access settings
            res = OK;
            newval = 0x100*rxmsg->buf[12] + rxmsg->buf[13]; 
            if (newval != node.allow.all)  // if new settings are different
            {                
                i2cbb.buf[0] = rxmsg->buf[12];
                i2cbb.buf[1] = rxmsg->buf[13];
                i2cbb.buf[2] = ~rxmsg->buf[12];
                i2cbb.buf[3] = ~rxmsg->buf[13];
                wrbuf = 1; // write new security
            }
            // store EEPROM key
            if (rxmsg->len > 20) // new cipher supplied
            {
                if (HBcipher.valid == 0) // stored EEPROM cipher is blank
                {
                    for (uchar i=0; i<4; i++) // 4 keys
                    {
                        for (uchar j=0; j<4; j++) // 4 bytes each
                        {
                            // reverse byte order in every key
                            i2cbb.buf[4*i+j] = rxmsg->buf[14 + 4*i + 3-j];
                        }
                    }
                    uint crc = calc_crc(i2cbb.buf, 16);
                    i2cbb.buf[0x10] = (uchar)(crc >> 8);
                    i2cbb.buf[0X11] = (uchar)crc;
                    i2cbb.buf[0X12] = 0xFF;
                    i2cbb.buf[0X13] = 0xFF;
                    i2cbb.buf[0x14] = rxmsg->buf[12];
                    i2cbb.buf[0x15] = rxmsg->buf[13];
                    i2cbb.buf[0x16] = ~rxmsg->buf[12];
                    i2cbb.buf[0x17] = ~rxmsg->buf[13];
                    wrbuf |= 2;  // write key                    
                }
                else
                {
                   PRINT(" EEkey_ignored");
                }
            }
        }
        switch (wrbuf & 3)
        {
        case 1: // write security only
            res = i2cbb.write_EE(i2cbb.buf, (uint)EE_SECURITY, 4);
            PRINT(" New_security");
            if (res == OK)
            {
                node.allow.all = newval;
            }            
            break;
        case 2: // write key only
            res = i2cbb.write_EE(i2cbb.buf, EE_XTEA_KEY, 0x12);
            PRINT(" New_EEkey");
            if (res == OK)
            {
                copy_buf(i2cbb.buf, HBcipher.key.uch, 16); 
                HBcipher.encrypt_EEkeys();
            }
            break;
        case 3: // write security and key 
            res = i2cbb.write_EE(i2cbb.buf, EE_XTEA_KEY, 0x18);             
            PRINT(" New security and EEkey");
            if (res == OK)
            {
                node.allow.all = newval;
                copy_buf(i2cbb.buf, HBcipher.key.uch, 16);                
                HBcipher.encrypt_EEkeys();
            }            
            break;
        default:
            PRINTLN(" Security_ignored"); 
            res = ERR_PARAM;       
            break;    
        }
        if (res == OK)
        {
            PRINTLN(" stored");
        }
        else if (res == ERR)
        {
            PRINTLN(" failed to store");
        }
        this->prep_rply_hdr(rxmsg, rply);
        this->add_to_hdr(rply, res);
        add_txmsg_uchar(rply, (uchar)(node.allow.all >> 8));
        add_txmsg_uchar(rply, (uchar)node.allow.all);
        return READY;
    }
    // ----------------------
    // read
    // ----------------------
    else
    {
        if ((rxmsg->encrypt) || (node.allow.rdsecurity))
        {
            val = (HBcipher.valid)? OK1 : OK;
            this->prep_rply_hdr(rxmsg, rply);
            this->add_to_hdr(rply, val);
            add_txmsg_uchar(rply, (uchar)(node.allow.all >> 8));
            add_txmsg_uchar(rply, (uchar)node.allow.all);
            return READY;
        }
        return ERR_SECURITY;
    }
    return ERR_PARAM;
}

// =====================================
// Reply C_CMD
// =====================================
uchar HB_cmd::rply_custom(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    if ((rxmsg->encrypt) || (node.allow.customcmd))
    {
        if (custom_cmd)
        {
            this->prep_rply_hdr(rxmsg, rply);
            this->add_to_hdr(rply, OK);
            rply = custom_cmd(rxmsg);        // whatever defined
        }
        else
        {
            this->prep_rply_hdr(rxmsg, rply); 
            this->add_to_hdr(rply, ERR);  // custom command not defined
        }
        return READY;
    }
    return ERR_SECURITY;
}

// =====================================
// Set custom command
// =====================================
void HB_cmd::set_custom_cmd(hb_msg_t* (*c_cmd)(hb_msg_t* msg))
{
    custom_cmd = c_cmd;   // set user-defined custom command
}

// =====================================
// Reply topic, eg pair TopicId + TopicName
// =====================================
uchar  HB_cmd::rply_topic(hb_msg_t* rxmsg, hb_msg_t* rply)
{
    if ((rxmsg->encrypt) || (node.allow.topic))
    {
        uchar ti = rxmsg->buf[7];               // get topic index
        if (HBmqtt.flag[ti].topic_name_valid)
        {
            this->prep_rply_hdr(rxmsg, rply);
            this->add_to_hdr(rply, OK);
            add_txmsg_uchar(rply, (uchar)(ownTopicId[ti] >> 8));
            add_txmsg_uchar(rply, (uchar)ownTopicId[ti]);
            char buf[0x40];
            uchar len = HBmqtt.copy_topic_name(ti, buf);
            for (uchar i=0; i<len; i++)
            {
                add_txmsg_uchar(rply, (char)buf[i]);
            }
            if (HBmqtt.flag[ti].val_type)    // if value valid
            {
                add_txmsg_uchar(rply, (uchar)',');
                add_txmsg_uchar(rply, (uchar)' ');
                HBmqtt.print_own_val(ti, buf);
                add_txmsg_z_str(rply, buf);
            }
        }
        else
        {
            this->prep_rply_hdr(rxmsg, rply);
            this->add_to_hdr(rply, ERR);
        }
        return READY;
    }
    return ERR_SECURITY;
}

// =====================================
// Every 10 ms
// =====================================
void HB_cmd::tick10ms(void)
{
    if (this->ignore_collect)
    {
        this->ignore_collect--;
    }
    if (node.led_cnt)
    {
        node.led_cnt--;
        if (node.led_cnt == 0)
        {
            digitalWrite(RLED, HIGH);
        }
    }
}

/* EOF */
