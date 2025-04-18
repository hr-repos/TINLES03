#include "GY271_QMC5883L.h"

// Register addresses
#define QMC5883L_REG_X_LSB 0x00
#define QMC5883L_REG_X_MSB 0x01
#define QMC5883L_REG_Y_LSB 0x02
#define QMC5883L_REG_Y_MSB 0x03
#define QMC5883L_REG_Z_LSB 0x04
#define QMC5883L_REG_Z_MSB 0x05
#define QMC5883L_REG_STATUS 0x06
#define QMC5883L_REG_TEMP_LSB 0x07
#define QMC5883L_REG_TEMP_MSB 0x08
#define QMC5883L_REG_CONTROL_1 0x09
#define QMC5883L_REG_CONTROL_2 0x0A
#define QMC5883L_REG_PERIOD 0x0B

// Mode settings
#define QMC5883L_MODE_STANDBY 0x00
#define QMC5883L_MODE_CONTINUOUS 0x01

// Output Data Rate settings
#define QMC5883L_ODR_10HZ 0x00
#define QMC5883L_ODR_50HZ 0x04
#define QMC5883L_ODR_100HZ 0x08
#define QMC5883L_ODR_200HZ 0x0C

// Full Scale Range settings
#define QMC5883L_RNG_2G 0x00
#define QMC5883L_RNG_8G 0x10

// Over Sampling Ratio settings
#define QMC5883L_OSR_512 0x00
#define QMC5883L_OSR_256 0x40
#define QMC5883L_OSR_128 0x80
#define QMC5883L_OSR_64 0xC0

GY271_QMC5883L::GY271_QMC5883L(uint8_t i2c_addr, TwoWire &wire) : _i2c_addr(i2c_addr), _wire(&wire) {}

bool GY271_QMC5883L::begin() {
    _wire->begin();
    
    // Check if device responds
    if (!isConnected()) {
        return false;
    }
    
    // Set default configuration
    setMode(QMC5883L_MODE_CONTINUOUS);
    setOutputDataRate(QMC5883L_ODR_200HZ);
    setRange(QMC5883L_RNG_8G);
    setOversampling(QMC5883L_OSR_512);
    
    // Set reset period register
    writeRegister(QMC5883L_REG_PERIOD, 0x01);
    
    return true;
}

bool GY271_QMC5883L::isConnected() {
    _wire->beginTransmission(_i2c_addr);
    return _wire->endTransmission() == 0;
}

void GY271_QMC5883L::setCalibration(float x_min, float x_max, float y_min, float y_max) {
    _x_min = x_min;
    _x_max = x_max;
    _y_min = y_min;
    _y_max = y_max;
}

void GY271_QMC5883L::readRaw(int16_t *x, int16_t *y, int16_t *z) {
    _wire->beginTransmission(_i2c_addr);
    _wire->write(QMC5883L_REG_X_LSB);
    _wire->endTransmission();
    
    _wire->requestFrom(_i2c_addr, 6);
    if (_wire->available() >= 6) {
        *x = (int16_t)(_wire->read() | _wire->read() << 8);
        *y = (int16_t)(_wire->read() | _wire->read() << 8);
        *z = (int16_t)(_wire->read() | _wire->read() << 8);
    }
}

float GY271_QMC5883L::getHeading() {
    int16_t x, y, z;
    readRaw(&x, &y, &z);
    
    float x_cal = mapFloat(x, _x_min, _x_max, -1000, 1000);
    float y_cal = mapFloat(y, _y_min, _y_max, -1000, 1000);
    
    return atan2(y_cal, x_cal);
}

float GY271_QMC5883L::getHeadingDegrees() {
    float heading = getHeading() * 180.0 / PI;
    if (heading < 0) {
        heading += 360;
    }
    return heading;
}

void GY271_QMC5883L::getCalibratedValues(float *x, float *y) {
    int16_t raw_x, raw_y, raw_z;
    readRaw(&raw_x, &raw_y, &raw_z);
    
    *x = mapFloat(raw_x, _x_min, _x_max, -1000, 1000);
    *y = mapFloat(raw_y, _y_min, _y_max, -1000, 1000);
}

// Configuration methods
void GY271_QMC5883L::setMode(uint8_t mode) {
    uint8_t value = readRegister(QMC5883L_REG_CONTROL_1);
    value = (value & 0xFE) | (mode & 0x01);
    writeRegister(QMC5883L_REG_CONTROL_1, value);
}

void GY271_QMC5883L::setOutputDataRate(uint8_t odr) {
    uint8_t value = readRegister(QMC5883L_REG_CONTROL_1);
    value = (value & 0xF3) | (odr & 0x0C);
    writeRegister(QMC5883L_REG_CONTROL_1, value);
}

void GY271_QMC5883L::setRange(uint8_t range) {
    uint8_t value = readRegister(QMC5883L_REG_CONTROL_1);
    value = (value & 0xEF) | (range & 0x10);
    writeRegister(QMC5883L_REG_CONTROL_1, value);
}

void GY271_QMC5883L::setOversampling(uint8_t osr) {
    uint8_t value = readRegister(QMC5883L_REG_CONTROL_1);
    value = (value & 0x3F) | (osr & 0xC0);
    writeRegister(QMC5883L_REG_CONTROL_1, value);
}

// Private methods
void GY271_QMC5883L::writeRegister(uint8_t reg, uint8_t value) {
    _wire->beginTransmission(_i2c_addr);
    _wire->write(reg);
    _wire->write(value);
    _wire->endTransmission();
}

uint8_t GY271_QMC5883L::readRegister(uint8_t reg) {
    _wire->beginTransmission(_i2c_addr);
    _wire->write(reg);
    _wire->endTransmission();
    
    _wire->requestFrom(_i2c_addr, 1);
    if (_wire->available()) {
        return _wire->read();
    }
    return 0;
}

float GY271_QMC5883L::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}