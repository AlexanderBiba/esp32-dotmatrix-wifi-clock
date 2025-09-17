#ifndef COUNTDOWN_H_
#define COUNTDOWN_H_

#include "settings.h"

#define RAW_COUNTDOWN_BITMAP_SIZE 32

class Countdown
{
public:
    Countdown(AppSettings *settings) : settings(settings) { updateCountdownData(); };
    uint8_t *getCountdown();
    bool updateCountdownData();

private:
    AppSettings *settings;
    struct
    {
        int days;
    } countdownData;
    uint8_t bitmap[RAW_COUNTDOWN_BITMAP_SIZE];
    void loadBitmap(const char *countdown);
};

#endif
