/*
 * Sketch   HBus_DD32.ino - HBus implementation for HBnode
 * Author   A.Kouznetsov
 * Target   HBnode (AVRxxDD32)
 * Compiler Arduino with DxCore

Redistribution and use in source and binary forms, with or without modification, 
are permitted.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//##############################################################################
// Inc
//##############################################################################

#include "HBcommon.h"
#include "HBmonitor.h"
#include "HBcipher.h"
#include "HBus.h"
#include "HBcmd.h"
#include "HBmqtt.h"

//##############################################################################
// Var
//##############################################################################


//##############################################################################
// Func
//##############################################################################

/*
// ========================================
// From https://docs.arduino.cc/learn/programming/memory-guide
// ========================================
int freeRam() 
{
  extern int __heap_start,*__brkval;
  int v;
  return (int)&v - (__brkval == 0  ? (int)&__heap_start : (int) __brkval);  
}
*/
// ========================================
// Setup clock
// ========================================
void set_xtal_24MHz(void)
{
    CCP = IOREG;                  // unlock
    CLKCTRL.XOSCHFCTRLA = 0x19;   // XTAL 24 MHz, settle in 1K cycles
    while ((CLKCTRL.MCLKSTATUS & 0x10) == 0) // wait until XTAL is stable
    {
      Nop();
    }
    CCP = IOREG;                  // unlock
    CLKCTRL.MCLKCTRLA = 0x03;     // EXTCLK, no clockout
}
// ========================================
// Task 0 blinks green LED
// ========================================
void coos_task0(void)
{
//  static uchar cnt;
  while(1)
  {
    digitalWrite(GLED, LOW); 
    COOS_DELAY(100);      
    digitalWrite(GLED, HIGH); 
    COOS_DELAY(900);      
  }
}
// ========================================
// Broadcast topic values
// ========================================
// if node has permanent ID (eg if node configured) then every minute
// check next topic and broadcast its value if value is valid
void coos_task_broadcast_val(void)
{
    //hb_msg_t*  msg;
    static uchar idx = 0;    // topic index
    static uchar topic_id_refresh = 250;
    COOS_DELAY(5000);                                   // initial pause 5 sec
    HBmqtt.validate_topics();
    PRINT("MON> ");
    // -------------------------------
    // loop
    // -------------------------------
    while(1)
    {
        if (++topic_id_refresh >= 200)  // after power-up and once in a while
        {
            topic_id_refresh = 0;
            // annonce own topics at HBus
            while (HBmqtt.init_topic_id(node.ID) != OK)
            {
                COOS_DELAY(500);
            }
        }
        if ((node.boot_in_progr == 0) && (node.ID < 0xF000)) // if not a temporary ID
        {
            if ((HBmqtt.flag[idx].val_type) && (HBmqtt.flag[idx].topic_valid))  // broadcast only valid values
            {
                // msg = HBmqtt.publish_own_val(idx);
                HBmqtt.publish_own_val(idx);
            }
            if (++idx >= MAX_TOPIC)
            {
                idx = 0;
            }
        }
        //COOS_DELAY(1000);
        COOS_DELAY(30000);  // pause 30 sec
        COOS_DELAY(30000);  // pause 30 sec
    }
}
// ========================================
// Monitor task runs every 1 ms 
// ========================================
void coos_task_1ms(void)
{
  static uchar cnt = 0;
  while(1)
  {
    COOS_DELAY(1);
    if (i2cbb.EE_busy)
    {
      i2cbb.EE_busy--;  // count EEPROM delay  
    }  
    if (node.pause_cnt < 200)
    {
      node.pause_cnt++;
    }
    if (node.boot_in_progr)
    {
      node.boot_in_progr--;
    }
    if (++cnt >= 10)  // 10 ms
    {
      cnt = 0;
      HBcmd.tick10ms();
      if (OK == mon.Rx()) // when monitor command string ready
      {
        mon.parse();
        mon.exe();
      }
      if (node.rst_cnt)
      {
        node.rst_cnt--;
        if (node.rst_cnt == 0)  // must reset
        {
          PRINTLN(" resetting...");
          COOS_DELAY(10);
          CCP = IOREG;          // unlock
          RSTCTRL.SWRR = 1;     // software reset
          while(1)              // not required really...
          {
            Nop();    
          }
        }
      }
    }
  }
}  
// ========================================
// Clock prints current time every minute
// ========================================
void clock_func(void)
{
    char buf[0x10];
    sprintf(buf," %02d:%02d ", coos.hour(), coos.minute());
}
// ========================================
// Print bar
// ========================================
void ser_println_bar(uchar len)
{
    for (uchar i=0; i<len; i++)
    {
      Serial1.print('=');
    }      
    Serial1.println();
}
// ================================
void println_bar(uchar len)
{
    for (uchar i=0; i<len; i++)
    {
      PRINT('=');
    }      
    PRINTLN();
}
// ========================================
// Print header text to HBus
// ========================================
void print_hdr_txt(void)
{
    while (Serial1.available())
    {
        Serial1.read();
        delay(1);
    }
    Serial1.println();
    uchar len = strlen(PROJECT_NAME);
    ser_println_bar(len+8);
    Serial1.print("=== ");
    Serial1.print(PROJECT_NAME);
    Serial1.println(" ===");
    ser_println_bar(len+8);
    
    Serial1.print("NodeID 0x");
    Serial1.print(node.ID, HEX);
    if (vld_char(node.name_str[0]))
    {
      Serial1.print(", name: ");
      Serial1.print(node.name_str);
    }
    Serial1.println();

    Serial1.print("Module '");
    Serial1.print(ARDUINO_MODULE);
    Serial1.print("', h/w rev ");
    Serial1.print(HW_REV_MAJ);
    Serial1.print('.');
    Serial1.print(HW_REV_MIN);
    Serial1.print(", S/N ");
    if (node.sn[1])
    {
      Serial1.print(node.sn[1]);
      Serial1.print('.');
    }
    Serial1.println(node.sn[0]);

    Serial1.print("Sketch: '");
    Serial1.print(SKETCH_NAME);
    Serial1.print("', rev ");
    Serial1.print(SW_REV_MAJ);
    Serial1.print('.');
    Serial1.print(SW_REV_MIN);
    Serial1.print(", signature ");
    Serial1.print(node.prog_len, HEX);
    Serial1.print('.');
    Serial1.println(node.prog_crc, HEX);

    if (vld_char(node.location_str[0]))
    {
      Serial1.print("Location: ");
      Serial1.println(node.location_str);
    }
    if (vld_char(node.descr_str[0]))
    {
      Serial1.print("Description: ");
      Serial1.println(node.descr_str);
    }  
    Serial1.println(" ");
}

