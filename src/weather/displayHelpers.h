#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// #include <Adafruit_BusIO.h>
#include "math.h"

/**
   @brief draw an arc on a 128x64 OLED SSD1306 display

   @param display reference to an OLED display object
   @param x horizontal position on the screen
   @param y vertical position on the screen. Increases downwards
   @param radius radius of the arc's great circle. In pixels
   @param angleBegin angle that arc of the circle starts. In degrees
   @param angleEnd angle that the arc of the circle ends. In degrees
   @param thickness thickness of the arc. In pixels

   @returns Nothing

   @note This should use the Adafruit GFX and SSD1306 library

*/
void drawArc(
   Adafruit_SSD1306 &display,
   unsigned int x, 
   unsigned int y, 
   unsigned int radius, 
   float angleBegin, 
   float angleEnd,
   unsigned int thickness
);