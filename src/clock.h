#ifndef CLOCK_H
#define CLOCK_H

#include "settings.h"

#define TIME_BUFFER_SIZE sizeof("hh:mm:ss")
#define RAW_TIME_BITMAP_SIZE 8 * 4

#define SMALL_SECONDS_CLOCK 1

class Clock
{
public:
    Clock(AppSettings *settings) : settings(settings) { updateTime(); };
    const uint8_t *getTime();
    void updateTime();
    void loadBitmap();

private:
    AppSettings *settings;
    long epoch = 0;
    long currentTime = 0;
    char clockBuffer[TIME_BUFFER_SIZE] = {0};
    uint8_t bitmap[RAW_TIME_BITMAP_SIZE] = {0};
};

#endif