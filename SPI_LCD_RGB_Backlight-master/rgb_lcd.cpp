/*
  rgb_lcd.cpp
  
*/

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "rgb_lcd.h"

void rgb_lcd::spi_send_byte(unsigned char dta) {
	uint8_t i, temp;

	digitalWrite(_SPIclock, HIGH);
	delayMicroseconds(10);
	
	temp = dta;
	for(i=0; i<9; i++)
	{
		digitalWrite(_SPIclock, LOW);
	    if(i== 0)
		{
			digitalWrite(_SPIdata, HIGH);
		}
		else {
			if(temp & 0x80)        
				digitalWrite(_SPIdata, HIGH);
			else                                
				digitalWrite(_SPIdata, LOW);
			
			temp = temp<<1;
		}
	    delayMicroseconds(10);
		digitalWrite(_SPIclock, HIGH);
		delayMicroseconds(10);
	}
	
	delayMicroseconds(10);
	
	digitalWrite(_SPIclock, HIGH);
}

void rgb_lcd::spi_send_byteS(unsigned char *dta, unsigned char len) {
	for(int i=0;i<len;i++) {
		spi_send_byte(dta[i]);
	}
}

rgb_lcd::rgb_lcd()
{
}

rgb_lcd::rgb_lcd(uint8_t data, uint8_t clock, uint8_t latch ) {
  _i2cAddr = 255;

  _displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
  
  _SPIdata = data;
  _SPIclock = clock;
  _SPIlatch = latch;

  _SPIbuff = 0;

  // we can't begin() yet :(
}

void rgb_lcd::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) 
{
    if (_SPIclock != 255) {
		pinMode(_SPIdata, OUTPUT);
		pinMode(_SPIclock, OUTPUT);
		pinMode(_SPIlatch, OUTPUT);
		_SPIbuff = 0x80; // backlight
	}


    if (lines > 1) {
        _displayfunction |= LCD_2LINE;
    }
    _numlines = lines;
    _currline = 0;

    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != 0) && (lines == 1)) {
        _displayfunction |= LCD_5x10DOTS;
    }

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
    delayMicroseconds(50000);


    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(4500);  // wait more than 4.1ms

    // second try
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(150);

    // third go
    command(LCD_FUNCTIONSET | _displayfunction);


    // finally, set # lines, font size, etc.
    command(LCD_FUNCTIONSET | _displayfunction);

    // turn the display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    // clear it off
    clear();

    // Initialize to default text direction (for romance languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    command(LCD_ENTRYMODESET | _displaymode);
}

/********** high level commands, for the user! */
void rgb_lcd::clear()
{
    command(LCD_CLEARDISPLAY);        // clear display, set cursor position to zero
    delayMicroseconds(2000);          // this command takes a long time!
}

void rgb_lcd::home()
{
    command(LCD_RETURNHOME);        // set cursor position to zero
    delayMicroseconds(2000);        // this command takes a long time!
}

void rgb_lcd::setCursor(uint8_t col, uint8_t row)
{

    col = (row == 0 ? col|0x80 : col|0xc0);
    unsigned char dta[2] = {0x80, col};

	command(col);
}

// Turn the display on/off (quickly)
void rgb_lcd::noDisplay()
{
    _displaycontrol &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void rgb_lcd::display() {
    _displaycontrol |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void rgb_lcd::noCursor()
{
    _displaycontrol &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void rgb_lcd::cursor() {
    _displaycontrol |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void rgb_lcd::noBlink()
{
    _displaycontrol &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void rgb_lcd::blink()
{
    _displaycontrol |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void rgb_lcd::scrollDisplayLeft(void)
{
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void rgb_lcd::scrollDisplayRight(void)
{
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void rgb_lcd::leftToRight(void)
{
    _displaymode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void rgb_lcd::rightToLeft(void)
{
    _displaymode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void rgb_lcd::autoscroll(void)
{
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void rgb_lcd::noAutoscroll(void)
{
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void rgb_lcd::createChar(uint8_t location, uint8_t charmap[])
{
    location &= 0x7; // we only have 8 locations 0-7
    command(LCD_SETCGRAMADDR | (location << 3));
    
	digitalWrite(_SPIlatch, LOW);//LCD_CS_LOW;
    spi_send_byteS(charmap, 8);
	digitalWrite(_SPIlatch, HIGH);//LCD_CS_HIGH;
}

// Control the backlight LED blinking
void rgb_lcd::blinkLED(void)
{
    // blink period in seconds = (<reg 7> + 1) / 24
    // on/off ratio = <reg 6> / 256
    setReg(0x07, 0x17);  // blink every second
    setReg(0x06, 0x7f);  // half on, half off
}

void rgb_lcd::noBlinkLED(void)
{
    setReg(0x07, 0x00);
    setReg(0x06, 0xff);
}

/*********** mid level commands, for sending data/cmds */

// send command
inline void rgb_lcd::command(uint8_t value)
{
	uint8_t i, temp;
	digitalWrite(_SPIlatch, LOW);
	
	digitalWrite(_SPIclock, HIGH);
	delayMicroseconds(10);
	
	temp = value;
	for(i=0; i<9; i++) {
		digitalWrite(_SPIclock, LOW);
	    if(i== 0) {
			digitalWrite(_SPIdata, LOW);
		} else {
			if(temp & 0x80)        
				digitalWrite(_SPIdata, HIGH);
			else                                
				digitalWrite(_SPIdata, LOW);
			
			temp = temp<<1;
		}
	    delayMicroseconds(10);
		digitalWrite(_SPIclock, HIGH);
		delayMicroseconds(10);
	}
	
	delayMicroseconds(10);
	digitalWrite(_SPIclock, HIGH);
	
	digitalWrite(_SPIlatch, HIGH);
}

// send data
inline size_t rgb_lcd::write(uint8_t value)
{
	digitalWrite(_SPIlatch, LOW);
	spi_send_byte(value);
	digitalWrite(_SPIlatch, HIGH);
	return 1; // assume sucess
}

void rgb_lcd::setReg(unsigned char addr, unsigned char dta)
{
	digitalWrite(_SPIlatch, LOW);
	spi_send_byte(addr);
	spi_send_byte(dta);
	digitalWrite(_SPIlatch, HIGH);
}

void rgb_lcd::setRGB(unsigned char r, unsigned char g, unsigned char b)
{
    setReg(REG_RED, r);
    setReg(REG_GREEN, g);
    setReg(REG_BLUE, b);
}

const unsigned char color_define[4][3] = 
{
    {255, 255, 255},            // white
    {255, 0, 0},                // red
    {0, 255, 0},                // green
    {0, 0, 255},                // blue
};

void rgb_lcd::setColor(unsigned char color)
{
    if(color > 3)return ;
    setRGB(color_define[color][0], color_define[color][1], color_define[color][2]);
}
