#ifndef Mpu_module_h
#define Mpu_module_h

#include <Arduino.h>
#include <Wire.h>

class MpuModule
{
    private:
    const int MPU = 0x68;
    public:
        int16_t accelerationX,accelerationY,accelerationZ,Tmp,gyroX,gyroY,gyroZ;
        
        void update();
        void printAllData();
};

#endif
