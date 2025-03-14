#ifndef COMPASS_MODULE_H
#define COMPASS_MODULE_H

#include <Arduino.h>
#include <Wire.h>
#include "qmc5883l.h"

class CompassModule {
public:
    CompassModule();
    float getYaw(); // Get the current yaw angle

private:
    QMC5883L compass;
    void update(); // Update the compass data
};

#endif