#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

SemaphoreHandle_t xBinarySemaphore = NULL;

const uint8_t inputPin = 2;
const uint8_t outputPin = 4;

static void vHandlerTask(void *pvParameters);
static void vPeriodicTask(void *pvParameters);
static void IRAM_ATTR vExampleInterruptHandler(void);

void setup() {
  Serial.begin(115200);
  xBinarySemaphore = xSemaphoreCreateBinary();

  if (xBinarySemaphore != NULL) {
    xTaskCreatePinnedToCore(vHandlerTask, "Handler", 2048, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(vPeriodicTask, "Periodic", 2048, NULL, 1, NULL, 0);

    pinMode(inputPin, INPUT_PULLUP);
    pinMode(outputPin, OUTPUT);
    digitalWrite(outputPin, HIGH);
    if (digitalRead(inputPin)) {
      Serial.println("pin 2 must be connected to pin 4");
      while (1)
        ;
    }
    attachInterrupt(digitalPinToInterrupt(inputPin), vExampleInterruptHandler, RISING);

  } else {
    Serial.println("Semaphore creation failed!");
    while (1)
      ;
  }
}

static void vHandlerTask(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE) {
      Serial.print("Handler task - Processing event.\r\n");
    }
  }
}

static void vPeriodicTask(void *pvParameters) {
  for (;;) {
    Serial.print("Periodic task - About to generate an interrupt.\r\n");
    digitalWrite(outputPin, LOW);
    xSemaphoreGive(xBinarySemaphore); // Signal the handler task
    vTaskDelay(pdMS_TO_TICKS(1)); // Give some time for the handler task to run
    digitalWrite(outputPin, HIGH);
    Serial.print("Periodic task - Interrupt generated.\r\n\r\n\r\n");
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

static void IRAM_ATTR vExampleInterruptHandler(void) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);

  if (xHigherPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

void loop() {}