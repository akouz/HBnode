/*  Project:  HEXloader
 *  Author:   A.Kouznetsov
 *  File:     RxTx.h
 */
#ifndef __RXTX_H
#define __RXTX_H

//##############################################################################
// Inc
//##############################################################################

#include "description.h"

//##############################################################################
// Def
//##############################################################################

#define RX_BUF_LEN    0x120

//##############################################################################
// Class
//##############################################################################
class RxTx{
  public:
            RxTx(void);
    uchar   add_rx_char(char c);
    uchar   reply_rev(void);
    uchar   HEX_to_bin(binbuf_t*  binbuf);
    char    rxbuf[RX_BUF_LEN];
    uint    rxlen;
    uint    codelen;
  private:
};

#endif /* __RXTX_H */
