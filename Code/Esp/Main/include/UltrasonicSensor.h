#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>

class UltrasonicSensor {
public:
    UltrasonicSensor(uint8_t triggerPin, uint8_t echoPin);
    float getDistance();

private:
    uint8_t triggerPin;
    uint8_t echoPin;
    float calculateDistance();
};

#endif