/*
 * File     HBconfig.h
 * Target   HBnode (AVRxxDD32)
 * Compiler Arduino with DxCore

 * (c) 2023 Alex Kouznetsov,  https://github.com/akouz/hbnode
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

#ifndef __HB_CONFIG_H
#define	__HB_CONFIG_H

//##############################################################################
// Inc
//##############################################################################

//#include <coos.h>

//##############################################################################
// History
//##############################################################################

// rev 0.1 - derived from test_monitor.ino rev 1.9
// rev 0.2 - porting HBus code
// rev 0.3 - invoke HBus rx/tx
// rev 0.4 - monitor changed
// rev 0.5 - monitor handles backspace and can store long strings with blanks
// rev 0.6 - bug fix in backspace handling
// rev 0.7 - modified monitor parser
// rev 0.8 - eliminate mon.param string buffers
// rev 0.9 - can talk to HBus
// rev 0.10 - new implementation of HB_cmd::rply_boot, other changes in HBcmd.cpp
// rev 0.11 - use '-' to erase strings
// rev 0.12 - monitor commands EErd, EEwr EEclr
// rev 0.13 - wip, security settings
// rev 0.14 - can download sketches via HBus
// rev 0.15 - fine tune
// rev 0.16 - restoring HBus functionality
// rev 0.17 - handles HBus time stamps; rply_security() uses a workaround and operates 
//            correctly disregards to phenomena with passing parameters to functions 
// rev 0.18 - implementing CSMA/CA using CCL and EVSYS
// rev 0.19 - improved CSMA/CA implemetation, see CCL_config() 

//##############################################################################
// Def
//##############################################################################

// ==================================
// Software revision
// ==================================

#define PROJECT_NAME  "HBnode"  
#define ARDUINO_MODULE "HBnode Pro Mini"
#define HW_REV_MAJ    1
#define HW_REV_MIN    0
#define SKETCH_NAME   "HBnode_DD32.ino"
#define SW_REV_MAJ    0
#define SW_REV_MIN    19

#define SW_REV  (0x100*SW_REV_MAJ + SW_REV_MIN)

// ==================================
// Device descriptor for REV command
// ==================================
#define HB_DEV_TYPE         2           // device type
#define HB_DEV_MODEL        1           // device model
#define HB_REV_MAJ          1
#define HB_REV_MIN          5
#define HB_HW_REV_MAJ       HW_REV_MAJ  // h/w rev major
#define HB_HW_REV_MIN       HW_REV_MIN  // h/w rev minor
#define HB_BOOT_REV_MAJ     1           // boot rev major
#define HB_BOOT_REV_MIN     0           // boot rev minor, 1.0 is current revision of HBnode bootloader
#define HB_SKETCH_REV_MAJ   SW_REV_MAJ  // sketch rev major
#define HB_SKETCH_REV_MIN   SW_REV_MIN  // sketch rev minor

// ==================================
// COOS
// ==================================
#define COOS_TASKS    6

// ==================================
// Topics
// ==================================
#define MAX_TOPIC   4           // should be not greater than 16
#define TOPIC0      "test1"
#define TOPIC1      "test2"
#define TOPIC2      "test3"
#define TOPIC3      "test4"

// ==================================
// Pins, EEPROM, LEDs 
// ==================================
#define SET_SCL PORTD.DIRCLR = BIT3   // input
#define CLR_SCL PORTD.DIRSET = BIT3   // output
#define SCL_IN (PORTD.IN & BIT3)
#define SET_SDA PORTD.DIRCLR = BIT4
#define CLR_SDA PORTD.DIRSET = BIT4   
#define SDA_IN (PORTD.IN & BIT4)

#define EE_WR_ADDR  0xA8        // A0=0, A1=0, A2=1  
#define EE_RD_ADDR  0xA9        // A0=0, A1=0, A2=1  
#define FT_WR_ADDR  0x44        // 7-bit addr 0x22
#define FT_RD_ADDR  0x45  

#define RED_LED  PIN_PF5
#define GRN_LED  PIN_PD5

#define RLED  RED_LED
#define GLED  GRN_LED

#define TOGGLE_RLED PORTF.OUT ^= BIT5
#define TOGGLE_GLED PORTD.OUT ^= BIT5

#endif	/* __CONFIG_H */
