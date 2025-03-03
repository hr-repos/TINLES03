#ifndef Lcd_module_h
#define Lcd_module_h

class LcdModule
{
    private:
        int sdaPin;
        int sclPin;
        int i2cAddress;
        bool backLightState;
        
    public:
        const int MAX_LINE_LENGTH = 16;
        const int TOTAL_LINES = 2;
        
        // LcdModule();
        void initialize();
        void setTextFirstLine(char *text);
        void setTextSecondLine(char *text);
        void setTextLineWrap(char *text);
        void clearLineOne();
        void clearLineTwo();
        void clearDisplay();
        void changeBackLightState(bool state);
};

#endif
