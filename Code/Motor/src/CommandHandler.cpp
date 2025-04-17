#include "CommandHandler.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAX_ADC 4095.0
#define REFERENCE_VOLTAGE 3.3
#define VOLTAGE_DIVIDER_RATIO 3.0
#define BATTERY_PIN 34


// Constructor
CommandHandler::CommandHandler(PubSubClient& mqttClient, 
                             QueueHandle_t motor1Queue, 
                             QueueHandle_t motor2Queue, 
                             QueueHandle_t motor3Queue,
                             PIDController& headingPID) :
    _mqttClient(mqttClient),
    _motor1Queue(motor1Queue),
    _motor2Queue(motor2Queue),
    _motor3Queue(motor3Queue),
    _headingPID(headingPID) {}

// Handle incoming commands
void CommandHandler::handleCommand(char command) {
    // Handle mode commands (0-3)
    if (command >= '0' && command <= '3') {
        switch(command) {
            case '0': setDriveMode(DriveMode::MANUAL); break;
            case '1': setDriveMode(DriveMode::OBSTACLE_AVOIDANCE); break;
            case '2': setDriveMode(DriveMode::WALL_FOLLOWING); break;
            case '3': setDriveMode(DriveMode::ROOM_MAPPING); break;
        }
        publishStatus();
        return;
    }

    if (_driveMode != DriveMode::MANUAL && command != ' ') {
        return;
    }

    // Handle movement commands
    switch(command) {
        case 'w': // Forward
            _targetHeading = _currentHeading;    
            _headingPID.setTarget(_targetHeading);
            _isMovingStraight = true;
            _currentDirectionX = 0;
            _currentDirectionY = 1;
            moveOmni(0, -1);
            break;

        case 's': // Backward
            _targetHeading = _currentHeading;
            _headingPID.setTarget(_targetHeading);
            _isMovingStraight = true;
            _currentDirectionX = 0;
            _currentDirectionY = -1;
            moveOmni(0, 1);
            break;

        case 'a': // Left
            _targetHeading = _currentHeading;
            _headingPID.setTarget(_targetHeading);
            _isMovingStraight = true;
            _currentDirectionX = -1;
            _currentDirectionY = 0;
            moveOmni(1, 0);
            break;

        case 'd': // Right
            _targetHeading = _currentHeading;
            _headingPID.setTarget(_targetHeading);
            _isMovingStraight = true;
            _currentDirectionX = 1;
            _currentDirectionY = 0;
            moveOmni(-1, 0);
            break;
            
        case 'q': // Rotate left
            _isMovingStraight = false;
            moveOmni(0, 0, -1);
            break;
            
        case 'e': // Rotate right
            _isMovingStraight = false;
            moveOmni(0, 0, 1);
            break;

        case ' ': // Stop
            stopAllMotors();
            break;
    }

    publishStatus();
}

// Motor control functions
void CommandHandler::stopAllMotors() {
    MotorCommand cmd = {0, 0};
    xQueueSend(_motor1Queue, &cmd, 0);
    xQueueSend(_motor2Queue, &cmd, 0);
    xQueueSend(_motor3Queue, &cmd, 0);
    _isMovingStraight = false;
    //esp_light_sleep_start();
}

// Helper function to send commands from Motion struct
void CommandHandler::sendMotorCommandsFromMotion(const Motion& m) {
    sendMotorCommands(
        (m.m1 > 0) ? 1 : -1, static_cast<int>(fabs(m.m1) * _globalSpeed),
        (m.m2 > 0) ? 1 : -1, static_cast<int>(fabs(m.m2) * _globalSpeed),
        (m.m3 > 0) ? 1 : -1, static_cast<int>(fabs(m.m3) * _globalSpeed)
    );
}

