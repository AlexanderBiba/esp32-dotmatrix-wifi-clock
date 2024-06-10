#ifndef APP_SETTINGS
#define APP_SETTINGS

#include "Arduino.h"

#define TIMEZONE_BUFFER_SIZE 128
#define STOCK_API_KEY_BUFFER_SIZE 128

#define MAGIC_NUMBER 0x5A
#define BASE_EEPROM_ADDR 0

typedef struct
{
    uint8_t magic = MAGIC_NUMBER;
    char timezone[TIMEZONE_BUFFER_SIZE];
    char stockApiKey[STOCK_API_KEY_BUFFER_SIZE];
    uint8_t brightness;
    uint16_t scrollDelay;
} AppSettings;

void persistScrollDelay(uint16_t _scrollDelay);
void persistBrightness(uint8_t _brightness);
void persistTimezone(const char _timezone[TIMEZONE_BUFFER_SIZE]);
void persistStockApiKey(const char _stockApiKey[STOCK_API_KEY_BUFFER_SIZE]);
void readSettings(AppSettings *settings);

#endif