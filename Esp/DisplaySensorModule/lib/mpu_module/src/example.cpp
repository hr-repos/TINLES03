#include <Arduino.h>
#include "mpu_module.h"

long timer = 0;
MpuModule mpuModule;

void setup() {
    Wire.begin();
    Serial.begin(115600);
}

void loop() {
    if(millis() - timer > 1000){
        mpuModule.update();
        mpuModule.printAllData();
        timer = millis();
    }
}
