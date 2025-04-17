#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include <functional>  // For std::function

// Command callback function type
typedef std::function<void(char)> CommandCallback;

struct MotorCommand {
    int direction; // 1 = forward, -1 = backward, 0 = stop
    int speed;     // 0-255
};

class Motor {
public:
    Motor(int ena, int in1, int in2);
    void begin();
    void updateMotor(MotorCommand command);
    void stop();
    void sleep();
    void wake();
    
    // Command handling
    void setCommandCallback(CommandCallback callback);
    void handleCommand(char command);
    
    // Movement commands
    void forward(int speed = 255);
    void backward(int speed = 255);
    void rotateCW(int speed = 255);  // Clockwise
    void rotateCCW(int speed = 255); // Counter-clockwise

private:
    int _ena;
    int _in1;
    int _in2;
    CommandCallback _commandCallback;
    bool _isAsleep = false;
};

#endif