#ifndef LCD_MODULE_H
#define LCD_MODULE_H

class LcdModule {
private:
    bool backLightState;
    bool lcdInitialized = false;

public:
    const int MAX_LINE_LENGTH = 16;
    const int TOTAL_LINES = 2;

    void initialize();
    void setTextFirstLine(const char* text);
    void setTextSecondLine(const char* text);
    void setTextLineWrap(const char* text);
    void clearLineOne();
    void clearLineTwo();
    void clearDisplay();
    void changeBackLightState(bool state);

    // Sensor and battery display methods
    bool displaySensorData(float north, float east, float south, float west, float yaw);
    void displayBatteryLevel(float voltage);
};

#endif