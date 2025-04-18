#ifndef GY271_QMC5883L_H
#define GY271_QMC5883L_H

#include <Arduino.h>
#include <Wire.h>

class GY271_QMC5883L {
public:
    GY271_QMC5883L(uint8_t i2c_addr = 0x0D, TwoWire &wire = Wire);
    
    bool begin();
    bool isConnected();
    
    void setCalibration(float x_min, float x_max, float y_min, float y_max);
    void readRaw(int16_t *x, int16_t *y, int16_t *z);
    float getHeading();
    float getHeadingDegrees();
    void getCalibratedValues(float *x, float *y);
    
    // Configuration methods
    void setMode(uint8_t mode);
    void setOutputDataRate(uint8_t odr);
    void setRange(uint8_t range);
    void setOversampling(uint8_t osr);
    
private:
    uint8_t _i2c_addr;
    TwoWire *_wire;
    
    // Calibration values
    float _x_min = -1379.0;
    float _x_max = 1683.0;
    float _y_min = -1694.0;
    float _y_max = 1195.0;
    
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);
    float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
};

#endif // GY271_QMC5883L_H