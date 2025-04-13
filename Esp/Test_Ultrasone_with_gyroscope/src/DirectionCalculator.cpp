#include "DirectionCalculator.h"
#include <Arduino.h>

DirectionCalculator::DirectionCalculator(UltrasonicSensor& north, UltrasonicSensor& east, UltrasonicSensor& south,
    UltrasonicSensor& west, GY271_QMC5883L& compass)
    : northSensor(north)
    , eastSensor(east)
    , southSensor(south)
    , westSensor(west)
    , compass(compass)
{
}

float DirectionCalculator::applyFilter(float* history, float newValue) {
    history[historyIndex] = newValue;
    
    float sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++) {
        sum += history[i];
    }
    
    historyIndex = (historyIndex + 1) % FILTER_SIZE;
    return sum / FILTER_SIZE;
}

void DirectionCalculator::update() {
    // Get filtered raw distances
    rawNorth = applyFilter(rawNorthHistory, northSensor.getDistance());
    rawEast = applyFilter(rawEastHistory, eastSensor.getDistance());
    rawSouth = applyFilter(rawSouthHistory, southSensor.getDistance());
    rawWest = applyFilter(rawWestHistory, westSensor.getDistance());

    // Get current yaw from compass
    float currentRawYaw = compass.getHeadingDegrees();
    currentRawYaw = applyFilter(yawHistory, currentRawYaw);  // Filter yaw readings

    // Rest of the update method remains the same...
    if (!hasInitialYaw) {
        initialYaw = currentRawYaw;
        hasInitialYaw = true;
    }

    currentYaw = fmod(currentRawYaw - initialYaw + 360.0f, 360.0f);
    recalculateData(currentYaw);
}

void DirectionCalculator::recalculateData(float yaw)
{
    float radians = yaw * PI / 180.0;

    recalculatedNorth = fabs(rawNorth * cos(radians) - rawWest * sin(radians)) + 0.01;
    recalculatedEast = fabs(rawEast * cos(radians) - rawNorth * sin(radians)) + 0.01;
    recalculatedSouth = fabs(rawSouth * cos(radians) - rawEast * sin(radians)) + 0.01;
    recalculatedWest = fabs(rawWest * cos(radians) - rawSouth * sin(radians)) + 0.01;

    // Handle special cases for 90°, 180°, and 270°
    if (yaw >= 85 && yaw <= 95) { // 90° rotation
        recalculatedNorth = rawWest + 0.01;
        recalculatedEast = rawNorth + 0.01;
        recalculatedSouth = rawEast + 0.01;
        recalculatedWest = rawSouth + 0.01;
    } else if (yaw >= 175 && yaw <= 185) { // 180° rotation
        recalculatedNorth = rawSouth + 0.01;
        recalculatedEast = rawWest + 0.01;
        recalculatedSouth = rawNorth + 0.01;
        recalculatedWest = rawEast + 0.01;
    } else if (yaw >= 265 && yaw <= 275) { // 270° rotation
        recalculatedNorth = rawEast + 0.01;
        recalculatedEast = rawSouth + 0.01;
        recalculatedSouth = rawWest + 0.01;
        recalculatedWest = rawNorth + 0.01;
    }
    printData();
}

void DirectionCalculator::printData()
{
    Serial.println("Raw Data:");
    Serial.printf(
        "North: %.2f cm, East: %.2f cm, South: %.2f cm, West: %.2f cm\n", rawNorth, rawEast, rawSouth, rawWest);

    Serial.println("Recalculated Data:");
    Serial.printf("Raw Yaw: %.2f degrees, Normalized Yaw: %.2f degrees\n", compass.getHeadingDegrees(), currentYaw);
    Serial.printf("North: %.2f cm, East: %.2f cm, South: %.2f cm, West: %.2f cm\n", recalculatedNorth, recalculatedEast,
        recalculatedSouth, recalculatedWest);

    Serial.println("-----------------------------");
}