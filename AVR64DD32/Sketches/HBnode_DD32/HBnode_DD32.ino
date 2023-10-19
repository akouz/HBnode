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
#include "HBus.h"
#include "HBcmd.h"
#include "HBcipher.h"

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
// Task 0 toggles led every 0.5 sec
// ========================================
void coos_task0(void)
{
//  static uchar cnt;
  while(1)
  {
//    digitalWrite(RLED, LOW); 
    COOS_DELAY(200);          // 500 ms
//    digitalWrite(RLED, HIGH); 
    COOS_DELAY(300);          // 500 ms
  }
}
// ========================================
// Task 1 sets led OFF every 0.65 sec
// ========================================
void coos_task1(void)
{
  COOS_DELAY(100);    
  while(1)
  {
    COOS_DELAY(950);      
    digitalWrite(GLED, LOW); 
    COOS_DELAY(50);      
    digitalWrite(GLED, HIGH); 
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
    if (++cnt >= 10)  // 10 ms
    {
        cnt = 0;
        HBcmd.tick10ms();
        if (OK == mon.Rx()) // when monitor command string ready
        {
            mon.parse();
            mon.exe();
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
    Serial1.println(SW_REV_MIN);

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
  CCP = IOREG;                // unlock
  WDT.CTRLA = 7;              // WDT period 0.5 sec
  pinMode(RLED, OUTPUT);
  digitalWrite(RLED, HIGH);   // red LED off
  pinMode(GLED, OUTPUT);
  digitalWrite(GLED, LOW);    // green LED on
  set_xtal_24MHz();
  Serial1.begin(19200);
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
  coos.register_clock(clock_func);  // register clock function
  coos.register_task(coos_task0);   // register task 0   
  coos.register_task(coos_task1);   // register task 1
  coos.register_task(coos_task_HBus_rxtx);
  coos.register_task(coos_task_1ms);  
  coos.start();                     // init registered tasks
}

// ========================================
// Main loop 
// ========================================
void loop()
{  
  coos.run();         // Coos scheduler 
  if (WDT.CTRLA > 5)  // short WDT periods used for deliberate reset
  {
    asm("wdr");       // reset watchdog
  }
}
/* EOF */
