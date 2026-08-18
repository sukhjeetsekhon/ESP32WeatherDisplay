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

constexpr byte LED_PIN = 2;
constexpr byte BUTTON_PIN = 4;
constexpr byte SDA_PIN = 21;
constexpr byte SCL_PIN = 22;


// Declare task handle
TaskHandle_t BlinkTaskHandle = NULL;

/**
   @brief toggle LED when button is pressed
*/
void BlinkTask(void *parameter) {
   constexpr uint8_t buttonDebounceDelay = 1000;
   for (;;) { // Infinite loop
      if (HIGH == digitalRead(BUTTON_PIN)) {
         digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      }
      vTaskDelay(buttonDebounceDelay / portTICK_PERIOD_MS);
   }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  xTaskCreatePinnedToCore(
    BlinkTask,         // Task function
    "BlinkTask",       // Task name
    10000,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &BlinkTaskHandle,  // Task handle
    1                  // Core 1
  );
}

void loop() {
   // Empty because FreeRTOS scheduler runs the task
}