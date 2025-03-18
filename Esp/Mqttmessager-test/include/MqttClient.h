#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <WiFi.h>
#include <PubSubClient.h>

class MqttClient {
public:
    MqttClient(const char* ssid, const char* password, const char* mqttServer, int mqttPort, 
               const char* mqttUser = nullptr, const char* mqttPassword = nullptr);

    void begin();
    void loop();
    void publishMessage(const char* topic, const char* message);
    void subscribeTopic(const char* topic);
    bool isConnected();

private:
    void connectWiFi();
    void connectMQTT();
    static void callback(char* topic, byte* payload, unsigned int length); // Callback function

    const char* _ssid;
    const char* _password;
    const char* _mqttServer;
    int _mqttPort;
    const char* _mqttUser;
    const char* _mqttPassword;

    WiFiClient _espClient;
    PubSubClient _client;
};

#endif
