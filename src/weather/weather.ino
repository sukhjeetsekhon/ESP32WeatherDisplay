/**
   @file weather.ino

   @brief control OLED display, read DHT sensor data, call Open-Meteo API
*/

/*

   OLED Wiring:
   3v3 -> VCC
   GND -> GND
   D21 -> SDA
   D22 -> SCL

   Button Wiring:
   3v3 -> Button Leads 1
   GND -> Resistor -> Button Leads 2 -> Input pin

*/

#include "Arduino.h"
#include "esp32-hal-gpio.h"
#include <SPI.h>
#include "math.h"

// SSD1306 OLED display libraries 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>

// WiFi, HTTP(S), JSON libraries 
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// DHT11 sensor library
#include "DHT.h"

// local header files
#include "icons.h"
#include "displayHelpers.h"
#include "printHelpers.h"
#include "credentials.h"
#include "Queue.h"
#include "api.h"
#include "config.h"
#include "pins.h"


// different pages to display on OLED with different information
enum DisplayPage {
   startup,        // TODO: show a logo and title
   wifiConnection, // TODO: make it show signal strength
   currentWeather, // TODO: show basic temperature and humidity with icons
   weatherTemperature,
   currentSensorData,
   averageSensorData,
   currentSensorTemperatureGraph,
   currentSensorHumidityGraph,
   currentSensorHeatIndexGraph,
   currentWeatherPage
};


// DHT data variables
float DHT_temperature = IMPOSSIBLE_TEMPERATURE;
float DHT_relativeHumidity = IMPOSSIBLE_HUMIDITY;
float DHT_heatIndex = IMPOSSIBLE_TEMPERATURE;

// DHT data queues to hold a lot of data and easily take averages over time
Queue<float> temperatureData(DHT_QUEUE_SIZE);
Queue<float> relativeHumidityData(DHT_QUEUE_SIZE);
Queue<float> heatIndexData(DHT_QUEUE_SIZE);

// WiFi config
constexpr wifi_power_t WIFI_MAX_POWER = WIFI_POWER_19_5dBm; 
long wifiStrength = 0;


// Page flipping config
constexpr DisplayPage firstPage = wifiConnection; // first page after startup finishes
constexpr DisplayPage lastPage = currentWeatherPage; // last page after startup finishes

volatile DisplayPage currentPage = startup;

volatile bool isButtonPressed = false;

// TODO: rename toggleButtonState function to mention flipping pages
/**
   @brief toggle button state by pushing it down and flip display page
*/
void toggleButtonState() {
   isButtonPressed = !isButtonPressed; // toggle button
   if (lastPage == currentPage) {
      currentPage = firstPage; // wrap back to first page
   } else { // flip to next page
      currentPage = static_cast<DisplayPage>(static_cast<int>(currentPage) + 1);
   }
}




