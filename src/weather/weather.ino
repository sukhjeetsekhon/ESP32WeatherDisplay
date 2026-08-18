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

constexpr byte LED_PIN = 2;
constexpr byte BUTTON_PIN = 4;
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
}

void loop() {
   // Empty because FreeRTOS scheduler runs the task
}