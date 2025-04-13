#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Wire.h>
#include "UltrasonicSensor.h"
#include "DirectionCalculator.h"
#include "GY271_QMC5883L.h"
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

// I2C Address for this master device
#define I2C_SLAVE_ADDRESS 0x40

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

// Battery level from slave
float slaveBatteryLevel = 0.0f;

// FreeRTOS Task Handles
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t mqttTaskHandle = NULL;
TaskHandle_t i2cTaskHandle = NULL;

// I2C data buffer
uint8_t i2cDataBuffer[20]; // 5 floats (n,e,s,w,yaw)

// MQTT callback
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Handle MQTT messages from web interface
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    Serial.printf("MQTT Message: %s -> %s\n", topic, message);
}

// Sensor Task
void sensorTask(void *pvParameters) {
    while (1) {
        directionCalculator.update();
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz update
    }
}

// MQTT Task
void mqttTask(void *pvParameters) {
    mqttClient.begin();
    mqttClient.subscribeTopic("robot/command");
    
    while (1) {
        mqttClient.loop();
        
        // Publish sensor data periodically
        static TickType_t lastPublish = 0;
        if (xTaskGetTickCount() - lastPublish > pdMS_TO_TICKS(1000)) {
            char payload[128];
            snprintf(payload, sizeof(payload),
                "{\"n\":%.2f,\"e\":%.2f,\"s\":%.2f,\"w\":%.2f,\"y\":%.2f,\"bat\":%.2f}",
                directionCalculator.recalculatedNorth,
                directionCalculator.recalculatedEast,
                directionCalculator.recalculatedSouth,
                directionCalculator.recalculatedWest,
                directionCalculator.currentYaw,
                slaveBatteryLevel);
            
            mqttClient.publishMessage("robot/sensors", payload);
            lastPublish = xTaskGetTickCount();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// I2C Task
void i2cTask(void *pvParameters) {
    Wire.begin(I2C_SLAVE_ADDRESS);
    Wire.onRequest([](){
        // Update I2C buffer with latest sensor data
        float* floatData = (float*)i2cDataBuffer;
        directionCalculator.getSensorData(floatData[0], floatData[1], 
                                        floatData[2], floatData[3], floatData[4]);
        Wire.write(i2cDataBuffer, sizeof(i2cDataBuffer));
    });
    
    Wire.onReceive([](int byteCount){
        if (byteCount == sizeof(float)) {
            Wire.readBytes((uint8_t*)&slaveBatteryLevel, sizeof(float));
            Serial.printf("Battery level from slave: %.2fV\n", slaveBatteryLevel);
        }
    });
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    if (!compass.begin()) {
        Serial.println("Failed to initialize compass!");
        while(1);
    }
    compass.setCalibration(-1379.0, 1683.0, -1694.0, 1195.0);
    
    // Create tasks
    xTaskCreate(sensorTask, "SensorTask", 4096, NULL, 2, &sensorTaskHandle);
    xTaskCreate(mqttTask, "MqttTask", 4096, NULL, 2, &mqttTaskHandle);
    xTaskCreate(i2cTask, "I2CTask", 2048, NULL, 3, &i2cTaskHandle);
    
    Serial.println("Master setup complete");
}

void loop() {
    vTaskDelete(NULL); // FreeRTOS tasks handle everything
}