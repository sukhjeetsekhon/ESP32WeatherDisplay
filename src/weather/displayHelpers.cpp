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