#include "MqttClient.h"

MqttClient::MqttClient(const char* ssid, const char* password, const char* mqttServer, int mqttPort, 
                       const char* mqttUser, const char* mqttPassword)
    : _ssid(ssid), _password(password), _mqttServer(mqttServer), _mqttPort(mqttPort),
      _mqttUser(mqttUser), _mqttPassword(mqttPassword), _client(_espClient) {
    _client.setCallback(callback);
}

void MqttClient::begin() {
    connectWiFi();
    _client.setServer(_mqttServer, _mqttPort);
    connectMQTT();
}

void MqttClient::connectWiFi() {
    Serial.print("Connecting to WiFi...");
    Serial.printf("Connecting to WiFi... SSID: %s, Password: %s\n", _ssid, _password);

    WiFi.begin(_ssid, _password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
}

void MqttClient::connectMQTT() {
    while (!_client.connected()) {
        Serial.print("Connecting to MQTT...");
        if (_client.connect("ESP32_Client", _mqttUser, _mqttPassword)) {
            Serial.println("Connected!");
            _client.subscribe("test/topic"); // Subscribe after successful connection
        } else {
            Serial.print("Failed, rc=");
            Serial.print(_client.state());
            Serial.println(" Retrying in 5s...");
            delay(5000);
        }
    }
}

void MqttClient::loop() {
    if (!_client.connected()) {
        connectMQTT();
    }
    _client.loop();
}

void MqttClient::publishMessage(const char* topic, const char* message) {
    Serial.print("Publishing: ");
    Serial.print(topic);
    Serial.print(" -> ");
    Serial.println(message);
    _client.publish(topic, message);
}

void MqttClient::subscribeTopic(const char* topic) {
    _client.subscribe(topic);
    Serial.print("Subscribed to topic: ");
    Serial.println(topic);
}

bool MqttClient::isConnected() {
    return _client.connected();
}

// MQTT Message Callback Function
void MqttClient::callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}