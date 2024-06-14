
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <HttpClient.h>

#include "utils.h"
#include "weather.h"
#include "charmaps.h"
#include "main.h"

#define WEATHER_UPDATE_INTERVAL 10 * 60 * 1000

void Weather::loadWeatherBitmap(const char *weather)
{
    uint8_t *rawWeather = weatherBitmap;

    PRINT("Weather: ", weather);
    int count = 0;
    for (int i = 0; i < strlen(weather); ++i)
    {
        if ('0' <= weather[i] && weather[i] <= '9')
        {
            if (count++ == 32)
            {
                return;
            }
            const uint8_t *digit = digitCharMap[weather[i] - '0'];
            for (int i = 0; i < CLOCK_DIGIT_WIDTH; ++i)
            {
#ifdef BOTTOM_ALIGN
                *rawWeather++ = *digit++ * 2;
#else
                *rawWeather++ = *digit++;
#endif
            }
        }
        else
        {
            const uint8_t *charBitmap = appCharMap[weather[i] - '!'];
            *rawWeather++ = 0;
            for (int j = 0; j < charBitmap[0]; ++j)
            {
                if (count++ == 32)
                {
                    return;
                }
#ifdef BOTTOM_ALIGN
                *rawWeather++ = charBitmap[j + 1] * 2;
#else
                *rawWeather++ = charBitmap[j + 1];
#endif
            }
            *rawWeather++ = 0;
        }
    }
    *rawWeather++ = 0;
    if (settings->getWeatherUnits() == 'c')
    {
        *rawWeather++ = 0xE0;
        *rawWeather++ = 0xA0;
        *rawWeather++ = 0xA0;
    }
    else if (settings->getWeatherUnits() == 'f')
    {
        *rawWeather++ = 0xF0;
        *rawWeather++ = 0x50;
        *rawWeather++ = 0x10;
    }
    for (int i = count; i < RAW_WEATHER_BITMAP_SIZE - count; ++i)
    {
        *rawWeather++ = 0;
    }
}

const uint8_t *Weather::getWeather()
{
    static long lastRefresh = 0;

    if ((millis() - lastRefresh > WEATHER_UPDATE_INTERVAL) && updateWeatherData())
    {
        lastRefresh = millis();
        updateWeatherData();
    }

    return weatherBitmap;
}

bool Weather::updateWeatherData()
{
    static char localBuffer[RAW_WEATHER_BITMAP_SIZE] = {0};

    if (settings->getLatitude() == 0 || settings->getLongitude() == 0)
    {
        loadWeatherBitmap("No LOC");
    }

    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();
    HttpClient client = HttpClient(wifiClient, "api.open-meteo.com", 443);
    char url[128] = {0};
    sprintf(url,
            "/v1/forecast?latitude=%lf&longitude=%lf&temperature_unit=%s&current=temperature",
            settings->getLatitude(),
            settings->getLongitude(),
            settings->getWeatherUnits() == 'c' ? "celsius" : "fahrenheit");
    PRINT("url: ", url);
    client.get(url);
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    if (statusCode == 429)
    {
        PRINTS("Rate limited");
        return false;
    }

    PRINT("Status code: ", statusCode);
    PRINT("Response: ", response);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    if (error)
    {
        PRINT("deserializeJson() failed: ", error.f_str());
        return false;
    }

    weatherData = {doc["current"]["temperature"]};

    sprintf(localBuffer, "%.2lf", weatherData.temp);
    loadWeatherBitmap(localBuffer);

    return true;
}