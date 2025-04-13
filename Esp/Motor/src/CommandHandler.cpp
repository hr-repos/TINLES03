#include "CommandHandler.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

CommandHandler::CommandHandler(QueueHandle_t motor1Queue, 
                             QueueHandle_t motor2Queue, 
                             QueueHandle_t motor3Queue,
                             PIDController& headingPID) :
    _motor1Queue(motor1Queue),
    _motor2Queue(motor2Queue),
    _motor3Queue(motor3Queue),
    _headingPID(headingPID) {
}

void CommandHandler::handleCommand(char command) {
    // Extended commands (x0, x1, x2)
    if (command == 'x') {
        if (_extendedCommand.length() > 0) {
            char modeChar = _extendedCommand[0];
            switch(modeChar) {
                case '0': setDriveMode(DriveMode::MANUAL); break;
                case '1': setDriveMode(DriveMode::OBSTACLE_AVOIDANCE); break;
                case '2': setDriveMode(DriveMode::WALL_FOLLOWING); break;
            }
            _extendedCommand = "";
            return;
        }
        setDriveMode(_driveMode == DriveMode::MANUAL ? 
                    DriveMode::OBSTACLE_AVOIDANCE : 
                    DriveMode::MANUAL);
        return;
    }
    
    // Buffer extended commands
    if (command >= '0' && command <= '2') {
        _extendedCommand += command;
        return;
    }

    // Ignore movement commands in autonomous modes
    if (_driveMode != DriveMode::MANUAL && command != ' ') {
        return;
    }

    switch(command) {
        case 'w': // Forward
            _targetHeading = _currentHeading;
            _headingPID.setTarget(_targetHeading);
            _isMovingStraight = true;
            _currentDirectionX = 0;
            _currentDirectionY = 1;
            moveOmni(0, 1);
            break;

        case 's': // Backward
            _targetHeading = _currentHeading;
            _headingPID.setTarget(_targetHeading);
            _isMovingStraight = true;
            _currentDirectionX = 0;
            _currentDirectionY = -1;
            moveOmni(0, -1);
            break;

        case 'a': // Left
            _targetHeading = _currentHeading;
            _headingPID.setTarget(_targetHeading);
            _isMovingStraight = true;
            _currentDirectionX = -1;
            _currentDirectionY = 0;
            moveOmni(-1, 0);
            break;

        case 'd': // Right
            _targetHeading = _currentHeading;
            _headingPID.setTarget(_targetHeading);
            _isMovingStraight = true;
            _currentDirectionX = 1;
            _currentDirectionY = 0;
            moveOmni(1, 0);
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
}

void CommandHandler::stopAllMotors() {
    MotorCommand cmd = {0, 0};
    xQueueSend(_motor1Queue, &cmd, 0);
    xQueueSend(_motor2Queue, &cmd, 0);
    xQueueSend(_motor3Queue, &cmd, 0);
    _isMovingStraight = false;
}

void CommandHandler::moveOmni(float directionX, float directionY, float rotation) {
    const float motorComp[3] = {1.0f, 0.8f, 1.0f};
    const float deadzone = 0.1f;

    // Pure rotation
    if (fabs(directionX) < 0.01f && fabs(directionY) < 0.01f && fabs(rotation) > 0.01f) {
        int speed = static_cast<int>(_globalSpeed * 0.9f);
        sendMotorCommands(
            (rotation > 0) ? 1 : -1, speed,
            (rotation > 0) ? 1 : -1, speed,
            (rotation > 0) ? 1 : -1, speed
        );
        return;
    }

    // Normal movement
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

void CommandHandler::updateHeading(float newHeading) {
    _currentHeading = newHeading;

    if (_isMovingStraight) {
        double error = _currentHeading - _targetHeading;
        if (fabs(error) > 10.0) {
            double correction = _headingPID.compute(_currentHeading);
            moveOmni(_currentDirectionX, _currentDirectionY, correction);
        } else {
            moveOmni(_currentDirectionX, _currentDirectionY, 0);
        }
    }
}

void CommandHandler::updateSensorData(const char* payload) {
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

void CommandHandler::setDriveMode(DriveMode mode) {
    _driveMode = mode;
    stopAllMotors();
}

void CommandHandler::autonomousDrive() {
    switch (_driveMode) {
        case DriveMode::OBSTACLE_AVOIDANCE:
            avoidObstacles();
            break;
        case DriveMode::WALL_FOLLOWING:
            followRightWall();
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
            moveOmni(0, MIN_SPEED, 0.5);
        } else {
            moveOmni(0, MIN_SPEED, -0.5);
        }
        delay(200);
    }
}

void CommandHandler::followRightWall() {
    float error = _sensorEast - WALL_DISTANCE;
    float steering = constrain(error * 0.02f, -0.3f, 0.3f);
    float forwardSpeed = (_sensorNorth > SAFE_DISTANCE) ? MAX_SPEED : 
                       (_sensorNorth > TOO_CLOSE) ? MAX_SPEED * 0.5f : 0;
    moveOmni(0, forwardSpeed, steering);
}

void CommandHandler::setGlobalSpeed(int speed) {
    _globalSpeed = speed;
}