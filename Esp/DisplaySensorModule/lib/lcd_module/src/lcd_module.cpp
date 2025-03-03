#include <LiquidCrystal_I2C.h>
#include "lcd_module.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

char emptyLine[16];

void LcdModule::initialize(){
    lcd.init();
    lcd.clear();
    changeBackLightState(true);
}

void LcdModule::setTextFirstLine(char *text) {
    lcd.setCursor(0, 0);
    lcd.print(text);
}

void LcdModule::setTextSecondLine(char *text){
    lcd.setCursor(0, 1);
    lcd.print(text);
}

void LcdModule::setTextLineWrap(char *text){

}

void LcdModule::clearLineOne(){
    setTextFirstLine(emptyLine);
}

void LcdModule::clearLineTwo(){
    setTextSecondLine(emptyLine);
}

void LcdModule::clearDisplay(){
    lcd.clear();
}

void LcdModule::changeBackLightState(bool state){
    lcd.setBacklight(state);
    backLightState = state;
}