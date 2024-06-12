#ifndef APP_SETTINGS
#define APP_SETTINGS

#include "Arduino.h"

#define TIMEZONE_BUFFER_SIZE 128
#define STOCK_API_KEY_BUFFER_SIZE 128

#define MAGIC_NUMBER 0x5A
#define BASE_EEPROM_ADDR 0

;

class AppSettings
{
public:
    AppSettings();
    void setScrollDelay(uint16_t _scrollDelay);
    void setBrightness(uint8_t _brightness);
    void setTimezone(const char _timezone[TIMEZONE_BUFFER_SIZE]);
    void setStockApiKey(const char _stockApiKey[STOCK_API_KEY_BUFFER_SIZE]);

    uint16_t getScrollDelay() { return settings.scrollDelay; }
    uint8_t getBrightness() { return settings.brightness; }
    const char *getTimezone() { return settings.timezone; }
    const char *getStockApiKey() { return settings.stockApiKey; }

    void printSettings()
    {
        Serial.println("Settings:");
        Serial.println(settings.magic);
        Serial.println(settings.timezone);
        Serial.println(settings.stockApiKey);
        Serial.println(settings.brightness);
        Serial.println(settings.scrollDelay);
    }

private:
    struct _AppSettings
    {
        uint8_t magic;
        char timezone[TIMEZONE_BUFFER_SIZE];
        char stockApiKey[STOCK_API_KEY_BUFFER_SIZE];
        uint8_t brightness;
        uint16_t scrollDelay;
    } settings;
};

#endif