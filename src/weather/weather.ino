
constexpr byte LED_PIN = 2;
constexpr byte BUTTON_PIN = 4;
constexpr byte SDA_PIN = 21;
constexpr byte SCL_PIN = 22;

// Declare task handle
TaskHandle_t BlinkTaskHandle = NULL;

void BlinkTask(void *parameter) {
  for (;;) { // Infinite loop
    digitalWrite(LED_PIN, HIGH);
    Serial.println("BlinkTask: LED ON");
    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1000ms
    digitalWrite(LED_PIN, LOW);
    Serial.println("BlinkTask: LED OFF");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.print("BlinkTask running on core ");
    Serial.println(xPortGetCoreID());
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