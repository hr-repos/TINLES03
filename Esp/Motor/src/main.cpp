#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <Wire.h>
#include "Motor.h"
#include "pidController.h"
#include "CommandHandler.h"
#include "MqttClient.h"
#include "secrets.h"

// I2C Address for master device
#define I2C_MASTER_ADDRESS 0x40

// Motor Pins
#define MOTOR1_ENA 32
#define MOTOR1_IN1 33
#define MOTOR1_IN2 25
#define MOTOR2_ENB 27
#define MOTOR2_IN1 26
#define MOTOR2_IN2 14
#define MOTOR3_ENC 4
#define MOTOR3_IN1 13
#define MOTOR3_IN2 12

// Battery monitoring
#define BATTERY_PIN 35
#define BATTERY_UPDATE_INTERVAL 5000

// Motor objects
Motor motor1(MOTOR1_ENA, MOTOR1_IN1, MOTOR1_IN2);
Motor motor2(MOTOR2_ENB, MOTOR2_IN1, MOTOR2_IN2);
Motor motor3(MOTOR3_ENC, MOTOR3_IN1, MOTOR3_IN2);

// PID Controller
PIDController headingPID(0.8, 0.015, 0.05, -0.3, 0.3);

// Command handler
CommandHandler* commandHandler = nullptr;

// MQTT Client
MqttClient mqttClient(ssid, password, mqttServer, mqttPort, mqttUser, mqttPassword);

// FreeRTOS components
QueueHandle_t motor1Queue;
QueueHandle_t motor2Queue;
QueueHandle_t motor3Queue;
TaskHandle_t motor1TaskHandle = NULL;
TaskHandle_t motor2TaskHandle = NULL;
TaskHandle_t motor3TaskHandle = NULL;
TaskHandle_t commandTaskHandle = NULL;
TaskHandle_t i2cTaskHandle = NULL;
TaskHandle_t mqttTaskHandle = NULL;

// Sensor data from master
struct SensorData {
    float north;
    float east;
    float south;
    float west;
    float yaw;
} sensorData;

// Battery level
float batteryLevel = 0.0f;

// Motor tasks (unchanged from original)
void motorTask1(void *pvParameters) {
    MotorCommand cmd = {0, 0};
    while (1) {
        if (xQueueReceive(motor1Queue, &cmd, portMAX_DELAY)) {
            motor1.updateMotor(cmd);
        }
    }
}

void motorTask2(void *pvParameters) {
    MotorCommand cmd = {0, 0};
    while (1) {
        if (xQueueReceive(motor2Queue, &cmd, portMAX_DELAY)) {
            motor2.updateMotor(cmd);
        }
    }
}

void motorTask3(void *pvParameters) {
    MotorCommand cmd = {0, 0};
    while (1) {
        if (xQueueReceive(motor3Queue, &cmd, portMAX_DELAY)) {
            motor3.updateMotor(cmd);
        }
    }
}

