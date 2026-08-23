#include "displayHelpers.h"

void drawArc(
    Adafruit_SSD1306 &display,
    unsigned int x,
    unsigned int y,
    unsigned int radius,
    float angleBegin,
    float angleEnd,
    unsigned int thickness
) {
   const float step = 1.0f;

   if (thickness == 0 || radius == 0) {
      return;
   }

   // Allow arcs to cross 0 degrees
   if (angleEnd < angleBegin) {
      angleEnd += 360.0f;
   }

   // Draw one arc for each pixel of thickness
   for (unsigned int t = 0; t < thickness; t++) {

      float currentRadius =
         radius - (thickness - 1) / 2.0f + t;

      if (currentRadius <= 0) {
         continue;
      }

      float radians = angleBegin * DEG_TO_RAD;

      int previousX = x + currentRadius * cosf(radians);
      int previousY = y + currentRadius * sinf(radians);

      for (float angle = angleBegin + step;
            angle <= angleEnd;
            angle += step) {

         radians = angle * DEG_TO_RAD;

         int currentX = x + currentRadius * cosf(radians);
         int currentY = y + currentRadius * sinf(radians);

         display.drawLine(
            previousX,
            previousY,
            currentX,
            currentY,
            SSD1306_WHITE
         );

         previousX = currentX;
         previousY = currentY;
      }

      // Ensure the exact end point is included
      radians = angleEnd * DEG_TO_RAD;

      int endX = x + currentRadius * cosf(radians);
      int endY = y + currentRadius * sinf(radians);

      display.drawPixel(
         endX,
         endY,
         SSD1306_WHITE
      );
   }
}

void drawShortWiFiArc(Adafruit_SSD1306 &display) {
   drawArc(display, 64, 52, 12, 225, 315, 3);
}

void drawMediumWiFiArc(Adafruit_SSD1306 &display) {
   drawArc(display, 64, 52, 20, 225, 315, 3);
}

void drawLongWiFiArc(Adafruit_SSD1306 &display) {
   drawArc(display, 64, 52, 28, 225, 315, 3);
}

void drawWiFiCircle(Adafruit_SSD1306 &display) {
   display.fillCircle(64, 52, 3, SSD1306_WHITE);
}

void drawWiFiIcon(Adafruit_SSD1306 &display) {
   drawWiFiCircle(display);
   drawShortWiFiArc(display);
   drawMediumWiFiArc(display);
   drawLongWiFiArc(display);
}

void drawWiFiStrength(Adafruit_SSD1306 &display, const long wifiStrength) {
   drawWiFiCircle(display);
   if (wifiStrength > EXCELLENT_WIFI_CONNECTION) {
      drawLongWiFiArc(display);
      drawMediumWiFiArc(display);
      drawShortWiFiArc(display);
   } else if (wifiStrength > GOOD_WIFI_CONNECTION) {
      drawMediumWiFiArc(display);
      drawShortWiFiArc(display);
   } else if (wifiStrength > BAD_WIFI_CONNECTION) {
      drawShortWiFiArc(display);
   }
}

void playWiFiConnectionAnimation(Adafruit_SSD1306 &display) {
   // create a counter that persists the program's lifetime
   static WiFiArc wifiArcCounter = shortArc;


   drawWiFiCircle(display); // always draw the WiFi circle in the icon
   switch(wifiArcCounter) {
      case longArc:
         drawLongWiFiArc(display); // draw all of the arcs
      case mediumArc:
         drawMediumWiFiArc(display); // draw medium and short arc
      case shortArc:
         drawShortWiFiArc(display); // only draw the short arc
   }

   if (longArc == wifiArcCounter ) { // if the counter reached the end, reset it
      wifiArcCounter = shortArc;
   } else { // set wifi arc to the next size
      wifiArcCounter = static_cast<WiFiArc>(static_cast<int>(wifiArcCounter) + 1);
   }
}

void drawCurrentSensorData(
   Adafruit_SSD1306 &display, 
   const float temp, 
   const float humidity, 
   const float heatIndex
) {
   display.setTextColor(WHITE); // Draw white text
   display.setCursor(0, 0);     // Start at top-left corner

   display.setTextSize(2);
   display.println(F("CURRENT"));
   display.println(F("SENSOR"));

   display.setTextSize(1);

   display.print(F("Temp: "));
   display.print(temp);

   #if USE_FAHRENHEIT == true
      display.println("F");
   #else
      display.println("C");
   #endif

   display.print(F("Humidity: "));
   display.print(humidity);
   display.println("%");

   display.print(F("Heat Index: "));
   display.print(heatIndex);
   #if USE_FAHRENHEIT == true
      display.println("F");
   #else
      display.println("C");
   #endif
}

void drawAverageSensorData(
   Adafruit_SSD1306 &display,
   const float temp, 
   const float humidity, 
   const float heatIndex
) {
   display.setTextColor(WHITE); // Draw white text
   display.setCursor(0, 0);     // Start at top-left corner

   display.setTextSize(2);
   display.println(F("AVERAGE"));
   display.println(F("SENSOR"));

   display.setTextSize(1);

   display.print(F("Temp: "));
   display.print(temp);

   #if USE_FAHRENHEIT == true
      display.println("F");
   #else
      display.println("C");
   #endif

   display.print(F("Humidity: "));
   display.print(humidity);
   display.println("%");

   display.print(F("Heat Index: "));
   display.print(heatIndex);

   #if USE_FAHRENHEIT == true
      display.println("F");
   #else
      display.println("C");
   #endif
}