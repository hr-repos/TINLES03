#include <Arduino.h>
#include <Wire.h>
#include "qmc5883l.h"

QMC5883L::QMC5883L() {
    magX = magY = magZ = 0;
}

bool QMC5883L::begin() {
    Wire.begin();
    Wire.beginTransmission(0x0D); // QMC5883L I2C address
    Wire.write(0x09); // Set reset and roll pointer
    Wire.write(0x1D); // Mode: continuous, ODR: 200Hz, Range: 8G, OSR: 512
    return Wire.endTransmission() == 0;
}

void QMC5883L::update() {
    readRawData();
}

void QMC5883L::readRawData() {
    Wire.beginTransmission(0x0D);
    Wire.write(0x00); // Start reading from register 0x00
    Wire.endTransmission();

    Wire.requestFrom(0x0D, 6); // Request 6 bytes of data
    if (Wire.available() >= 6) {
        magX = Wire.read() | Wire.read() << 8;
        magY = Wire.read() | Wire.read() << 8;
        magZ = Wire.read() | Wire.read() << 8;
    }
}

float QMC5883L::getYaw() {
    float yaw = atan2(magY, magX) * 180 / PI; // Calculate yaw in degrees
    if (yaw < 0) {
        yaw += 360; // Ensure yaw is between 0 and 360
    }
    return yaw;
}