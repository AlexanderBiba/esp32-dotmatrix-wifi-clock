#include <EEPROM.h>
#include <Arduino.h>
#include <ArduinoJson.h>

#include "settings.h"

AppSettings::AppSettings()
{
    EEPROM.get(BASE_EEPROM_ADDR, settings);
    if (settings.magic != MAGIC_NUMBER)
    {
        Serial.println("No settings found, using defaults");
        settings.magic = MAGIC_NUMBER;
        strcpy(settings.time.timezone, "America/New_York");
        strcpy(settings.stock.apiKey, "\0");
        settings.display.brightness = 0xf;
        settings.weather.latitude = INT_MAX;
        settings.weather.longitude = INT_MAX;
        settings.weather.units = 'f';
        for (int i = 0; i < OPERATION_MODE_LENGTH; ++i)
        {
            settings.activeCards[i] = true;
        }
        EEPROM.put(BASE_EEPROM_ADDR, settings);
        EEPROM.commit();
    }
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

void AppSettings::setActiveCards(bool _activeCards[OPERATION_MODE_LENGTH])
{
    for (int i = 0; i < OPERATION_MODE_LENGTH; ++i)
    {
        settings.activeCards[i] = _activeCards[i];
    }
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, activeCards), settings.activeCards);
    EEPROM.commit();
}

void AppSettings::toJson(JsonDocument &doc)
{
    doc["timezone"] = settings.time.timezone;
    doc["brightness"] = settings.display.brightness;
    doc["latitude"] = settings.weather.latitude;
    doc["longitude"] = settings.weather.longitude;
    doc["weatherUnits"] = settings.weather.units == 'f' ? "f" : "c";
    doc["activeCards"].to<JsonArray>();
    for (int i = 0; i < OPERATION_MODE_LENGTH; ++i)
    {
        if (settings.activeCards[i])
        {
            doc["activeCards"].add(OperationModeStr[i]);
        }
    }
}