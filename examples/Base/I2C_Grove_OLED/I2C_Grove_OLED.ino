#include "WiSEN.h"

#include <Wire.h>
#include <SeeedOLED.h> // http://librarymanager/All#SSD1308

//https://wiki.seeedstudio.com/Grove-OLED_Display_0.96inch/

void setup()
{
  // Config I2C: D21(36) => SDA, D20(35) => SCL
  Wire.setPins(D21, D20);
  Wire.begin();

  // Turn ON Sensor Power Supply
  WiSEN.SensorPower(ON);
  
  SeeedOled.init();  //initialze SEEED OLED display

  SeeedOled.clearDisplay();          //clear the screen and set start position to top left corner
  SeeedOled.setNormalDisplay();      //Set display to normal mode (i.e non-inverse mode)
  SeeedOled.setPageMode();           //Set addressing mode to Page Mode
  SeeedOled.setTextXY(0,0);          //Set the cursor to Xth Page, Yth Column  
  SeeedOled.putString("Hello World!"); //Print the String

}

void loop()
{}
