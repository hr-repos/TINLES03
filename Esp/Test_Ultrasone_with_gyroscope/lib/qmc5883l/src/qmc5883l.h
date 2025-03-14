#ifndef QMC5883L_H
#define QMC5883L_H

#include <Arduino.h>
#include <Wire.h>

class QMC5883L {
public:
    QMC5883L();
    bool begin();
    void update();
    float getYaw();

private:
    int16_t magX, magY, magZ;
    void readRawData();
};

#endif