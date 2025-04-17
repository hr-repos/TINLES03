#include <Arduino.h>
#include "Motor.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "pidController.h"
#include "MqttClient.h"
#include "esp_task_wdt.h"
#include "secrets.h"
#include "CommandHandler.h"

// FreeRTOS Queue Handles
QueueHandle_t motor1Queue;
QueueHandle_t motor2Queue;
QueueHandle_t motor3Queue;

// Task Handles
TaskHandle_t wifiMqttTaskHandle = NULL;
TaskHandle_t magnetometerTaskHandle = NULL;
TaskHandle_t statusTaskHandle = NULL;

// Semaphore Handle
SemaphoreHandle_t magSemaphore = NULL;

// Constants
#define MAG_FILTER_SIZE 5
const unsigned long magPrintInterval = 3000;

// MQTT Topics
const char *cmd_topic = "robot/command";
const char *status_topic = "robot/status";
const char *mag_topic = "sensors/recalculated";

// Motor Pins (L298N)
#define MOTOR1_ENA 32
#define MOTOR1_IN1 33
#define MOTOR1_IN2 25
#define MOTOR2_ENB 27
#define MOTOR2_IN1 26
#define MOTOR2_IN2 14
#define MOTOR3_ENC 4
#define MOTOR3_IN1 13
#define MOTOR3_IN2 12
#define STOPBUTTON 35

// Sleep variables
volatile unsigned long lastCommandTime = 0;
const unsigned long sleepTimeout = 60000; // 1 minute
bool isAsleep = false;

// Motor objects
Motor motor1(MOTOR1_ENA, MOTOR1_IN1, MOTOR1_IN2);
Motor motor2(MOTOR2_ENB, MOTOR2_IN1, MOTOR2_IN2);
Motor motor3(MOTOR3_ENC, MOTOR3_IN1, MOTOR3_IN2);

// PID Controller
PIDController headingPID(-0.8, -0.015, -0.03, -0.5, 0.5);

// Command handler instance
CommandHandler* commandHandler = nullptr;

// MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);
MqttClient mqttClient(ssid, password, mqttServer, mqttPort, mqttUser, mqttPassword);

volatile bool wifiConnected = false;
volatile bool mqttConnected = false;

volatile bool stopRequested = false;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

unsigned long lastStopTime = 0;
const unsigned long stopDebounceTime = 500; // ms

void IRAM_ATTR stopButtonISR() {
    static unsigned long lastInterruptTime = 0;
    unsigned long interruptTime = millis();
    
    if (interruptTime - lastInterruptTime > stopDebounceTime) {
        stopRequested = true;
    }
    lastInterruptTime = interruptTime;
}

void prepareForSleep() {
    Serial.println("Preparing for sleep...");
    commandHandler->stopAllMotors();
    
    motor1.sleep();
    motor2.sleep();
    motor3.sleep();
    
    WiFi.disconnect(true);
    isAsleep = true;
    Serial.println("Entered sleep mode");
}

void wakeUpSystem() {
    Serial.println("Waking up system...");
    isAsleep = false;
    
    WiFi.begin(ssid, password);
    lastCommandTime = millis();
    
    motor1.wake();
    motor2.wake();
    motor3.wake();
}

// Motor tasks (unchanged)
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

