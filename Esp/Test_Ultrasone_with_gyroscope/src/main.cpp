#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "UltrasonicSensor.h"
#include "CompassModule.h"
#include "DirectionCalculator.h"
#include "MqttClient.h"
#include "secrets.h"

// Define pins for ultrasonic sensors
#define TRIGGER_NORTH 17
#define ECHO_NORTH 16
#define TRIGGER_EAST 18
#define ECHO_EAST 5
#define TRIGGER_SOUTH 4
#define ECHO_SOUTH 19
#define TRIGGER_WEST 33
#define ECHO_WEST 32
// Create sensor objects
UltrasonicSensor northSensor(TRIGGER_NORTH, ECHO_NORTH);
UltrasonicSensor eastSensor(TRIGGER_EAST, ECHO_EAST);
UltrasonicSensor southSensor(TRIGGER_SOUTH, ECHO_SOUTH);
UltrasonicSensor westSensor(TRIGGER_WEST, ECHO_WEST);

// Create compass module object
CompassModule compass;

// Create direction calculator object
DirectionCalculator directionCalculator(northSensor, eastSensor, southSensor, westSensor, compass);

// Create MQTT client object
MqttClient mqttClient(ssid, password, mqttServer, mqttPort, mqttUser, mqttPassword);

// Function prototypes
void readAndProcessData(void *pvParameters);
void publishData(void *pvParameters);

void setup() {
    Serial.begin(115200);

    // Initialize MQTT
    mqttClient.begin();
    mqttClient.subscribeTopic("sensors/raw");
    mqttClient.subscribeTopic("sensors/recalculated");

    // Create FreeRTOS tasks
    xTaskCreate(readAndProcessData, "ReadAndProcessData", 2048, NULL, 1, NULL);
    xTaskCreate(publishData, "PublishData", 2048, NULL, 1, NULL);
}

void loop() {
    mqttClient.loop();
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// Task to read and process sensor data
void readAndProcessData(void *pvParameters) {
    while (1) {
        directionCalculator.update();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Task to publish sensor data
void publishData(void *pvParameters) {
    while (1) {
        // Publish raw data
        char rawData[256];
        snprintf(rawData, sizeof(rawData),
            "{\"north\":%.2f,\"east\":%.2f,\"south\":%.2f,\"west\":%.2f}",
            directionCalculator.rawNorth,
            directionCalculator.rawEast,
            directionCalculator.rawSouth,
            directionCalculator.rawWest
        );
        mqttClient.publishMessage("sensors/raw", rawData);

        // Publish recalculated data
        char recalculatedData[256];
        snprintf(recalculatedData, sizeof(recalculatedData),
            "{\"yaw\":%.2f,\"north\":%.2f,\"east\":%.2f,\"south\":%.2f,\"west\":%.2f}",
            compass.getYaw(),
            directionCalculator.recalculatedNorth,
            directionCalculator.recalculatedEast,
            directionCalculator.recalculatedSouth,
            directionCalculator.recalculatedWest
        );
        mqttClient.publishMessage("sensors/recalculated", recalculatedData);

        vTaskDelay(pdMS_TO_TICKS(5000)); // Publish every 5 seconds
    }
}