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
#include <Adafruit_I2CDevice.h>
#include "displayHelpers.h"
#include "math.h"
#include "credentials.h"
#include <WiFi.h>
#include "DHT.h"
#include "Queue.h"

typedef uint16_t DHTSizeType;

constexpr byte LED_PIN = 2;
constexpr byte BUTTON_PIN = 4;

#define DHT_TYPE DHT11   // DHT 11
constexpr byte DHT_PIN = 5;

constexpr byte SDA_PIN = 21;
constexpr byte SCL_PIN = 22;


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
   unsigned short wifiCounter = 0;

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

      // draw circle for WiFi signal icon
      display.fillCircle(64, 52, 3, SSD1306_WHITE);
      // draw solid WiFi signal
      if (WL_CONNECTED == WiFi.status()) {
         // small arc
         drawArc(display, 64, 52, 12, 225, 315, 3);
         // medium arc
         drawArc(display, 64, 52, 20, 225, 315, 3);
         // big arc
         drawArc(display, 64, 52, 28, 225, 315, 3);
      } else { // draw WiFi connecting animation
         if (wifiCounter > 2) {wifiCounter = 0;}
         switch(wifiCounter) {
            case 2:
               drawArc(display, 64, 52, 28, 225, 315, 3);
            case 1:
               drawArc(display, 64, 52, 20, 225, 315, 3);
            case 0:
               drawArc(display, 64, 52, 12, 225, 315, 3);
         }
         wifiCounter++;
      }
      

      display.display();

      vTaskDelay(200 / portTICK_PERIOD_MS);
   }
}

TaskHandle_t DHTTaskHandle = NULL;

void DHTTask(void *parameter) {
   
   #define USE_FAHRENHEIT true
   constexpr DHTSizeType DHT_QUEUE_SIZE = 100; // a larger size will change the average values more slowly and stabilize output measurements

   DHT dht(DHT_PIN, DHT_TYPE);

   dht.begin();

   DHTSizeType bufferIndex = 0;

   float temperature = dht.readTemperature(USE_FAHRENHEIT);
   Queue<float> temperatureData(DHT_QUEUE_SIZE);

   float relativeHumidity = dht.readHumidity();
   Queue<float> relativeHumidityData(DHT_QUEUE_SIZE);

   float heatIndex = dht.computeHeatIndex(
      temperature, 
      relativeHumidity, 
      USE_FAHRENHEIT
   );
   Queue<float> heatIndexData(DHT_QUEUE_SIZE);


   if (isnan(temperature) || isnan(relativeHumidity) || isnan(heatIndex)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      vTaskSuspend(DHTTaskHandle);
   }

   for (;;) {

      // TODO: refactor DHT into another class

      temperature = dht.readTemperature(USE_FAHRENHEIT);
      relativeHumidity = dht.readHumidity();
      heatIndex = dht.computeHeatIndex(
         temperature, 
         relativeHumidity, 
         USE_FAHRENHEIT
      );

      temperatureData.push(temperature);
      relativeHumidityData.push(relativeHumidity);
      heatIndexData.push(heatIndex);

      Serial.print("Current Temperature: ");
      Serial.print(temperature);
      #if USE_FAHRENHEIT == true
         Serial.println("F");
      #else
         Serial.println("C");
      #endif

      Serial.print("Average Temperature: ");
      Serial.print(temperatureData.calculateAverage());
      #if USE_FAHRENHEIT == true
         Serial.println("F");
      #else
         Serial.println("C");
      #endif

      Serial.print("Current Relative Humidity: ");
      Serial.print(relativeHumidity);
      Serial.println("%");

      Serial.print("Average Relative Humidity: ");
      Serial.print(relativeHumidityData.calculateAverage());
      Serial.println("%");

      Serial.print("Current Heat Index: ");
      Serial.print(heatIndex);
      #if USE_FAHRENHEIT == true
         Serial.println("F");
      #else
         Serial.println("C");
      #endif

      Serial.print("Average Heat Index: ");
      Serial.print(heatIndexData.calculateAverage());
      #if USE_FAHRENHEIT == true
         Serial.println("F");
      #else
         Serial.println("C");
      #endif

      vTaskDelay(1000 / portTICK_PERIOD_MS);
   }
}

TaskHandle_t WiFiTaskHandle = NULL;

/**
   @brief Set WiFi Mode and connect to WiFi, then suspend task once connected
*/
void WiFiTask(void *parameter) {
   WiFi.disconnect(); // clear any previous WiFi connections
   WiFi.mode(WIFI_STA); // ESP32 only connects to WiFi

   if (WL_CONNECTED == WiFi.begin(WIFI_SSID, WIFI_PASSWORD)) {
      Serial.println(F("WiFi Connected!"));
   } else {
      Serial.println(F("WiFi Idle..."));
   }

   while (WL_CONNECTED != WiFi.status()) {
      Serial.println(F("WiFi Connecting..."));
      vTaskDelay(1000 / portTICK_PERIOD_MS);
   }
   Serial.println(F("WiFi Connected!"));

   vTaskSuspend(WiFiTaskHandle);

   // TODO: add reconnection logic later

   for (;;) {

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
  xTaskCreatePinnedToCore(
    WiFiTask,         // Task function
    "WiFiTask",       // Task name
    10000,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &WiFiTaskHandle,  // Task handle
    1                  // Core 1
  );
}

void loop() {
   // Empty because FreeRTOS scheduler runs the task
}