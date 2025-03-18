#include <Arduino.h>
#include "MqttClient.h"

// Define your WiFi and MQTT settings
const char* ssid = "Steehouwer";
const char* password = "rovekyku";
const char* mqttServer = "192.168.11.131";
const int mqttPort = 1883;
const char* mqttUser = "NULL";  // Set NULL if no auth
const char* mqttPassword = "NULL";  // Set NULL if no auth

MqttClient mqttClient(ssid, password, mqttServer, mqttPort, mqttUser, mqttPassword);

void setup() {
    Serial.begin(115200);
    mqttClient.begin();

    // Subscribe to topic
    mqttClient.subscribeTopic("test/topic");
}

void loop() {
    mqttClient.loop();

    // Publish every 5 seconds
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish > 5000) {
        lastPublish = millis();
        mqttClient.publishMessage("test/topic", "Hello from ESP32!");
    }
}