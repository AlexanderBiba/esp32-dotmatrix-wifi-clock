#include <EEPROM.h>

#include "utils.h"
#include "app_settings.h"

void persistScrollDelay(uint16_t _scrollDelay)
{
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(AppSettings, scrollDelay), _scrollDelay);
    EEPROM.commit();
}

void persistBrightness(uint8_t _brightness)
{
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(AppSettings, brightness), _brightness);
    EEPROM.commit();
}

void persistTimezone(const char _timezone[TIMEZONE_BUFFER_SIZE])
{
    char tz[TIMEZONE_BUFFER_SIZE];
    strcpy(tz, _timezone);
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(AppSettings, timezone), tz);
    EEPROM.commit();
}

void persistStockApiKey(const char _stockApiKey[STOCK_API_KEY_BUFFER_SIZE])
{
    char apiKey[STOCK_API_KEY_BUFFER_SIZE];
    strcpy(apiKey, _stockApiKey);
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(AppSettings, stockApiKey), apiKey);
    EEPROM.commit();
}

void initDefaultSettings(AppSettings *settings)
{
    settings->magic = MAGIC_NUMBER;
    strcpy(settings->timezone, "America/New_York");
    strcpy(settings->stockApiKey, "");
    settings->brightness = 0xf;
    settings->scrollDelay = 75;
}

void readSettings(AppSettings *settings)
{
    EEPROM.get(BASE_EEPROM_ADDR, *settings);
    if (settings->magic != MAGIC_NUMBER)
    {
        PRINTS("No settings found, using defaults");
        initDefaultSettings(settings);
        EEPROM.put(BASE_EEPROM_ADDR, *settings);
        EEPROM.commit();
    }
}
