#include "Motor.h"

Motor::Motor(int ena, int in1, int in2) {
    _ena = ena;
    _in1 = in1;
    _in2 = in2;
}

void Motor::begin() {
    pinMode(_ena, OUTPUT);
    pinMode(_in1, OUTPUT);
    pinMode(_in2, OUTPUT);
    analogWrite(_ena, 0); // Start with motors stopped
}

void Motor::updateMotor(MotorCommand command) {
    int speed = constrain(command.speed, 0, 255); // Ensure valid speed range

    analogWrite(_ena, speed); // Apply PWM speed

    if (command.direction == 1) {
        digitalWrite(_in1, HIGH);
        digitalWrite(_in2, LOW);
    } else if (command.direction == -1) {
        digitalWrite(_in1, LOW);
        digitalWrite(_in2, HIGH);
    } else {
        stop();
    }
}

void Motor::stop() {
    analogWrite(_ena, 0);
    digitalWrite(_in1, LOW);
    digitalWrite(_in2, LOW);
}
