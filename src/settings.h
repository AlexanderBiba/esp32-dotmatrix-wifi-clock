#ifndef APP_SETTINGS
#define APP_SETTINGS

#include <Arduino.h>
#include <ArduinoJson.h>

#include "main.h"

#define TIMEZONE_BUFFER_SIZE 128
#define STOCK_API_KEY_BUFFER_SIZE 128
#define MDNS_DOMAIN_BUFFER_SIZE 64
#define MESSAGE_BUFFER_SIZE 255

#define MAGIC_NUMBER 0x5A
#define BASE_EEPROM_ADDR 0

class AppSettings
{
public:
    AppSettings();

    void setBrightness(uint8_t _brightness);
    void setFlipped(bool _flipped);
    void setTimezone(const char _timezone[TIMEZONE_BUFFER_SIZE]);
    void setStockApiKey(const char _stockApiKey[STOCK_API_KEY_BUFFER_SIZE]);
    void setLatitude(float _latitude);
    void setLongitude(float _longitude);
    void setWeatherUnits(char _units);
    void setDateFormat(char _format);
    void setActiveCards(bool _activeCards[OPERATION_MODE_LENGTH]);
    void setCardOrder(uint8_t _cardOrder[OPERATION_MODE_LENGTH]);
    void setCardDurations(uint16_t _cardDurations[OPERATION_MODE_LENGTH]);
    void setMdnsDomain(const char _mdnsDomain[MDNS_DOMAIN_BUFFER_SIZE]);
    void setMessage(const char _message[MESSAGE_BUFFER_SIZE]);
    void factoryReset();

    uint8_t getBrightness() { return settings.display.brightness; }
    bool getFlipped() { return settings.display.flipped; }
    const char *getTimezone() { return settings.time.timezone; }
    const char *getStockApiKey() { return settings.stock.apiKey; }
    float getLatitude() { return settings.weather.latitude; }
    float getLongitude() { return settings.weather.longitude; }
    char getWeatherUnits() { return settings.weather.units; }
    char getDateFormat() { return settings.display.dateFormat; }
    bool *getActiveCards() { return settings.activeCards; }
    uint8_t *getCardOrder() { return settings.cardOrder; }
    uint16_t *getCardDurations() { return settings.cardDurations; }
    const char *getMdnsDomain() { return settings.network.mdnsDomain; }
    const char *getMessage() { return settings.message.content; }

    void toJson(JsonDocument &doc);

private:
    void setDefaultValues();
    struct _AppSettings
    {
        uint8_t magic;
        struct
        {
            char timezone[TIMEZONE_BUFFER_SIZE];
        } time;
        struct
        {
            char apiKey[STOCK_API_KEY_BUFFER_SIZE];
        } stock;
        struct
        {
            uint8_t brightness;
            bool flipped;
            char dateFormat;
        } display;
        struct
        {
            float latitude;
            float longitude;
            char units;
        } weather;
        struct
        {
            char mdnsDomain[MDNS_DOMAIN_BUFFER_SIZE];
        } network;
        struct
        {
            char content[MESSAGE_BUFFER_SIZE];
        } message;
        bool activeCards[OPERATION_MODE_LENGTH];
        uint8_t cardOrder[OPERATION_MODE_LENGTH]; // Store the order of cards
        uint16_t cardDurations[OPERATION_MODE_LENGTH]; // Store duration in seconds for each card
        // Alarm struct removed - was tied to buzzer functionality
    } settings;
};

#endif