// TODO: change blink task to blink after pressing button instead of toggling LED
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

   constexpr unsigned short OLED_WIDTH = 128;
   constexpr unsigned short OLED_HEIGHT = 64;

   // counter to play wifi connecting animation by adding longer signal arcs 
   WiFiArc wifiArcCounter = shortArc;

   Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT);

   if (DISPLAY_FAILED == display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println(F("OLED Display did not begin!"));
      vTaskSuspend(DisplayTaskHandle);
      Serial.println(F("Display task suspended!"));
   }

   // clear display
   display.display();
   vTaskDelay(200 / portTICK_PERIOD_MS);
   display.clearDisplay();

   // play wifi connection animation
   while (WL_CONNECTED != WiFi.status()) {
      display.clearDisplay();
      playWiFiConnectionAnimation(display);
      display.display();

      vTaskDelay(200 / portTICK_PERIOD_MS);
   }

   display.clearDisplay();

   drawWiFiIcon(display);

   display.display();

   vTaskDelay(3000 / portTICK_PERIOD_MS);

   currentPage = currentWeather; // go to currentWeather after startup finishes

   // wait for Weather Task to set variables to valid values
   while (
      IMPOSSIBLE_TEMPERATURE == currentTemperature || 
      IMPOSSIBLE_HUMIDITY == currentRelativeHumidity ||
      IMPOSSIBLE_WEATHER_CODE == weatherCode
   ) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
   }

   for(;;) {
      display.clearDisplay();

      switch (currentPage) {
         case startup:
            // after finishing startup, go to wifiConnection
            currentPage = wifiConnection;
            break;
         case wifiConnection:
            // if the WiFi is connected, show its connection strength
            if (WL_CONNECTED == WiFi.status()) {
               wifiStrength = WiFi.RSSI();
               drawWiFiStrength(display, wifiStrength);
            } else {
               playWiFiConnectionAnimation(display);
            }
            break;

         case weatherTemperature:
            drawTemperature(display, currentTemperature);
            break;

         case currentSensorData:
            drawCurrentSensorData(
               display, 
               DHT_temperature, 
               DHT_relativeHumidity, 
               DHT_heatIndex
            );
            printCurrentSensorData(
               DHT_temperature, 
               DHT_relativeHumidity, 
               DHT_heatIndex
            );

            break;
            
         case averageSensorData:
            drawAverageSensorData(
               display, 
               temperatureData.calculateAverage(), 
               relativeHumidityData.calculateAverage(), 
               heatIndexData.calculateAverage()
            );
            printAverageSensorData(
               temperatureData.calculateAverage(), 
               relativeHumidityData.calculateAverage(), 
               heatIndexData.calculateAverage()
            );
            break;

         case currentSensorTemperatureGraph: // scrolling graph of temperature
            #if USE_FAHRENHEIT == true
               drawSensorGraph(display, temperatureData, "Temp (Live)", "F");
            #else
               drawSensorGraph(display, temperatureData, "Temp (Live)", "C");
            #endif
            break;

         case currentSensorHumidityGraph: // scrolling graph of humidity
            drawSensorGraph(display, relativeHumidityData, "Humidity (Live)", "%");
            break;

         case currentSensorHeatIndexGraph: // scrolling graph of heat index
            #if USE_FAHRENHEIT == true
               drawSensorGraph(display, heatIndexData, "Heat Index (Live)", "F");
            #else
               drawSensorGraph(display, heatIndexData, "Heat Index (Live)", "C");
            #endif
            break;

         case currentWeather:
            drawWeatherInfo(display, currentTemperature, currentRelativeHumidity);
            break;
         case currentWeatherPage:
            if (isSunny(weatherCode)) {
               drawSunIcon(display);
            } else if (isCloudySun(weatherCode)) {
               drawCloudySunIcon(display);
            } else if (isCloudy(weatherCode)) {
               drawCloudIcon(display);
            } else if (isRain(weatherCode)) {
               drawRaincloudIcon(display);
            } else if (isThunder(weatherCode)) {
               drawThundercloudIcon(display);
            } else if (isSnowy(weatherCode)) {
               drawSnowcloudIcon(display);
            }

            // TODO: change weather icons to accomodate page title
            display.setTextSize(1);      // Normal 1:1 pixel scale
            display.setTextColor(WHITE); // Draw white text
            display.setCursor(0, 0);     // Start at top-left corner
            display.println(F("Current Weather: "));
            break;
      }
      
      display.display();

      vTaskDelay(DISPLAY_REFRESH_DELAY / portTICK_PERIOD_MS);
   }
}

TaskHandle_t DHTTaskHandle = NULL;

void DHTTask(void *parameter) {
   
   
   DHT dht(DHT_PIN, DHT11);

   dht.begin();

   DHTSizeType bufferIndex = 0;

   DHT_temperature = dht.readTemperature(USE_FAHRENHEIT);

   DHT_relativeHumidity = dht.readHumidity();

   DHT_heatIndex = dht.computeHeatIndex(
      DHT_temperature, 
      DHT_relativeHumidity, 
      USE_FAHRENHEIT
   );


   if (isnan(DHT_temperature) 
      || isnan(DHT_relativeHumidity) 
      || isnan(DHT_heatIndex)
   ) {
      Serial.println(F("Failed to read from DHT sensor!"));
      vTaskSuspend(DHTTaskHandle);
   }

   for (;;) {

      DHT_temperature = dht.readTemperature(USE_FAHRENHEIT);
      DHT_relativeHumidity = dht.readHumidity();
      DHT_heatIndex = dht.computeHeatIndex(
         DHT_temperature, 
         DHT_relativeHumidity, 
         USE_FAHRENHEIT
      );

      temperatureData.push(DHT_temperature);
      relativeHumidityData.push(DHT_relativeHumidity);
      heatIndexData.push(DHT_heatIndex);

      vTaskDelay(DHT_UPDATE_DELAY / portTICK_PERIOD_MS);
   }
}

