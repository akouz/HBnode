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
    CLKCTRL.XOSCHFCTRLA = 0xA9;   // xtal 24 MHz
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
//    i2cbb.print('.');
    digitalWrite(RLED, LOW); 
    COOS_DELAY(500);          // 500 ms
    digitalWrite(RLED, HIGH); 
    COOS_DELAY(500);          // 500 ms
/*    
    if (++cnt >= 60)          // every minute if TICK_1000US selected
                              // or approx every 61 sec if TICK_1024US selected   
    {
      cnt = 0;
      i2cbb.print(" uptime = ");      // coos.uptime counts seconds correctly
      i2cbb.print((int)coos.uptime);  // disregard of tick selected
      i2cbb.println(" sec ");
    }
*/  
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
    COOS_DELAY(650);        // 650 ms
    digitalWrite(RLED, LOW); 
  }
}
// ========================================
// Monitor task runs every 1 ms 
// ========================================
void coos_task_monitor(void)
{
  static uchar cnt = 0;
  while(1)
  {
    COOS_DELAY(1);
    if (i2cbb.EE_busy)
    {
      i2cbb.EE_busy--;  // count EEPROM delay  
    }  
    if (++cnt >= 100)
    {
      cnt = 0;
      if (OK == mon.Rx()) // when command string ready
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
    // i2cbb.print(buf);
}
// ========================================
// Print bar
// ========================================
void println_bar(uchar len)
{
    for (uchar i=0; i<len; i++)
    {
      Serial1.print('=');
    }      
    Serial1.println();
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
    println_bar(len+8);
    Serial1.print("=== ");
    Serial1.print(PROJECT_NAME);
    Serial1.println(" ===");
    println_bar(len+8);
    
    Serial1.print("NodeID 0x");
    Serial1.print(node.ID, HEX);
    Serial1.print(", name: ");
    Serial1.println(node.name_str);

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

    Serial1.print("Location: ");
    Serial1.println(node.location_str);
    Serial1.print("Description: ");
    Serial1.println(node.descr_str);
}

// ========================================
// Setup
// ========================================
void setup()
{
  set_xtal_24MHz();
  Serial1.begin(19200);
  print_hdr_txt();
  i2cbb.println();
  i2cbb.println();
  i2cbb.println("==============");
  i2cbb.println("=== HBnode ===");
  i2cbb.println("==============");
  mon.print_id();   
  mon.print_rev();
  mon.print_sn();   
  mon.print_node_name();
  mon.print_node_location();
  mon.print_node_descr();
  i2cbb.print("MON> "); // prompt
  pinMode(RLED, OUTPUT);
  pinMode(GLED, OUTPUT);
  digitalWrite(GLED, HIGH);         // green LED off
  coos.register_clock(clock_func);  // register clock function
  coos.register_task(coos_task0);   // register task 0   
  coos.register_task(coos_task1);   // register task 1
  coos.register_task(coos_task_monitor); 
  coos.register_task(coos_task_HBus_rxtx);
  coos.register_task(coos_task_tick1ms);
  coos.start();                     // init registered tasks

}

// ========================================
// Main loop 
// ========================================
void loop()
{  
  coos.run();  // Coos scheduler 
}
/* EOF */
