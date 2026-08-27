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

/**
 * @brief Draws a scrolling line graph of the last N samples from a Queue<float>.
 *
 * Layout (128x64):
 *   [0..23]  = Y-axis labels (min/max)
 *   [24..127] = plot area  (104px wide, 46px tall)
 *   [0..8]   = title row
 *   [55..63] = X-axis label row
 *
 * @param display      Reference to the Adafruit_SSD1306 display
 * @param queue     The Queue<float> to read from
 * @param title     Graph title string (e.g. "Temp (Live)")
 * @param unit      Unit label for Y-axis (e.g. "F", "%", "F")
 */
void drawSensorGraph(
   Adafruit_SSD1306& display,
   Queue<float>&     queue,
   const char*       title,
   const char*       unit
) {

   // ── Graph Layout Constants ─────────────────────────────────────────────
   constexpr uint8_t G_TITLE_H   = 9;   // pixels reserved for title row (8px font + 1px gap)
   constexpr uint8_t G_LABEL_W   = 24;  // pixels reserved for Y-axis labels on the left
   constexpr uint8_t G_FOOTER_H  = 9;   // pixels reserved for X-axis label at the bottom

   constexpr uint8_t G_X         = G_LABEL_W;              // plot area left edge  (x=24)
   constexpr uint8_t G_Y         = G_TITLE_H;              // plot area top edge   (y=9)
   constexpr uint8_t G_W         = 128 - G_LABEL_W;        // plot area width      (104px)
   constexpr uint8_t G_H         = 64 - G_TITLE_H - G_FOOTER_H; // plot area height (46px)
   constexpr uint8_t G_BOTTOM    = G_Y + G_H;              // plot area bottom     (y=55)


   display.clearDisplay();

   const unsigned int numSamples = queue.getSize();

   // ── Nothing to draw yet ───────────────────────────────────────────────
   if (numSamples == 0) {
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(20, 28);
      display.print(F("No data yet..."));
      display.display();
      return;
   }

   // ── Retrieve min/max from Queue (O(1), already tracked) ───────────────
   float minVal = queue.getMin();
   float maxVal = queue.getMax();

   // Guard against flat line (all values equal) to avoid division by zero
   float range = maxVal - minVal;
   if (range < 0.001f) {
      range  = 1.0f;
      minVal -= 0.5f;
      maxVal  = minVal + 1.0f;
   }

   // ── Title ─────────────────────────────────────────────────────────────
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(0, 0);
   display.print(title);

   // ── Y-axis labels (min/max on the left) ───────────────────────────────
   // Max label — top of plot area
   display.setCursor(0, G_Y);
   display.print((int)maxVal);

   // Min label — bottom of plot area (nudge up by 8px for font height)
   display.setCursor(0, G_BOTTOM - 8);
   display.print((int)minVal);

   // ── Axis lines ────────────────────────────────────────────────────────
   // Vertical Y-axis line
   display.drawFastVLine(G_X - 1, G_Y, G_H, WHITE);
   // Horizontal X-axis line
   display.drawFastHLine(G_X - 1, G_BOTTOM, G_W + 1, WHITE);

   // ── X-axis label ─────────────────────────────────────────────────────
   // Center "Time ->" in the footer row
   display.setCursor(G_X + (G_W / 2) - 16, G_BOTTOM + 1);
   display.print(F("Time ->"));

   // ── Unit label (Y-axis, top-right of label area) ──────────────────────
   display.setCursor(0, G_Y + (G_H / 2) - 4);  // vertically centered
   display.print(unit);

   // ── Plot the line graph ───────────────────────────────────────────────
   //
   // We map up to G_W (104) samples across the plot width.
   // If fewer samples exist, we right-align them (scroll effect — newest = rightmost).
   //
   // Logical index 0 = oldest sample (front of queue)
   // Logical index (numSamples-1) = newest sample (back of queue)
   //
   // We only draw as many points as fit in G_W pixels.
   // startIdx lets older data scroll off the left edge once queue is full.

   unsigned int pointsToDraw = min((unsigned int)G_W, numSamples);
   unsigned int startIdx     = numSamples - pointsToDraw; // scroll: skip oldest if full

   int prevX = -1;
   int prevY = -1;

   for (unsigned int i = 0; i < pointsToDraw; i++) {
      float val = queue.get(startIdx + i);

      // Map value to Y pixel (invert: high value = low Y coordinate)
      int px = G_X + i;
      int py = G_BOTTOM - (int)(((val - minVal) / range) * (float)(G_H - 1));

      // Clamp within plot area
      py = constrain(py, G_Y, G_BOTTOM);

      if (prevX >= 0) {
         display.drawLine(prevX, prevY, px, py, WHITE);
      } else {
         display.drawPixel(px, py, WHITE);  // first point
      }

      prevX = px;
      prevY = py;
   }

   display.display();
}

void drawWeatherInfo(Adafruit_SSD1306 &display, const float temp, const float humidity) {
   display.setTextColor(WHITE); // Draw white text
   display.setCursor(0, 0);     // Start at top-left corner

   display.setTextSize(2);
   
   display.println("CURRENT");
   display.println("WEATHER");

   display.setTextSize(1);

   display.print(F("Temp: "));
   display.print(temp);
   display.println("F");

   display.print(F("Humidity: "));
   display.print(humidity);
   display.println("%");
}