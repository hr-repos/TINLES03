#include <Arduino.h>
#include <Wire.h>
#include "Mpu_module.h"

void MpuModule::update(){
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);  
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 12);  

    accelerationX = Wire.read()<<8|Wire.read();    
    accelerationY = Wire.read()<<8|Wire.read();  
    accelerationZ = Wire.read()<<8|Wire.read();  
    gyroX = Wire.read()<<8|Wire.read();  
    gyroY = Wire.read()<<8|Wire.read();  
    gyroZ = Wire.read()<<8|Wire.read();  
}

void MpuModule::printAllData(){
    Serial.printf("Accelerometer: X = %d | Y = %d | Z = %d\t\t", accelerationX, accelerationY, accelerationZ);
    Serial.printf("Gyroscope: X = %d | Y = %d | Z = %d\n", gyroX, gyroY, gyroZ);
}