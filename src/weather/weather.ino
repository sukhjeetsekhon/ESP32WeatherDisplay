/**
   @file weather.ino

   @brief control OLED, sensors, and button
*/

/*

   Button Wiring:
   3v3 -> Button Leads 1
   GND -> Resistor -> Button Leads 2 -> Input pin

*/

#include "Arduino.h"
#include "esp32-hal-gpio.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

typedef uint16_t DHTSizeType;

constexpr byte LED_PIN = 2;
constexpr byte BUTTON_PIN = 4;

#define DHT_TYPE DHT11   // DHT 11
constexpr byte DHT_PIN = 5;

constexpr byte SDA_PIN = 21;
constexpr byte SCL_PIN = 22;

// currently unused. Uncomment if needed
// float calculateAverage(float* buffer, unsigned int size) {
//    float total = 0;
//    for (int i=0;i<size;i++) {
//       total += buffer[i];
//    }
//    return total / float(size);
// }

volatile bool isButtonPressed = false;

void toggleButtonState() {
   isButtonPressed = !isButtonPressed;
}

TaskHandle_t BlinkTaskHandle = NULL;

/**
   @brief toggle LED when button is pressed
*/
void BlinkTask(void *parameter) {
   constexpr uint8_t buttonDebounceDelay = 1000;
   for (;;) { // Infinite loop
      digitalWrite(LED_PIN, isButtonPressed);
      vTaskDelay(buttonDebounceDelay / portTICK_PERIOD_MS);
   }
}

TaskHandle_t DisplayTaskHandle = NULL;

void DisplayTask(void *parameter) {

   constexpr bool DISPLAY_FAILED = false;

   constexpr byte OLED_WIDTH = 128;
   constexpr byte OLED_HEIGHT = 64;

   int displayNumber = 0;

   Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT);

   if (DISPLAY_FAILED == display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println(F("OLED Display did not begin!"));
      vTaskSuspend(DisplayTaskHandle);
      Serial.println(F("Display task suspended!"));
   }

   display.display();
   vTaskDelay(200 / portTICK_PERIOD_MS);
   display.clearDisplay();

   for(;;) {
      display.clearDisplay();
      display.setTextSize(1);      // Normal 1:1 pixel scale
      display.setTextColor(WHITE); // Draw white text
      display.setCursor(0, 0);     // Start at top-left corner

      display.print(displayNumber);
      displayNumber++;

      display.display();

      vTaskDelay(200 / portTICK_PERIOD_MS);
   }
}

TaskHandle_t DHTTaskHandle = NULL;

void DHTTask(void *parameter) {
   
   #define USE_FAHRENHEIT true
   constexpr DHTSizeType DHT_BUFFER_SIZE = 100; // a larger size will change the average values more slowly and stabilize output measurements

   DHT dht(DHT_PIN, DHT_TYPE);

   dht.begin();

   DHTSizeType bufferIndex = 0;

   float temperature = dht.readTemperature(USE_FAHRENHEIT);
   float averageTemperature = 0;
   float temperatureBuffer[DHT_BUFFER_SIZE] = {0};
   DHTSizeType numValidTemperature = 0;

   float relativeHumidity = dht.readHumidity();
   float averageRelativeHumidity = 0;
   float relativeHumidityBuffer[DHT_BUFFER_SIZE] = {0};
   DHTSizeType numValidRelativeHumidity = 0;

   float heatIndex = dht.computeHeatIndex(
      temperature, 
      relativeHumidity, 
      USE_FAHRENHEIT
   );
   float averageHeatIndex = 0;
   float heatIndexBuffer[DHT_BUFFER_SIZE] = {0};
   DHTSizeType numValidHeatIndex = 0;

   if (isnan(temperature) || isnan(relativeHumidity) || isnan(heatIndex)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      vTaskSuspend(DHTTaskHandle);
   }

   for (;;) {

      // TODO: the average value calculations are complicated and bulky. Either refactor or forget about it.

      temperature = dht.readTemperature(USE_FAHRENHEIT);
      averageTemperature = 0;

      relativeHumidity = dht.readHumidity();
      averageRelativeHumidity = 0;

      heatIndex = dht.computeHeatIndex(
         temperature, 
         relativeHumidity, 
         USE_FAHRENHEIT
      );
      averageHeatIndex = 0;

      if (bufferIndex > DHT_BUFFER_SIZE - 1) {
         bufferIndex = 0;
      }

      numValidTemperature = 0;
      numValidRelativeHumidity = 0;
      numValidHeatIndex = 0;

      for (int i = 0; i < DHT_BUFFER_SIZE; i++) {
         if (0 == temperatureBuffer[i] || isnan(temperatureBuffer[i])) {
            continue;
         }
         numValidTemperature++;
         averageTemperature += temperatureBuffer[i];
      }

      for (int i = 0; i < DHT_BUFFER_SIZE; i++) {
         if (0 == relativeHumidityBuffer[i] || isnan(relativeHumidityBuffer[i])) {
            continue;
         }
         numValidRelativeHumidity++;
         averageRelativeHumidity += relativeHumidityBuffer[i];
      }

      for (int i = 0; i < DHT_BUFFER_SIZE; i++) {
         if (0 == heatIndexBuffer[i] || isnan(heatIndexBuffer[i])) {
            continue;
         }
         numValidHeatIndex++;
         averageHeatIndex += heatIndexBuffer[i];
      }

      averageTemperature /= numValidTemperature;
      averageRelativeHumidity /= numValidRelativeHumidity;
      averageHeatIndex /= numValidHeatIndex;

      Serial.print("Current Temperature: ");
      Serial.print(temperature);
      temperatureBuffer[bufferIndex] = temperature;
      #if USE_FAHRENHEIT == true
         Serial.println("F");
      #else
         Serial.println("C");
      #endif

      Serial.print("Average Temperature: ");
      Serial.print(averageTemperature);
      #if USE_FAHRENHEIT == true
         Serial.println("F");
      #else
         Serial.println("C");
      #endif

      Serial.print("Current Relative Humidity: ");
      Serial.print(relativeHumidity);
      relativeHumidityBuffer[bufferIndex] = relativeHumidity;
      Serial.println("%");

      Serial.print("Average Relative Humidity: ");
      Serial.print(averageRelativeHumidity);
      Serial.println("%");

      Serial.print("Current Heat Index: ");
      Serial.print(heatIndex);
      heatIndexBuffer[bufferIndex] = heatIndex;
      #if USE_FAHRENHEIT == true
         Serial.println("F");
      #else
         Serial.println("C");
      #endif

      Serial.print("Average Heat Index: ");
      Serial.print(averageHeatIndex);
      #if USE_FAHRENHEIT == true
         Serial.println("F");
      #else
         Serial.println("C");
      #endif

      bufferIndex++;

      vTaskDelay(1000 / portTICK_PERIOD_MS);
   }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), toggleButtonState, RISING);

  xTaskCreatePinnedToCore(
    BlinkTask,         // Task function
    "BlinkTask",       // Task name
    10000,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &BlinkTaskHandle,  // Task handle
    1                  // Core 1
  );
  xTaskCreatePinnedToCore(
    DisplayTask,         // Task function
    "DisplayTask",       // Task name
    10000,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &DisplayTaskHandle,  // Task handle
    1                  // Core 1
  );
  xTaskCreatePinnedToCore(
    DHTTask,         // Task function
    "DHTTask",       // Task name
    10000,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &DHTTaskHandle,  // Task handle
    1                  // Core 1
  );
}

void loop() {
   // Empty because FreeRTOS scheduler runs the task
}