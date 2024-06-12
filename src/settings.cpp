#include <EEPROM.h>

#include "utils.h"
#include "settings.h"

AppSettings::AppSettings()
{
    EEPROM.get(BASE_EEPROM_ADDR, settings);
    if (settings.magic != MAGIC_NUMBER)
    {
        PRINTS("No settings found, using defaults");
        settings.magic = MAGIC_NUMBER;
        strcpy(settings.timezone, "America/New_York");
        strcpy(settings.stockApiKey, "\0");
        settings.brightness = 0xf;
        settings.scrollDelay = 75;
        EEPROM.put(BASE_EEPROM_ADDR, settings);
        EEPROM.commit();
    }
}

void AppSettings::setScrollDelay(uint16_t _scrollDelay)
{
    settings.scrollDelay = _scrollDelay;
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, scrollDelay), _scrollDelay);
    EEPROM.commit();
}

void AppSettings::setBrightness(uint8_t _brightness)
{
    settings.brightness = _brightness;
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, brightness), _brightness);
    EEPROM.commit();
}

void AppSettings::setTimezone(const char _timezone[TIMEZONE_BUFFER_SIZE])
{
    strcpy(settings.timezone, _timezone);
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, timezone), settings.timezone);
    EEPROM.commit();
}

void AppSettings::setStockApiKey(const char _stockApiKey[STOCK_API_KEY_BUFFER_SIZE])
{
    strcpy(settings.stockApiKey, _stockApiKey);
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, stockApiKey), settings.stockApiKey);
    EEPROM.commit();
}
