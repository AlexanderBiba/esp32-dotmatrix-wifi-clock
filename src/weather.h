#ifndef WEATHER_H_
#define WEATHER_H_

#include "settings.h"

#define RAW_WEATHER_BITMAP_SIZE 32

class Weather
{
public:
    Weather(AppSettings *settings) : settings(settings) { updateWeatherData(); };
    const uint8_t *getWeather();
    bool updateWeatherData();

private:
    AppSettings *settings;
    struct
    {
        float temp;
    } weatherData;
    uint8_t weatherBitmap[RAW_WEATHER_BITMAP_SIZE];
    void loadWeatherBitmap(const char *weather);
};

#endif