#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "UltrasonicSensor.h"
#include "DirectionCalculator.h"
#include "MqttClient.h"
#include "GY271_QMC5883L.h"
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

// Create compass object
GY271_QMC5883L compass;

// Create direction calculator object
DirectionCalculator directionCalculator(northSensor, eastSensor, southSensor, westSensor, compass);
// Create MQTT client object
MqttClient mqttClient(ssid, password, mqttServer, mqttPort, mqttUser, mqttPassword);

// Queue for MQTT messages (if needed)
QueueHandle_t mqttQueue = NULL;

// Function prototypes
void readAndProcessData(void *pvParameters);
void publishData(void *pvParameters);
void mqttLoopTask(void *pvParameters);
void connectToMqtt();

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    if (!compass.begin()) {
        Serial.println("Failed to initialize compass!");
        while(1);
    }
    // Set calibration values (you should calibrate your own)
    compass.setCalibration(-1379.0, 1683.0, -1694.0, 1195.0);
    
    // Create a queue for MQTT messages (optional)
    mqttQueue = xQueueCreate(10, sizeof(char[256]));
    // Initialize MQTT
    mqttClient.begin();
    mqttClient.subscribeTopic("sensors/raw");
    mqttClient.subscribeTopic("sensors/recalculated");

    // Create FreeRTOS tasks
    xTaskCreate(readAndProcessData, "ReadAndProcessData", 3072, NULL, 2, NULL);
    xTaskCreate(publishData, "PublishData", 3072, NULL, 1, NULL);
    xTaskCreate(mqttLoopTask, "MqttLoopTask", 4096, NULL, 1, NULL);
}

void loop() {
    // Main loop is empty as everything is handled by tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// Task to handle MQTT loop
void mqttLoopTask(void *pvParameters) {
    while (1) {
        // if (!mqttClient.isConnected()) {
        //     connectToMqtt();
        // }
        mqttClient.loop();
        vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to prevent hogging CPU
    }
}

// Task to read and process sensor data
void readAndProcessData(void *pvParameters) {
    while (1) {
        directionCalculator.update();
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz update rate
    }
}

// Task to publish sensor data
void publishData(void *pvParameters) {
    // Pre-allocate buffers to avoid stack fragmentation
    static char rawData[128];         // Reduced from 256
    static char recalculatedData[128]; // Reduced from 256
    
    while (1) {
        // Publish raw data (simplified format)
        snprintf(rawData, sizeof(rawData),
            "{\"n\":%.2f,\"e\":%.2f,\"s\":%.2f,\"w\":%.2f}",
            directionCalculator.rawNorth,
            directionCalculator.rawEast,
            directionCalculator.rawSouth,
            directionCalculator.rawWest
        );
        mqttClient.publishMessage("sensors/raw", rawData);

        // Publish recalculated data (simplified format)
        snprintf(recalculatedData, sizeof(recalculatedData),
            "{\"y\":%.2f,\"n\":%.2f,\"e\":%.2f,\"s\":%.2f,\"w\":%.2f}",
            directionCalculator.currentYaw,
            directionCalculator.recalculatedNorth,
            directionCalculator.recalculatedEast,
            directionCalculator.recalculatedSouth,
            directionCalculator.recalculatedWest
            
        );
        mqttClient.publishMessage("sensors/recalculated", recalculatedData);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}