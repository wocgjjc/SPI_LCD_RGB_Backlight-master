/*
  Hello World.ino
  
*/

#include "rgb_lcd.h"

rgb_lcd lcd(42, 44, 38);//data, clk, cs

void setup() 
{
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    
    // Print a message to the LCD.
    lcd.print("hello, world!");

    delay(1000);
}

void loop() 
{
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print(millis()/1000);

    delay(100);
}

