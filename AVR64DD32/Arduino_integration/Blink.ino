/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
*/
#define BIT0    0x01  
#define BIT1    0x02
#define BIT2    0x04  
#define BIT3    0x08  
#define BIT4    0x10  
#define BIT5    0x20  
#define BIT6    0x40  
#define BIT7    0x80  

#define RED_LED  PIN_PF5
#define GRN_LED  PIN_PD5

// the setup function runs once when you press reset or power the board
void setup() 
{
    // initialize 
    VPORTD.DIR = BIT5;  // green LED
    VPORTD.OUT = BIT5;
    VPORTF.DIR = BIT5;  // red LED
    VPORTF.OUT = BIT5;
    VPORTF.OUT ^= BIT5; // toggle
}

// the loop function runs over and over again forever
void loop() 
{
  while(1)
  {
    VPORTF.OUT ^= BIT5;
    VPORTD.OUT ^= BIT5;
    delay(500);
  }  
}