void CommandHandler::moveOmni(float directionX, float directionY, float rotation) {
    // First check if this matches any predefined motion
    if (fabs(rotation) < 0.01f) {
        if (directionY < -0.9f && fabs(directionX) < 0.1f) {
            // Forward
            sendMotorCommandsFromMotion(motions.at("forward"));
            return;
        }
        else if (directionY > 0.9f && fabs(directionX) < 0.1f) {
            // Backward
            sendMotorCommandsFromMotion(motions.at("backward"));
            return;
        }
        else if (directionX > 0.9f && fabs(directionY) < 0.1f) {
            // Left
            sendMotorCommandsFromMotion(motions.at("left"));
            return;
        }
        else if (directionX < -0.9f && fabs(directionY) < 0.1f) {
            // Right
            sendMotorCommandsFromMotion(motions.at("right"));
            return;
        }
    }
    else if (fabs(directionX) < 0.1f && fabs(directionY) < 0.1f) {
        if (rotation < -0.9f) {
            // Rotate Left
            sendMotorCommandsFromMotion(motions.at("rotateL"));
            return;
        }
        else if (rotation > 0.9f) {
            // Rotate Right
            sendMotorCommandsFromMotion(motions.at("rotateR"));
            return;
        }
    }

    // Fall back to omnidirectional calculation for other cases
    const float motorComp[3] = {1.0f, 0.6f, 1.0f};
    Motion m = {0, 0, 0};

    // Apply PID correction if needed
    if (_isMovingStraight && fabs(rotation) > 0.01f) {
        rotation = constrain(rotation * 0.5f, -0.3f, 0.3f);
    }

    // General omnidirectional movement
    m.m1 = (-0.5f * directionX + 0.866f * directionY + rotation) * motorComp[0];
    m.m2 = (0.5f * directionX - 0.866f * directionY + rotation) * motorComp[1];
    m.m3 = (directionX + rotation - (0.1f * directionY)) * motorComp[2];

    // Normalize
    float maxVal = max(max(fabs(m.m1), fabs(m.m2)), fabs(m.m3));
    if (maxVal > 1.0f) {
        m.m1 /= maxVal;
        m.m2 /= maxVal;
        m.m3 /= maxVal;
    }

    sendMotorCommands(
        (m.m1 > 0) ? 1 : -1, static_cast<int>(fabs(m.m1) * _globalSpeed),
        (m.m2 > 0) ? 1 : -1, static_cast<int>(fabs(m.m2) * _globalSpeed),
        (m.m3 > 0) ? 1 : -1, static_cast<int>(fabs(m.m3) * _globalSpeed)
    );
}

void CommandHandler::sendMotorCommands(int dir1, int speed1, int dir2, int speed2, int dir3, int speed3) {
    MotorCommand cmd1 = {dir1, speed1};
    MotorCommand cmd2 = {dir2, speed2};
    MotorCommand cmd3 = {dir3, speed3};
    
    xQueueSend(_motor1Queue, &cmd1, portMAX_DELAY);
    xQueueSend(_motor2Queue, &cmd2, portMAX_DELAY);
    xQueueSend(_motor3Queue, &cmd3, portMAX_DELAY);
}

// Sensor data processing
void CommandHandler::updateHeading(float newHeading) {
    _currentHeading = newHeading;

    if (_isMovingStraight) {
        double error = _currentHeading - _targetHeading;
        if (fabs(error) > 5.0) {
            double correction = _headingPID.compute(_currentHeading);
            moveOmni(_currentDirectionX, _currentDirectionY, correction);
        } else {
            moveOmni(_currentDirectionX, _currentDirectionY, 0);
        }
    }
}

void CommandHandler::updateSensorData(const char* payload) {
    // Manual JSON parsing for {"y":5.35,"n":8.54,"e":32.80,"s":1187.05,"w":5.68}
    _sensorNorth = extractFloatValue(payload, "\"n\":");
    _sensorEast = extractFloatValue(payload, "\"e\":");
    _sensorWest = extractFloatValue(payload, "\"w\":");
    float yaw = extractFloatValue(payload, "\"y\":");
    
    updateHeading(yaw);
    
    if (_driveMode != DriveMode::MANUAL) {
        autonomousDrive();
    }
}

float CommandHandler::extractFloatValue(const char* payload, const char* key) {
    const char* ptr = strstr(payload, key);
    if (ptr) {
        return atof(ptr + strlen(key));
    }
    return 0.0f;
}

// Autonomous modes
void CommandHandler::setDriveMode(DriveMode mode) {
    if (mode == DriveMode::ROOM_MAPPING) {
        _explorationPath.clear();
        _explorationPath.emplace_back(0, 0); // Start at origin
    }
    _driveMode = mode;
    stopAllMotors();
    publishStatus();
}

void CommandHandler::autonomousDrive() {
    switch (_driveMode) {
        case DriveMode::OBSTACLE_AVOIDANCE:
            avoidObstacles();
            break;
        case DriveMode::WALL_FOLLOWING:
            followRightWall();
            break;
        case DriveMode::ROOM_MAPPING:
            exploreRoom();
            break;
        default:
            break;
    }
}

void CommandHandler::avoidObstacles() {
    if (_sensorNorth > SAFE_DISTANCE) {
        moveOmni(0, MAX_SPEED);
    } 
    else if (_sensorNorth < TOO_CLOSE) {
        moveOmni(0, -MAX_SPEED/2);
        delay(300);
    }
    else {
        if (_sensorEast > _sensorWest) {
            moveOmni(0, MIN_SPEED, 0.7);
        } else {
            moveOmni(0, MIN_SPEED, -0.7);
        }
        delay(200);
    }
}

void CommandHandler::followRightWall() {
    float error = _sensorEast - WALL_DISTANCE;
    float steering = constrain(error * 0.03f, -0.3f, 0.3f);
    float forwardSpeed = (_sensorNorth > SAFE_DISTANCE) ? MAX_SPEED : 
                       (_sensorNorth > TOO_CLOSE) ? MAX_SPEED * 0.7f : 0;
    moveOmni(0, forwardSpeed, steering);
}

