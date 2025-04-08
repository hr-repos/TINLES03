#ifndef DIRECTION_CALCULATOR_H
#define DIRECTION_CALCULATOR_H

#include "GY271_QMC5883L.h"
#include "UltrasonicSensor.h"
#include <Arduino.h>

class DirectionCalculator {
public:
    DirectionCalculator(UltrasonicSensor& north, UltrasonicSensor& east, UltrasonicSensor& south,
        UltrasonicSensor& west, GY271_QMC5883L& compass);
    void update();
    void printData();

    // Public variables for MQTT access
    float rawNorth, rawEast, rawSouth, rawWest;
    float recalculatedNorth, recalculatedEast, recalculatedSouth, recalculatedWest;
    float currentYaw; // Add yaw angle

private:
    UltrasonicSensor& northSensor;
    UltrasonicSensor& eastSensor;
    UltrasonicSensor& southSensor;
    UltrasonicSensor& westSensor;
    GY271_QMC5883L& compass; // Compass reference
    float initialYaw = 0.0f;
    bool hasInitialYaw = false;

    void recalculateData(float yaw);
};

#endif