TaskHandle_t WiFiTaskHandle = NULL;

/**
   @brief Set WiFi Mode and connect to WiFi, then suspend task once connected
*/
void WiFiTask(void *parameter) {
   WiFi.disconnect(); // clear any previous WiFi connections
   WiFi.mode(WIFI_STA); // ESP32 only connects to WiFi

   WiFi.setTxPower(WIFI_MAX_POWER);

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

TaskHandle_t WeatherTaskHandle = NULL;

void WeatherTask(void *parameter) {

   constexpr bool HTTP_BEGIN_FAIL = false;

   while(WL_CONNECTED != WiFi.status()) {
      vTaskDelay(5000 / portTICK_PERIOD_MS);
   }
   

   WiFiClientSecure client;
   HTTPClient http;
   int httpCode = 0;

   http.useHTTP10(true);
   client.setInsecure();

   if (HTTP_BEGIN_FAIL == http.begin(client, OPEN_METEO_API)) {
      Serial.println(F("HTTP begin failed!"));
      vTaskSuspend(WeatherTaskHandle);
   }
   

   for(;;) {
      if (WL_CONNECTED == WiFi.status()) {
         Serial.println(F("GET Request sent!"));
         Serial.print(F("HTTP Code: "));
         httpCode = http.GET();
         Serial.println(httpCode);
         Serial.println(http.errorToString(httpCode));
         DynamicJsonDocument doc(2048);

         DeserializationError error = deserializeJson(doc, http.getStream());

         if (error) {
            Serial.print(F("JSON parse failed: "));
            Serial.println(error.c_str());
         } else {
            // Serial.print(F("Time: "));
            // Serial.println(doc["current"]["time"].as<const char*>());

            Serial.print(F("Temperature: "));
            Serial.println(doc["current"]["temperature_2m"].as<float>());
            currentTemperature = doc["current"]["temperature_2m"].as<float>();

            Serial.print(F("Humidity: "));
            Serial.println(doc["current"]["relative_humidity_2m"].as<float>());
            currentRelativeHumidity = doc["current"]["relative_humidity_2m"].as<float>();

            Serial.print(F("Weather Code: "));
            Serial.println(doc["current"]["weather_code"].as<int>());
            weatherCode = doc["current"]["weather_code"].as<int>();
         }
      }

      vTaskDelay(API_CALL_DELAY / portTICK_PERIOD_MS); // wait 15 seconds
   }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), toggleButtonState, RISING);

  xTaskCreatePinnedToCore(
    BlinkTask,             // Task function     
    "BlinkTask",           // Task name         
    5000,                  // Stack size (bytes)
    NULL,                  // Parameters        
    1,                     // Priority          
    &BlinkTaskHandle,      // Task handle       
    1                      // Core 1            
  );                      
  xTaskCreatePinnedToCore(
    DisplayTask,           // Task function     
    "DisplayTask",         // Task name         
    10000,                 // Stack size (bytes)
    NULL,                  // Parameters        
    1,                     // Priority          
    &DisplayTaskHandle,    // Task handle       
    1                      // Core 1            
  );                      
  xTaskCreatePinnedToCore(
    DHTTask,               // Task function     
    "DHTTask",             // Task name         
    5000,                  // Stack size (bytes)
    NULL,                  // Parameters        
    1,                     // Priority          
    &DHTTaskHandle,        // Task handle       
    1                      // Core 1            
  );                      
  xTaskCreatePinnedToCore(
    WiFiTask,              // Task function     
    "WiFiTask",            // Task name         
    10000,                 // Stack size (bytes)
    NULL,                  // Parameters        
    1,                     // Priority          
    &WiFiTaskHandle,       // Task handle       
    1                      // Core 1            
  );                      
  xTaskCreatePinnedToCore(
    WeatherTask,           // Task function
    "WeatherTask",         // Task name
    10000,                 // Stack size (bytes)
    NULL,                  // Parameters
    1,                     // Priority
    &WeatherTaskHandle,    // Task handle
    1                      // Core 1
  );
}

void loop() {
   // Empty because FreeRTOS scheduler runs the task
}