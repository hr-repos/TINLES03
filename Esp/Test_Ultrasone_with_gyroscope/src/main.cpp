#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "UltrasonicSensor.h"
#include "CompassModule.h"
#include "DirectionCalculator.h"

// Define pins for ultrasonic sensors
#define TRIGGER_NORTH 12
#define ECHO_NORTH 13
#define TRIGGER_EAST 14
#define ECHO_EAST 15
#define TRIGGER_SOUTH 18
#define ECHO_SOUTH 19
#define TRIGGER_WEST 32
#define ECHO_WEST 33

// Create ultrasonic sensor objects
UltrasonicSensor northSensor(TRIGGER_NORTH, ECHO_NORTH);
UltrasonicSensor eastSensor(TRIGGER_EAST, ECHO_EAST);
UltrasonicSensor southSensor(TRIGGER_SOUTH, ECHO_SOUTH);
UltrasonicSensor westSensor(TRIGGER_WEST, ECHO_WEST);

// Create compass module object
CompassModule compass;

// Create direction calculator object
DirectionCalculator directionCalculator(northSensor, eastSensor, southSensor, westSensor, compass);

// Function prototypes
void readAndProcessData(void *pvParameters);
void printData(void *pvParameters);

void setup() {
    Serial.begin(115200);

    // Create FreeRTOS tasks
    xTaskCreate(readAndProcessData, "ReadAndProcessData", 2048, NULL, 1, NULL);
    xTaskCreate(printData, "PrintData", 2048, NULL, 1, NULL);
}

void loop() {
    // FreeRTOS handles the tasks, so loop is empty
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// Task to read and process sensor data
void readAndProcessData(void *pvParameters) {
    while (1) {
        directionCalculator.update();
        vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100ms
    }
}

// Task to print data
void printData(void *pvParameters) {
    while (1) {
        directionCalculator.printData();
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}