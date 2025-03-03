#include <Arduino.h>
#include "lcd_module.h"

// put function declarations here:
// int myFunction(int, int);
LcdModule lcdModule;

void setup() {
    lcdModule.initialize();
}


void loop() {
    lcdModule.setTextFirstLine("Test");
    lcdModule.setTextSecondLine("Explorer bot");
}
