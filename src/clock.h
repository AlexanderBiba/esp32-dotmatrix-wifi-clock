#ifndef CLOCK_H
#define CLOCK_H

#include "settings.h"

#define TIME_BUFFER_SIZE 16
#define RAW_TIME_BITMAP_SIZE 8 * 4

#define MMDD_DATE_FORMAT 1
#define SMALL_SECONDS_CLOCK 1

class Clock
{
public:
    Clock(AppSettings *settings) : settings(settings) { updateTime(); };
    uint8_t *getTime();
    uint8_t *getDate();
    void updateTime();
    void loadClockBitmap();
    void loadDateBitmap();

private:
    AppSettings *settings;
    long epoch = 0;
    long currentTime = 0;
    char timeBuffer[TIME_BUFFER_SIZE] = {0};
    uint8_t clockBitmap[RAW_TIME_BITMAP_SIZE] = {0};
    uint8_t dateBitmap[RAW_TIME_BITMAP_SIZE] = {0};
};

#endif