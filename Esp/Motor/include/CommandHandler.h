#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "pidController.h"

class CommandHandler {
public:
    enum class DriveMode {
        MANUAL,
        OBSTACLE_AVOIDANCE,
        WALL_FOLLOWING
    };

    CommandHandler(QueueHandle_t motor1Queue, 
                  QueueHandle_t motor2Queue, 
                  QueueHandle_t motor3Queue,
                  PIDController& headingPID);
    
    void handleCommand(char command);
    void updateHeading(float newHeading);
    void updateSensorData(const char* payload);
    void setGlobalSpeed(int speed);
    
    // Getters
    DriveMode getDriveMode() const { return _driveMode; }
    float getCurrentHeading() const { return _currentHeading; }
    int getGlobalSpeed() const { return _globalSpeed; }

private:
    // Motor control structures
    struct MotorCommand {
        int direction;
        int speed;
    };

    struct Motion {
        float m1;
        float m2;
        float m3;
    };

    // Constants
    static constexpr float MAX_SPEED = 1.0f;
    static constexpr float MIN_SPEED = 0.3f;
    static constexpr float SAFE_DISTANCE = 50.0f; // cm
    static constexpr float TOO_CLOSE = 20.0f; // cm
    static constexpr float WALL_DISTANCE = 30.0f; // cm

    // Queues
    QueueHandle_t _motor1Queue;
    QueueHandle_t _motor2Queue;
    QueueHandle_t _motor3Queue;

    // PID Controller
    PIDController& _headingPID;

    // State variables
    DriveMode _driveMode = DriveMode::MANUAL;
    float _currentHeading = 0.0f;
    float _targetHeading = 0.0f;
    bool _isMovingStraight = false;
    float _currentDirectionX = 0.0f;
    float _currentDirectionY = 0.0f;
    int _globalSpeed = 200;
    
    // Sensor data
    float _sensorNorth = 0.0f;
    float _sensorEast = 0.0f;
    float _sensorWest = 0.0f;

    // Extended command buffer
    String _extendedCommand;

    // Motor control functions
    void stopAllMotors();
    void moveOmni(float directionX, float directionY, float rotation = 0.0f);
    void sendMotorCommands(int dir1, int speed1, int dir2, int speed2, int dir3, int speed3);

    // Autonomous modes
    void setDriveMode(DriveMode mode);
    void autonomousDrive();
    void avoidObstacles();
    void followRightWall();

    // Helper functions
    float extractFloatValue(const char* payload, const char* key);
};

#endif