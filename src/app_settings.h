#ifndef APP_SETTINGS
#define APP_SETTINGS

#include "Arduino.h"

#define MAGIC_NUMBER 0x5A
#define BASE_EEPROM_ADDR 0

typedef struct
{
    uint8_t magic = MAGIC_NUMBER;
    char timezone[32];
    uint8_t brightness;
    uint16_t scrollDelay;
} AppSettings;

void persistScrollDelay(uint16_t _scrollDelay);
void persistBrightness(uint8_t _brightness);
void persistTimezone(const char _timezone[32]);
void readSettings(AppSettings *settings);

#endif