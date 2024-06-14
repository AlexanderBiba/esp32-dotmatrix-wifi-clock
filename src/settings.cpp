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
        strcpy(settings.time.timezone, "America/New_York");
        strcpy(settings.stock.apiKey, "\0");
        settings.display.brightness = 0xf;
        settings.display.scrollDelay = 75;
        settings.weather.latitude = 0;
        settings.weather.longitude = 0;
        settings.weather.units = 0;
        EEPROM.put(BASE_EEPROM_ADDR, settings);
        EEPROM.commit();
    }
}

void AppSettings::setScrollDelay(uint16_t _scrollDelay)
{
    settings.display.scrollDelay = _scrollDelay;
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, display.scrollDelay), _scrollDelay);
    EEPROM.commit();
}

void AppSettings::setBrightness(uint8_t _brightness)
{
    settings.display.brightness = _brightness;
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, display.brightness), _brightness);
    EEPROM.commit();
}

void AppSettings::setTimezone(const char _timezone[TIMEZONE_BUFFER_SIZE])
{
    strcpy(settings.time.timezone, _timezone);
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, time.timezone), settings.time.timezone);
    EEPROM.commit();
}

void AppSettings::setStockApiKey(const char _stockApiKey[STOCK_API_KEY_BUFFER_SIZE])
{
    strcpy(settings.stock.apiKey, _stockApiKey);
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, stock.apiKey), settings.stock.apiKey);
    EEPROM.commit();
}

void AppSettings::setLatitude(float _latitude)
{
    settings.weather.latitude = _latitude;
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, weather.latitude), settings.weather.latitude);
    EEPROM.commit();
}

void AppSettings::setLongitude(float _longitude)
{
    settings.weather.longitude = _longitude;
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, weather.longitude), settings.weather.longitude);
    EEPROM.commit();
}

void AppSettings::setWeatherUnits(char _units)
{
    settings.weather.units = _units;
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, weather.units), settings.weather.units);
    EEPROM.commit();
}