// ========================================
// Setup
// ========================================
void setup()
{  
  //CCP = IOREG;                // unlock
  //WDT.CTRLA = 10;             // WDT period 4 sec
  pinMode(RLED, OUTPUT);
  digitalWrite(RLED, HIGH);   // red LED off
  pinMode(GLED, OUTPUT);
  digitalWrite(GLED, LOW);    // green LED on
  set_xtal_24MHz();
  Serial1.begin(19200);
  CCL_config();
  EVSYS_config();
  print_hdr_txt();
  PRINTLN();
  PRINTLN();
//  PRINT("RSTFR = ");
//  PRINTLN((uchar)RSTCTRL.RSTFR);
  uchar len = strlen(PROJECT_NAME);
  println_bar(len+8);
  PRINT("=== ");
  PRINT(PROJECT_NAME);
  PRINTLN(" ===");
  println_bar(len+8);
  mon.print_id();   
  mon.print_rev();
  mon.print_sn();   
  mon.print_node_name();
  mon.print_node_location();
  mon.print_node_descr();
  HBcipher.get_EE_key();
  PRINTLN();
  PRINT("EEPROM key ");
  (HBcipher.valid)? PRINTLN("valid"): PRINTLN("invalid");
  PRINT("MON> "); // prompt
  node.rst_cnt = 0;
  coos.register_clock(clock_func);              // register clock function
  coos.register_task(coos_task0);               // register task 0   
  coos.register_task(coos_task_broadcast_val); 
  coos.register_task(coos_task_HBus_rxtx);
  coos.register_task(coos_task_1ms);  
  coos.start();                     // init registered tasks
}

// ========================================
// Main loop 
// ========================================
void loop()
{  
  coos.run();       // Coos scheduler 
  asm("wdr");       // reset watchdog
}
/* EOF */
