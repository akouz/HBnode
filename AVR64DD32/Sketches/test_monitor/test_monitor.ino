/*
 * Sketch   test_monitor.ino - monitor for HBnode
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

#include "config.h"
#include "monitor.h"


//##############################################################################
// Var
//##############################################################################

Coos <6, 0> coos;     // create coos with up to 6 tasks, tick is irrelevant

Mon mon;


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
// Task 2 runs every 1 ms
// ========================================
void coos_task2(void)
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
/*        
        i2cbb.print(mon.parse());
        i2cbb.print(" words, ");
        i2cbb.print(mon.param.val_cnt);
        i2cbb.print(" values: ");
        for (uchar i=0; i<mon.param.val_cnt; i++)
        {
          if (mon.param.val_t[i] == VAL_T_INT)
          {
            i2cbb.print((int)mon.param.val[i]);  
          }
          else
          {
            i2cbb.print("0x");
            i2cbb.print((uint)mon.param.val[i]);  
          }
          i2cbb.print(", ");
        }
        if (mon.param.str[0])
        {
          i2cbb.print("string=");
          i2cbb.print(mon.param.str);
          i2cbb.print(", ");
        }
        i2cbb.print("cmd=");
        i2cbb.println(mon.param.cmd);
*/        
        mon.parse();
        mon.exe();
        // i2cbb.println((uint)freeRam());
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
// Setup
// ========================================
void setup()
{
  i2cbb.println();
  i2cbb.println("==============");
  i2cbb.println("=== HBnode ===");
  i2cbb.println("==============");
  mon.print_rev();
  mon.print_sn();
  mon.print_unit_name();
  mon.print_unit_location();
  i2cbb.print("MON> "); // prompt
  
  pinMode(RLED, OUTPUT);
  pinMode(GLED, OUTPUT);
  digitalWrite(GLED, HIGH); 
  coos.register_clock(clock_func);  // register clock function
  coos.register_task(coos_task0);   // register task 0   
  coos.register_task(coos_task1);   // register task 1
  coos.register_task(coos_task2);   // register task 2
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