// Room mapping implementation
void CommandHandler::exploreRoom() {
    static enum State { FORWARD, TURN_RIGHT, DECIDE } state = DECIDE;
    static unsigned long turnStart = 0;

    // Emergency stop
    if (_sensorNorth < TOO_CLOSE) {
        moveOmni(0, -0.9);
        delay(300);
        state = DECIDE;
        return;
    }

    switch(state) {
        case FORWARD:
            if (_sensorNorth > SAFE_DISTANCE) {
                moveOmni(0, -0.9);
                updateMap(_currentHeading, _sensorNorth);
            } else {
                state = TURN_RIGHT;
                turnStart = millis();
            }
            break;

        case TURN_RIGHT:
            moveOmni(0, 0, 0.8);
            if (millis() - turnStart > 500) {
                state = DECIDE;
            }
            break;

        case DECIDE:
            state = (_sensorNorth > SAFE_DISTANCE) ? FORWARD : TURN_RIGHT;
            if (state == TURN_RIGHT) turnStart = millis();
            break;
    }
}

void CommandHandler::updateMap(float heading, float distance) {
    float angle = heading * M_PI / 180.0f;
    float dx = cos(angle) * distance * 0.1f;
    float dy = sin(angle) * distance * 0.1f;
    
    if (!_explorationPath.empty()) {
        auto& last = _explorationPath.back();
        _explorationPath.emplace_back(last.first + dx, last.second + dy);
    }

    if (millis() - _lastMapUpdate > 2000) {
        publishMap();
        _lastMapUpdate = millis();
    }
}

// Communication functions
void CommandHandler::publishMap() {
    String mapData = "{\"path\":[";
    for (size_t i = 0; i < _explorationPath.size(); ++i) {
        mapData += "[" + String(_explorationPath[i].first, 2) + "," 
                 + String(_explorationPath[i].second, 2) + "]";
        if (i < _explorationPath.size() - 1) mapData += ",";
    }
    mapData += "],\"n\":" + String(_sensorNorth, 2) 
             + ",\"e\":" + String(_sensorEast, 2)
             + ",\"w\":" + String(_sensorWest, 2)
             + ",\"y\":" + String(_currentHeading, 2) + "}";
    
    _mqttClient.publish("robot/map", mapData.c_str());
}

void CommandHandler::publishStatus() {
    if (!_mqttClient.connected()) return;

    const char* modeStr = "";
    switch(_driveMode) {
        case DriveMode::MANUAL: modeStr = "MANUAL"; break;
        case DriveMode::OBSTACLE_AVOIDANCE: modeStr = "AVOIDANCE"; break;
        case DriveMode::WALL_FOLLOWING: modeStr = "WALL_FOLLOW"; break;
        case DriveMode::ROOM_MAPPING: modeStr = "MAPPING"; break;
    }
    
    String status = "Mode:" + String(modeStr) +
                  "|Speed:" + String(_globalSpeed) +
                  "|H:" + String(_currentHeading,1) + "°";
    _mqttClient.publish(_statusTopic, status.c_str());
}

// Configuration
void CommandHandler::setGlobalSpeed(int speed) {
    _globalSpeed = speed;
}

void CommandHandler::setStatusTopic(const char* topic) {
    _statusTopic = topic;
}

void CommandHandler::updateBatteryLevel() {
    if (millis() - _lastBatteryUpdate > BATTERY_UPDATE_INTERVAL) {
        float voltage = readBatteryVoltage();
        
        // Convert voltage to percentage (adjust these values for your battery)
        const float FULL_VOLTAGE = 8.4;
        const float EMPTY_VOLTAGE = 3.3;
        
        _batteryLevel = constrain(
            (voltage - EMPTY_VOLTAGE) / (FULL_VOLTAGE - EMPTY_VOLTAGE) * 100.0,
            0.0, 100.0
        );
        
        publishBatteryLevel();
        _lastBatteryUpdate = millis();
    }
}

float CommandHandler::readBatteryVoltage() {
    int raw = analogRead(BATTERY_PIN);
    float voltage = (raw / MAX_ADC) * REFERENCE_VOLTAGE * VOLTAGE_DIVIDER_RATIO;
    return voltage;
}

void CommandHandler::publishBatteryLevel() {
    if (_mqttClient.connected()) {
        String payload = "{\"voltage\":" + String(readBatteryVoltage(), 2) + 
                        ",\"percent\":" + String(_batteryLevel, 1) + "}";
        _mqttClient.publish("robot/batterylvl", payload.c_str());
        printf("Battery Level: %.1f%%\n", _batteryLevel);
    }
}

void CommandHandler::setEmergencyStop(bool state) {
    emergencyStop = state;
    if (state) {
        stopAllMotors();
    }
}

bool CommandHandler::isEmergencyStop() const {
    return emergencyStop;
}
