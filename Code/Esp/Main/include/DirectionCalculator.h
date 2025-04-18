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
    void getSensorData(float& north, float& east, float& south, float& west, float& yaw) const {
        north = recalculatedNorth;
        east = recalculatedEast;
        south = recalculatedSouth;
        west = recalculatedWest;
        yaw = currentYaw;
    }

private:
    UltrasonicSensor& northSensor;
    UltrasonicSensor& eastSensor;
    UltrasonicSensor& southSensor;
    UltrasonicSensor& westSensor;
    GY271_QMC5883L& compass; // Compass reference
    float initialYaw = 0.0f;
    bool hasInitialYaw = false;

    static const int FILTER_SIZE = 5;
    float rawNorthHistory[FILTER_SIZE] = {0};
    float rawEastHistory[FILTER_SIZE] = {0};
    float rawSouthHistory[FILTER_SIZE] = {0};
    float rawWestHistory[FILTER_SIZE] = {0};
    float yawHistory[FILTER_SIZE] = {0};
    int historyIndex = 0;

    float applyFilter(float* history, float newValue);

    void recalculateData(float yaw);
};

#endif