void batteryTask(void *pvParameters) {
    const TickType_t xFrequency = 10000 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while(1) {
        if (!isAsleep) {
            commandHandler->updateBatteryLevel();
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void setup_wifi() {
    delay(10);
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length) {
    esp_task_wdt_reset();
    char payloadStr[length + 1];
    memcpy(payloadStr, payload, length);
    payloadStr[length] = '\0';

    // Wake up if asleep
    if (isAsleep) {
        wakeUpSystem();
    }

    // Update last command time
    lastCommandTime = millis();

    if (strcmp(topic, cmd_topic) == 0) {
        commandHandler->handleCommand(payloadStr[0]);
        
        if (payloadStr[0] == ' ') {
            prepareForSleep();
        }
    }
    else if (strcmp(topic, mag_topic) == 0) {
        char *yPtr = strstr(payloadStr, "\"y\":");
        float heading = 0.0;

        if (yPtr) {
            sscanf(yPtr, "\"y\":%f", &heading);
            commandHandler->updateHeading(heading);
            commandHandler->updateSensorData(payloadStr);
        }
    }
    esp_task_wdt_reset();
}

void wifiMqttTask(void *pvParameters) {
    setup_wifi();
    wifiConnected = true;

    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);

    while (1) {
        if (!isAsleep) {
            if (WiFi.status() != WL_CONNECTED) {
                wifiConnected = false;
                mqttConnected = false;
                Serial.println("WiFi disconnected! Reconnecting...");
                setup_wifi();
                wifiConnected = true;
                continue;
            }

            if (!client.connected()) {
                mqttConnected = false;
                Serial.println("Attempting MQTT connection...");
                if (client.connect("ESP32Robot", status_topic, 1, true, "Robot disconnected")) {
                    mqttConnected = true;
                    Serial.println("MQTT connected");
                    client.subscribe(cmd_topic);
                    client.subscribe(mag_topic);
                    client.publish(status_topic, "Robot connected", true);
                }
            }

            if (mqttConnected) {
                client.loop();
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void magnetometerTask(void *pvParameters) {
    esp_task_wdt_add(NULL);
    const TickType_t xFrequency = 100 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        if (!isAsleep) {
            static unsigned long lastPrint = 0;
            if (millis() - lastPrint > magPrintInterval) {
                lastPrint = millis();
                if (commandHandler) {
                    Serial.printf("Current Mode: %d | Heading: %.1f°\n",
                                static_cast<int>(commandHandler->getDriveMode()),
                                commandHandler->getCurrentHeading());
                }
            }
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        esp_task_wdt_reset();
    }
}

void statusTask(void *pvParameters) {
    esp_task_wdt_add(NULL);
    const TickType_t xFrequency = 2000 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        if (!isAsleep && client.connected() && commandHandler) {
            String modeStr;
            switch(commandHandler->getDriveMode()) {
                case CommandHandler::DriveMode::MANUAL: modeStr = "MANUAL"; break;
                case CommandHandler::DriveMode::OBSTACLE_AVOIDANCE: modeStr = "AVOIDANCE"; break;
                case CommandHandler::DriveMode::WALL_FOLLOWING: modeStr = "WALL_FOLLOW"; break;
            }
            
            String status = "Mode:" + modeStr + 
                          " | Speed:" + String(commandHandler->getGlobalSpeed()) +
                          " | Heading:" + String(commandHandler->getCurrentHeading(), 1) + "°";
            client.publish(status_topic, status.c_str());
        }
        esp_task_wdt_reset();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void setup() {
    stopRequested = false;
    Serial.begin(115200);
    Wire.begin();
    esp_task_wdt_init(10, true);

    pinMode(STOPBUTTON, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(STOPBUTTON), stopButtonISR, FALLING);

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

    // Create semaphore
    magSemaphore = xSemaphoreCreateMutex();

    // Initialize command handler
    commandHandler = new CommandHandler(client, motor1Queue, motor2Queue, motor3Queue, headingPID);
    commandHandler->setStatusTopic(status_topic);
    commandHandler->setGlobalSpeed(255);

    // Create tasks
    xTaskCreate(motorTask1, "Motor1", 2048, NULL, 2, NULL);
    xTaskCreate(motorTask2, "Motor2", 2048, NULL, 2, NULL);
    xTaskCreate(motorTask3, "Motor3", 2048, NULL, 2, NULL);
    xTaskCreate(wifiMqttTask, "WiFiMQTT", 4096, NULL, 3, &wifiMqttTaskHandle);
    xTaskCreate(magnetometerTask, "Mag", 2048, NULL, 2, &magnetometerTaskHandle);
    xTaskCreate(statusTask, "Status", 2048, NULL, 1, &statusTaskHandle);
    xTaskCreate(batteryTask, "Battery", 2048, NULL, 1, NULL);

    lastCommandTime = millis();
    Serial.println("Setup complete - Robot ready");
    esp_task_wdt_add(NULL);
}

void loop() {
    if (!isAsleep && (millis() - lastCommandTime > sleepTimeout)) {
        prepareForSleep();
    }

    if (stopRequested) {
        stopRequested = false;
        lastStopTime = millis();
        commandHandler->stopAllMotors();
        Serial.println("Emergency stop activated!");
    }

    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
}