#include "../uwb.h"
// this code read the distance from the tag to two other anchors
// 
Uwb uwb;

void setup()
{
    uwb.initialize();
}

void loop()
{
    double anchor1_dis = uwb.getDistance(0x89);
    double anchor2_dis = uwb.getDistance(0x88);

    if (anchor1_dis > 0.05) {
        Serial.print("distance1: ");Serial.println(anchor1_dis);
    }
    if (anchor2_dis > 0.05) {
        Serial.print("distance2: ");Serial.println(anchor2_dis);
    }
    delay(1000);
}