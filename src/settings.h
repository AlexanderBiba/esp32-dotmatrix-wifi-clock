#ifndef APP_SETTINGS
#define APP_SETTINGS

#include "Arduino.h"
#include "main.h"

#define TIMEZONE_BUFFER_SIZE 128
#define STOCK_API_KEY_BUFFER_SIZE 128

#define MAGIC_NUMBER 0x5A
#define BASE_EEPROM_ADDR 0

class AppSettings
{
public:
    AppSettings();

    void setBrightness(uint8_t _brightness);
    void setTimezone(const char _timezone[TIMEZONE_BUFFER_SIZE]);
    void setStockApiKey(const char _stockApiKey[STOCK_API_KEY_BUFFER_SIZE]);
    void setLatitude(float _latitude);
    void setLongitude(float _longitude);
    void setWeatherUnits(char _units);
    void setActiveCards(bool _activeCards[OPERATION_MODE_LENGTH]);

    uint8_t getBrightness() { return settings.display.brightness; }
    const char *getTimezone() { return settings.time.timezone; }
    const char *getStockApiKey() { return settings.stock.apiKey; }
    float getLatitude() { return settings.weather.latitude; }
    float getLongitude() { return settings.weather.longitude; }
    char getWeatherUnits() { return settings.weather.units; }
    bool *getActiveCards() { return settings.activeCards; }

private:
    struct _AppSettings
    {
        uint8_t magic;
        struct
        {
            char timezone[TIMEZONE_BUFFER_SIZE];
        } time;
        struct
        {
            char apiKey[STOCK_API_KEY_BUFFER_SIZE];
        } stock;
        struct
        {
            uint8_t brightness;
        } display;
        struct
        {
            float latitude;
            float longitude;
            char units;
        } weather;
        bool activeCards[OPERATION_MODE_LENGTH];
    } settings;
};

#endif