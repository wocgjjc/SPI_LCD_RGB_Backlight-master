/*
  Hello World.ino

*/

#include <Wire.h>
#include <Digital_Light_TSL2561.h>
#include "rgb_lcd.h"

rgb_lcd lcd(42, 44, 38);//data, clk, cs
signed long light_value = 0;

void setup() 
{
    Wire.setSCL(7);
    Wire.setSDA(32);
    Wire.begin();
    
    Serial.begin(9600);
    
    TSL2561.init();
  
    lcd.begin(16, 2);
    lcd.print("Light value:");
    delay(1000);
}


void loop() 
{
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    Serial.print("The Light value is: ");
    light_value = TSL2561.readVisibleLux();
    Serial.println(light_value);
    lcd.print(light_value);
    lcd.print(' ');
    delay(1000);
}

