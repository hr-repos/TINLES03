#include <Arduino.h>
#include "CompassModule.h"

CompassModule::CompassModule() {
    compass.begin();
}

float CompassModule::getYaw() {
    compass.update();
    return compass.getYaw();
}