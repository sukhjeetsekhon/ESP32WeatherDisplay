#include "HardwareSerial.h"
#include "printHelpers.h"
#include "config.h"

void printCurrentSensorData(
   const float temp, 
   const float humidity, 
   const float heatIndex
) {
   Serial.print(F("Current Temperature: "));
   Serial.print(temp);
   #if USE_FAHRENHEIT == true
      Serial.println("F");
   #else
      Serial.println("C");
   #endif

   Serial.print(F("Current Relative Humidity: "));
   Serial.print(humidity);
   Serial.println("%");

   Serial.print(F("Current Heat Index: "));
   Serial.print(heatIndex);
   #if USE_FAHRENHEIT == true
      Serial.println("F");
   #else
      Serial.println("C");
   #endif
}