#ifndef DIRECTION_CALCULATOR_H
#define DIRECTION_CALCULATOR_H

#include <Arduino.h>
#include "UltrasonicSensor.h"
#include "CompassModule.h"

class DirectionCalculator {
public:
    DirectionCalculator(UltrasonicSensor &north, UltrasonicSensor &east, UltrasonicSensor &south, UltrasonicSensor &west, CompassModule &compass);
    void update();
    void printData();

private:
    UltrasonicSensor &northSensor;
    UltrasonicSensor &eastSensor;
    UltrasonicSensor &southSensor;
    UltrasonicSensor &westSensor;
    CompassModule &compass;

    float rawNorth, rawEast, rawSouth, rawWest;
    float recalculatedNorth, recalculatedEast, recalculatedSouth, recalculatedWest;

    void recalculateData(float yaw);
};

#endif