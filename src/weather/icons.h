/**


   @see https://media.istockphoto.com/id/1405447145/vector/weather-icons-pixel-art-set-weather-conditions-forecast-collection.jpg?s=612x612&w=0&k=20&c=se8L1LurN_24EbaVzRElSz2osp1vhqSlnQdTgp3JlTo=
*/

#ifndef ICONS_H
#define ICONS_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "math.h"

void drawSunIcon(Adafruit_SSD1306 &display);
void drawCloudIcon(Adafruit_SSD1306 &display);
void drawCloudySunIcon(Adafruit_SSD1306 &display);
void drawRaincloudIcon(Adafruit_SSD1306 &display);
void drawGustyWindIcon(Adafruit_SSD1306 &display);

#endif /* ICONS_H */