// Command task (handles MQTT commands)
void commandTask(void *pvParameters) {
    while (1) {
        // Commands are handled via MQTT callback
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// I2C Task - Gets sensor data from master
void i2cTask(void *pvParameters) {
    while (1) {
        Wire.requestFrom(I2C_MASTER_ADDRESS, sizeof(SensorData));
        
        if (Wire.available() == sizeof(SensorData)) {
            Wire.readBytes((uint8_t*)&sensorData, sizeof(SensorData));
            
            if (commandHandler) {
                commandHandler->updateHeading(sensorData.yaw);
                
                char sensorJson[128];
                snprintf(sensorJson, sizeof(sensorJson),
                    "{\"y\":%.2f,\"n\":%.2f,\"e\":%.2f,\"s\":%.2f,\"w\":%.2f}",
                    sensorData.yaw, sensorData.north, sensorData.east, 
                    sensorData.south, sensorData.west);
                    
                commandHandler->updateSensorData(sensorJson);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz update
    }
}

// Battery Task - Sends battery level to master
void batteryTask(void *pvParameters) {
    while (1) {
        // Read battery voltage (assuming voltage divider)
        int rawValue = analogRead(BATTERY_PIN);
        batteryLevel = (rawValue / 4095.0) * 3.3 * 2; // Adjust for your divider ratio
        
        // Send to master via I2C
        Wire.beginTransmission(I2C_MASTER_ADDRESS);
        Wire.write((uint8_t*)&batteryLevel, sizeof(float));
        Wire.endTransmission();
        
        vTaskDelay(pdMS_TO_TICKS(BATTERY_UPDATE_INTERVAL));
    }
}

// MQTT Task
void mqttTask(void *pvParameters) {
    mqttClient.begin();
    mqttClient.subscribeTopic("robot/command");
    
    while (1) {
        mqttClient.loop();
        
        // Publish status periodically
        static TickType_t lastPublish = 0;
        if (xTaskGetTickCount() - lastPublish > pdMS_TO_TICKS(2000)) {
            if (commandHandler) {
                const char* modeStr = "";
                switch(commandHandler->getDriveMode()) {
                    case CommandHandler::DriveMode::MANUAL: modeStr = "MANUAL"; break;
                    case CommandHandler::DriveMode::OBSTACLE_AVOIDANCE: modeStr = "AVOIDANCE"; break;
                    case CommandHandler::DriveMode::WALL_FOLLOWING: modeStr = "WALL_FOLLOW"; break;
                }
                
                char status[128];
                snprintf(status, sizeof(status),
                    "{\"mode\":\"%s\",\"speed\":%d,\"heading\":%.1f,\"battery\":%.2f}",
                    modeStr, commandHandler->getGlobalSpeed(), 
                    commandHandler->getCurrentHeading(), batteryLevel);
                
                mqttClient.publishMessage("robot/status", status);
            }
            lastPublish = xTaskGetTickCount();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// MQTT Callback
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, "robot/command") == 0 && commandHandler) {
        char cmd = (length > 0) ? (char)payload[0] : ' ';
        commandHandler->handleCommand(cmd);
    }
}

void setup() {
    Serial.begin(115200);
    
    // Initialize I2C
    Wire.begin();
    
    // Initialize battery monitoring
    pinMode(BATTERY_PIN, INPUT);
    
    // Initialize PWM and motors
    ledcSetup(0, 1000, 8);
    ledcAttachPin(MOTOR1_ENA, 0);
    ledcSetup(1, 1000, 8);
    ledcAttachPin(MOTOR2_ENB, 1);
    ledcSetup(2, 1000, 8);
    ledcAttachPin(MOTOR3_ENC, 2);
    motor1.begin();
    motor2.begin();
    motor3.begin();

    // Create queues
    motor1Queue = xQueueCreate(5, sizeof(MotorCommand));
    motor2Queue = xQueueCreate(5, sizeof(MotorCommand));
    motor3Queue = xQueueCreate(5, sizeof(MotorCommand));

    // Create command handler
    commandHandler = new CommandHandler(motor1Queue, motor2Queue, motor3Queue, headingPID);
    commandHandler->setGlobalSpeed(200);

    // Set MQTT callback
    mqttClient.setCallback(mqttCallback);

    // Create tasks
    xTaskCreate(motorTask1, "Motor1", 2048, NULL, 2, &motor1TaskHandle);
    xTaskCreate(motorTask2, "Motor2", 2048, NULL, 2, &motor2TaskHandle);
    xTaskCreate(motorTask3, "Motor3", 2048, NULL, 2, &motor3TaskHandle);
    xTaskCreate(commandTask, "Command", 2048, NULL, 2, &commandTaskHandle);
    xTaskCreate(i2cTask, "I2C", 2048, NULL, 3, &i2cTaskHandle);
    xTaskCreate(batteryTask, "Battery", 2048, NULL, 2, NULL);
    xTaskCreate(mqttTask, "MQTT", 4096, NULL, 2, &mqttTaskHandle);
    
    Serial.println("Slave setup complete");
}

void loop() {
    vTaskDelete(NULL);
}