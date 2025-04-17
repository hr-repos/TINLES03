#include "lcd_module.h"
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

// Custom constrain function if not available
template <typename T> T constrain_value(T value, T min, T max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust I2C address if needed

void LcdModule::initialize()
{
    lcd.init();
    lcd.backlight();
    lcd.clear();
    backLightState = true;
    lcdInitialized = true; // Add this line

    // Display initial message
    lcd.setCursor(0, 0);
    lcd.print("Robot Online");
    lcd.setCursor(0, 1);
    lcd.print("Initializing...");
}

void LcdModule::setTextFirstLine(const char* text)
{
    lcd.setCursor(0, 0);
    lcd.print("                "); // Clear line first
    lcd.setCursor(0, 0);
    lcd.print(text);
}

void LcdModule::setTextSecondLine(const char* text)
{
    lcd.setCursor(0, 1);
    lcd.print("                "); // Clear line first
    lcd.setCursor(0, 1);
    lcd.print(text);
}

void LcdModule::clearLineOne() { setTextFirstLine("                "); }

void LcdModule::clearLineTwo() { setTextSecondLine("                "); }

void LcdModule::clearDisplay() { lcd.clear(); }

void LcdModule::changeBackLightState(bool state)
{
    if (state) {
        lcd.backlight();
    } else {
        lcd.noBacklight();
    }
    backLightState = state;
}

bool LcdModule::displaySensorData(float north, float east, float south, float west, float yaw)
{
    if (!lcdInitialized)
        return false;

    char line1[17];
    char line2[17];
    snprintf(line1, sizeof(line1), "N:%3.0f S:%3.0f", north, south);
    snprintf(line2, sizeof(line2), "E:%3.0f W:%3.0f", east, west);
    Serial.println(line1);
    Serial.println(line2);

    setTextFirstLine(line1);
    setTextSecondLine(line2);
    return true;
}

void LcdModule::displayBatteryLevel(float voltage)
{
    char battLine[17];
    // Calculate percentage (3.0V-4.2V range for LiPo)
    int percent = static_cast<int>(((voltage - 3.0f) / (4.2f - 3.0f)) * 100.0f);
    percent = constrain_value(percent, 0, 100); // Use our custom constrain

    snprintf(battLine, sizeof(battLine), "Batt: %.1fV %3d%%", voltage, percent);

    // Alternate display between battery and other info
    static bool showBatt = false;
    if (showBatt) {
        setTextSecondLine(battLine);
    }
    showBatt = !showBatt;
}