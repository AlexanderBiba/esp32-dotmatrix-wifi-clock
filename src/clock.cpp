#include <MD_MAX72xx.h>
#include <WiFi.h>
#include <ezTime.h>

#include "clock.h"
#include "settings.h"
#include "charmaps.h"
#include "main.h"
#include "renderer.h"

extern Renderer *renderer;

Timezone myTimezone;

void Clock::loadClockBitmap()
{
    uint8_t h = myTimezone.hour();
    uint8_t m = myTimezone.minute();
    uint8_t s = myTimezone.second();

    uint8_t *p = clockBitmap;

#if SMALL_SECONDS_CLOCK
    // Render hour digits
    p += renderer->writeCharToBuffer('0' + (h / 10), p);
    p += renderer->writeCharToBuffer('0' + (h % 10), p);
    *p++ = 0;
    *p++ = 0; // Separator
    // Render minute digits
    p += renderer->writeCharToBuffer('0' + (m / 10), p);
    p += renderer->writeCharToBuffer('0' + (m % 10), p);
    *p++ = 0;
    *p++ = 0; // Separator
    // Render second digits (small)
    p += renderer->writeSmallCharToBuffer('0' + (s / 10), p);
    *p++ = 0;
    p += renderer->writeSmallCharToBuffer('0' + (s % 10), p);
#else
    // Render hour digits
    p += renderer->writeCharToBuffer('0' + (h / 10), p);
    p += renderer->writeCharToBuffer('0' + (h % 10), p);
    *p++ = 0; // Separator
    // Render minute digits
    p += renderer->writeCharToBuffer('0' + (m / 10), p);
    p += renderer->writeCharToBuffer('0' + (m % 10), p);
    *p++ = 0; // Separator
    // Render second digits
    p += renderer->writeCharToBuffer('0' + (s / 10), p);
    p += renderer->writeCharToBuffer('0' + (s % 10), p);
#endif
}

void Clock::loadDateBitmap()
{
    static const char *weekdayCharMap[7] = {
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat"
    };
    uint8_t *p = dateBitmap;

    const char *weekday = weekdayCharMap[myTimezone.weekday() - 1];
    p = renderer->loadStringToBitmap(weekday, p);
    *p++ = 0;
    *p++ = 0;

    // Use runtime setting for date format
    char dateFormat = settings->getDateFormat();
    if (dateFormat == 'm') {
        // MM.DD format
        p += renderer->writeSmallCharToBuffer('0' + myTimezone.month() / 10, p);
        *p++ = 0;
        p += renderer->writeSmallCharToBuffer('0' + myTimezone.month() % 10, p);
        *p++ = 0x20;
        p += renderer->writeSmallCharToBuffer('0' + myTimezone.day() / 10, p);
        *p++ = 0;
        p += renderer->writeSmallCharToBuffer('0' + myTimezone.day() % 10, p);
    } else {
        // DD.MM format
        p += renderer->writeSmallCharToBuffer('0' + myTimezone.day() / 10, p);
        *p++ = 0;
        p += renderer->writeSmallCharToBuffer('0' + myTimezone.day() % 10, p);
        *p++ = 0x20;
        p += renderer->writeSmallCharToBuffer('0' + myTimezone.month() / 10, p);
        *p++ = 0;
        p += renderer->writeSmallCharToBuffer('0' + myTimezone.month() % 10, p);
    }

    renderer->alignBitmapContentToCenter(dateBitmap, p);
}

Clock::Clock(AppSettings *settings)
{
    this->settings = settings;

    waitForSync();
    Serial.println("ezTime synced with NTP server.");

    String timezone_setting = settings->getTimezone();
    if (timezone_setting.length() > 0)
    {
        if (myTimezone.setLocation(timezone_setting))
        {
            Serial.printf("Timezone set to: %s\n", timezone_setting.c_str());
        }
        else
        {
            Serial.printf("Error: Could not set timezone '%s'. Defaulting to UTC.\n", timezone_setting.c_str());
        }
    }
    else
    {
        Serial.println("No timezone set in settings. Defaulting to UTC.");
    }
    currentTime = 0;
}

uint8_t *Clock::getTime()
{
    time_t now = ::now();
    if (now != currentTime)
    {
        currentTime = now;
        loadClockBitmap();
    }
    return clockBitmap;
}

uint8_t *Clock::getDate()
{
    time_t now = ::now();
    if (now != currentTime)
    {
        currentTime = now;
        loadDateBitmap();
    }
    return dateBitmap;
}

/**
 * @brief Sets the clock's timezone dynamically at runtime.
 * @param new_timezone The IANA timezone name (e.g., "America/Los_Angeles").
 */
void Clock::updateTimezone()
{
    const char *new_timezone = settings->getTimezone();
    if (myTimezone.setLocation(new_timezone))
    {
        Serial.printf("Timezone updated to: %s\n", new_timezone);

        // Force an immediate redraw of the bitmaps with the new time.
        currentTime = myTimezone.now();
        loadClockBitmap();
        loadDateBitmap();
    }
    else
    {
        Serial.printf("Error: Could not set timezone '%s'. It may be invalid.\n", new_timezone);
    }
}