#include "DirectionCalculator.h"
#include "GY271_QMC5883L.h"
#include "MqttClient.h"
#include "UltrasonicSensor.h"
#include "lcd_module.h"
#include "secrets.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "uwb.h"

// Define pins for ultrasonic sensors
#define TRIGGER_NORTH 18
#define ECHO_NORTH 19
#define TRIGGER_EAST 25
#define ECHO_EAST 26
#define TRIGGER_SOUTH 27
#define ECHO_SOUTH 32
#define TRIGGER_WEST 14
#define ECHO_WEST 15

// Create sensor objects
UltrasonicSensor northSensor(TRIGGER_NORTH, ECHO_NORTH);
UltrasonicSensor eastSensor(TRIGGER_EAST, ECHO_EAST);
UltrasonicSensor southSensor(TRIGGER_SOUTH, ECHO_SOUTH);
UltrasonicSensor westSensor(TRIGGER_WEST, ECHO_WEST);

// Create compass object
GY271_QMC5883L compass;
LcdModule Lcd;
Uwb uwb;

// Create direction calculator object
DirectionCalculator directionCalculator(northSensor, eastSensor, southSensor, westSensor, compass);
// Create MQTT client object
MqttClient mqttClient(ssid, password, mqttServer, mqttPort, mqttUser, mqttPassword);

// Queue for MQTT messages (if needed)
QueueHandle_t mqttQueue = NULL;

double anchor1_dis;
double anchor2_dis;

// Function prototypes
void readAndProcessData(void* pvParameters);
void publishData(void* pvParameters);
void read_uwb_distance(void* pvParameters);
void mqttLoopTask(void* pvParameters);
void connectToMqtt();

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    ;
    
    uwb.initialize();
    if (!compass.begin()) {
        Serial.println("Failed to initialize compass!");
        while (1)
            ;
    }
    // Set calibration values
    compass.setCalibration(-1379.0, 1683.0, -1694.0, 1195.0);
    Lcd.initialize();

    // Create a queue for MQTT messages (optional)
    mqttQueue = xQueueCreate(10, sizeof(char[256]));
    // Initialize MQTT
    mqttClient.begin();
    mqttClient.subscribeTopic("sensors/raw");
    mqttClient.subscribeTopic("sensors/recalculated");
    mqttClient.subscribeTopic("robot/status");

    
    // Create FreeRTOS tasks
    xTaskCreate(readAndProcessData, "ReadAndProcessData", 3072, NULL, 2, NULL);
    xTaskCreate(publishData, "PublishData", 3072, NULL, 1, NULL);
    xTaskCreate(mqttLoopTask, "MqttLoopTask", 4096, NULL, 1, NULL);
    xTaskCreate(read_uwb_distance, "ReadUwb", 4096, NULL, 3, NULL);
}

void loop()
{
    // Main loop is empty as everything is handled by tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
    Lcd.displaySensorData(directionCalculator.recalculatedNorth, directionCalculator.recalculatedEast,
        directionCalculator.recalculatedSouth, directionCalculator.recalculatedWest, directionCalculator.currentYaw);
}

// Task to handle MQTT loop
void mqttLoopTask(void* pvParameters)
{
    while (1) {
        // if (!mqttClient.isConnected()) {
        //     connectToMqtt();
        // }
        mqttClient.loop();
        vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to prevent hogging CPU
    }
}

// Task to read and process sensor data
void readAndProcessData(void* pvParameters)
{
    while (1) {
            directionCalculator.update();
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz update rate
    }
}

// Task to read and process sensor data
void read_uwb_distance(void* pvParameters)
{
    while (1) {
        double distance1 = uwb.getDistance(0x89); // Get distance from anchor 1
        double distance2 = uwb.getDistance(0x88); // Get distance from anchor 2
        if (distance1 >0.05){
            anchor1_dis = distance1;
        }
        if (distance2 >0.05){
            anchor2_dis = distance2;
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // 10Hz update rate
    }
}

// Task to publish sensor data
void publishData(void* pvParameters)
{
    // Pre-allocate buffers to avoid stack fragmentation
    static char rawData[128]; // Reduced from 256
    static char uwbData[128]; // Reduced from 256
    static char recalculatedData[128]; // Reduced from 256

    while (1) {
        // Publish raw data (simplified format)
        snprintf(rawData, sizeof(rawData), "{\"n\":%.2f,\"e\":%.2f,\"s\":%.2f,\"w\":%.2f}",
            directionCalculator.rawNorth, directionCalculator.rawEast, directionCalculator.rawSouth,
            directionCalculator.rawWest);
        mqttClient.publishMessage("sensors/raw", rawData);

        // Publish recalculated data (simplified format)
        snprintf(recalculatedData, sizeof(recalculatedData), "{\"y\":%.2f,\"n\":%.2f,\"e\":%.2f,\"s\":%.2f,\"w\":%.2f}",
            directionCalculator.currentYaw, directionCalculator.recalculatedNorth, directionCalculator.recalculatedEast,
            directionCalculator.recalculatedSouth, directionCalculator.recalculatedWest

        );
        mqttClient.publishMessage("sensors/recalculated", recalculatedData);
        // Publish UWB data (simplified format)
        snprintf(uwbData, sizeof(uwbData), "{\"a\":%.2f,\"b\":%.2f}",
        anchor1_dis, anchor2_dis);
        mqttClient.publishMessage("sensors/uwb", uwbData);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}