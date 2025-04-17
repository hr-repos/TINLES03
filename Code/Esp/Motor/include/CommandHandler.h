#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <PubSubClient.h>
#include <Arduino.h>
#include <map>
#include <string>
#include <vector>
using namespace std;

#include "Motor.h"
#include "pidController.h"

class CommandHandler {
public:
    struct Motion {
        float m1, m2, m3;
    };

    std::string _extendedCommand;
    
    enum class DriveMode {
        MANUAL,
        OBSTACLE_AVOIDANCE,
        WALL_FOLLOWING,
        ROOM_MAPPING
    };
    
    CommandHandler(PubSubClient& mqttClient, 
                  QueueHandle_t motor1Queue, 
                  QueueHandle_t motor2Queue, 
                  QueueHandle_t motor3Queue,
                  PIDController& headingPID);
    
    // Core functions
    void handleCommand(char command);
    void stopAllMotors();
    void moveOmni(float directionX, float directionY, float rotation = 0);

    
    
    // Sensor and autonomous functions
    void updateHeading(float newHeading);
    void updateSensorData(const char* payload);
    void setDriveMode(DriveMode mode);
    void autonomousDrive();
    
    // Configuration
    void setGlobalSpeed(int speed);
    void setStatusTopic(const char* topic);
    
    // Getters
    int getGlobalSpeed() const { return _globalSpeed; }
    double getCurrentHeading() const { return _currentHeading; }
    DriveMode getDriveMode() const { return _driveMode; }

    float extractFloatValue(const char* payload, const char* key);
    void publishMap();
    void publishStatus();
    void updateMap(float heading, float distance);
    void updateBatteryLevel();

    void setEmergencyStop(bool state);
    bool isEmergencyStop() const;
    
private:
    // Motor control
    void sendMotorCommands(int dir1, int speed1, int dir2, int speed2, int dir3, int speed3);
    void sendMotorCommandsFromMotion(const Motion& m);

    const std::map<std::string, Motion> motions = {
        {"forward",  {0.826f, -0.6f, -1.0f}},
        {"backward", {-0.866f, 0.95f, 1.0f}},
        {"left",     {0.75f, 0.6f, -0.6f}},
        {"right",    {-0.95f, -0.6f, 0.7f}},
        {"rotateL",  {-1.0f, -1.0f, -1.0f}},
        {"rotateR",  {1.0f, 1.0f, 1.0f}}
    };
    
    // Autonomous behaviors
    void avoidObstacles();
    void followRightWall();
    void followLeftWall();
    
    // Components
    PubSubClient& _mqttClient;
    QueueHandle_t _motor1Queue;
    QueueHandle_t _motor2Queue;
    QueueHandle_t _motor3Queue;
    PIDController& _headingPID;
    
    // State
    bool emergencyStop = false;
    int _globalSpeed = 255;
    float _currentDirectionX = 0;
    float _currentDirectionY = 0;
    double _currentHeading = 0;
    double _targetHeading = 0;
    bool _isMovingStraight = false;
    DriveMode _driveMode = DriveMode::MANUAL;
    const char* _statusTopic = "robot/status";

    bool _expectingExtendedCommand = false;
    unsigned long _extendedCommandTimeout = 0;
    
    // Sensor data
    float _sensorNorth = 0;  // Front
    float _sensorEast = 0;   // Right
    float _sensorSouth = 0;  // Back
    float _sensorWest = 0;   // Left
    
    // Parameters
    const float SAFE_DISTANCE = 30.0f;    // cm
    const float TOO_CLOSE = 15.0f;        // cm
    const float WALL_DISTANCE = 25.0f;    // cm
    const float MAX_SPEED = 1.0f;
    const float MIN_SPEED = 0.6f;

    // Mapping variables
    std::vector<std::pair<float, float>> _explorationPath;
    unsigned long _lastMapUpdate = 0;
    const float MAP_RESOLUTION = 10.0f; // cm per grid cell

    float _batteryLevel = 100.0;
    unsigned long _lastBatteryUpdate = 0;
    const unsigned long BATTERY_UPDATE_INTERVAL = 60000; // 1 minute
    
    
    float readBatteryVoltage();
    void publishBatteryLevel();
    
    // Mapping methods
    void exploreRoom();
    void updateMap(float x, float y, float obstacleDistance);
    bool isUnexploredDirection(float angle);
};

#endif