#include <EEPROM.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <string.h>

#include "settings.h"

AppSettings::AppSettings()
{
    EEPROM.get(BASE_EEPROM_ADDR, settings);
    if (settings.magic != MAGIC_NUMBER)
    {
        Serial.println("No settings found, using defaults");
        setDefaultValues();
        
        // Validate settings before writing
        EEPROM.put(BASE_EEPROM_ADDR, settings);
        if (!EEPROM.commit()) {
            Serial.println("Failed to commit settings to EEPROM");
        }
    }
    else
    {
        Serial.println("Settings loaded from EEPROM");
        // Serial.printf("Card order from EEPROM: ");
        // for (int i = 0; i < OPERATION_MODE_LENGTH; i++)
        // {
        //   Serial.printf("%d ", settings.cardOrder[i]);
        // }
        // Serial.println();
        
        // Check if card order needs to be initialized (for existing devices)
        // This happens when the cardOrder field was added to an existing structure
        bool needsCardOrderInit = false;
        for (int i = 0; i < OPERATION_MODE_LENGTH; i++)
        {
          if (settings.cardOrder[i] >= OPERATION_MODE_LENGTH)
          {
            needsCardOrderInit = true;
            break;
          }
        }
        
        // Also check if all values are 0 (uninitialized)
        if (!needsCardOrderInit)
        {
          bool allZero = true;
          for (int i = 0; i < OPERATION_MODE_LENGTH; i++)
          {
            if (settings.cardOrder[i] != 0)
            {
              allZero = false;
              break;
            }
          }
          if (allZero)
          {
            needsCardOrderInit = true;
          }
        }
        
        if (needsCardOrderInit)
        {
          Serial.println("Initializing card order for existing device");
          for (int i = 0; i < OPERATION_MODE_LENGTH; ++i)
          {
            settings.cardOrder[i] = i; // Default order: 0, 1, 2, 3, 4, 5, 6
          }
          EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, cardOrder), settings.cardOrder);
          if (!EEPROM.commit()) {
            Serial.println("Failed to commit card order initialization");
          }
        }
        
        // Check if card durations need to be initialized (for existing devices)
        bool needsCardDurationsInit = false;
        for (int i = 0; i < OPERATION_MODE_LENGTH; i++)
        {
          if (settings.cardDurations[i] == 0)
          {
            needsCardDurationsInit = true;
            break;
          }
        }
        
        if (needsCardDurationsInit)
        {
          Serial.println("Initializing card durations for existing device");
          for (int i = 0; i < OPERATION_MODE_LENGTH; ++i)
          {
            // Set default durations (in seconds)
            switch (i) {
                case 0: // CLOCK
                    settings.cardDurations[i] = 10; // 10 seconds
                    break;
                case 1: // DATE
                    settings.cardDurations[i] = 5; // 5 seconds
                    break;
                case 2: // WEATHER
                    settings.cardDurations[i] = 5; // 5 seconds
                    break;
                case 3: // SNAKE
                    settings.cardDurations[i] = 5; // 5 seconds
                    break;
                case 4: // MESSAGE
                    settings.cardDurations[i] = 5; // 5 seconds
                    break;
                case 5: // IP_ADDRESS
                    settings.cardDurations[i] = 5; // 5 seconds
                    break;
                case 6: // RAIN
                    settings.cardDurations[i] = 5; // 5 seconds
                    break;
                default:
                    settings.cardDurations[i] = 5; // Default 5 seconds
                    break;
            }
          }
          EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, cardDurations), settings.cardDurations);
          if (!EEPROM.commit()) {
            Serial.println("Failed to commit card durations initialization");
          }
        }
    }
    
    // Validate loaded settings
    if (settings.display.brightness > 0xf) settings.display.brightness = 0xf;
    
    // Validate MDNS domain - ensure it's not empty or contains invalid characters
    bool mdnsDomainValid = true;
    if (strlen(settings.network.mdnsDomain) == 0) {
        mdnsDomainValid = false;
    } else {
        // Check for invalid characters in MDNS domain
        for (int i = 0; i < strlen(settings.network.mdnsDomain); i++) {
            char c = settings.network.mdnsDomain[i];
            if (!isalnum(c) && c != '-' && c != '_') {
                mdnsDomainValid = false;
                break;
            }
        }
    }
    
    if (!mdnsDomainValid) {
        Serial.println("Invalid MDNS domain found, resetting to default");
        strcpy(settings.network.mdnsDomain, "digiclk");
        EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, network.mdnsDomain), settings.network.mdnsDomain);
        if (!EEPROM.commit()) {
            Serial.println("Failed to commit MDNS domain fix");
        }
    }
}

void AppSettings::setBrightness(uint8_t _brightness)
{
    if (_brightness > 0xf) {
        _brightness = 0xf; // Clamp to valid range
    }
    settings.display.brightness = _brightness;
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, display.brightness), _brightness);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit brightness setting");
    }
}

void AppSettings::setFlipped(bool _flipped)
{
    settings.display.flipped = _flipped;
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, display.flipped), _flipped);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit flipped setting");
    }
}

void AppSettings::setTimezone(const char _timezone[TIMEZONE_BUFFER_SIZE])
{
    if (strlen(_timezone) >= TIMEZONE_BUFFER_SIZE) {
        Serial.println("Timezone too long, truncated");
    }
    strncpy(settings.time.timezone, _timezone, TIMEZONE_BUFFER_SIZE - 1);
    settings.time.timezone[TIMEZONE_BUFFER_SIZE - 1] = '\0'; // Ensure null termination
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, time.timezone), settings.time.timezone);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit timezone setting");
    }
}

void AppSettings::setStockApiKey(const char _stockApiKey[STOCK_API_KEY_BUFFER_SIZE])
{
    if (strlen(_stockApiKey) >= STOCK_API_KEY_BUFFER_SIZE) {
        Serial.println("API key too long, truncated");
    }
    strncpy(settings.stock.apiKey, _stockApiKey, STOCK_API_KEY_BUFFER_SIZE - 1);
    settings.stock.apiKey[STOCK_API_KEY_BUFFER_SIZE - 1] = '\0'; // Ensure null termination
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, stock.apiKey), settings.stock.apiKey);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit API key setting");
    }
}

void AppSettings::setLatitude(float _latitude)
{
    if (_latitude < -90.0f || _latitude > 90.0f) {
        Serial.println("Invalid latitude value");
        return;
    }
    settings.weather.latitude = _latitude;
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, weather.latitude), settings.weather.latitude);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit latitude setting");
    }
}

void AppSettings::setLongitude(float _longitude)
{
    if (_longitude < -180.0f || _longitude > 180.0f) {
        Serial.println("Invalid longitude value");
        return;
    }
    settings.weather.longitude = _longitude;
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, weather.longitude), settings.weather.longitude);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit longitude setting");
    }
}

void AppSettings::setWeatherUnits(char _units)
{
    if (_units != 'c' && _units != 'f') {
        Serial.println("Invalid weather units");
        return;
    }
    settings.weather.units = _units;
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, weather.units), settings.weather.units);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit weather units setting");
    }
}

void AppSettings::setActiveCards(bool _activeCards[OPERATION_MODE_LENGTH])
{
    for (int i = 0; i < OPERATION_MODE_LENGTH; ++i)
    {
        settings.activeCards[i] = _activeCards[i];
    }
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, activeCards), settings.activeCards);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit active cards setting");
    }
}

void AppSettings::setCardOrder(uint8_t _cardOrder[OPERATION_MODE_LENGTH])
{
    for (int i = 0; i < OPERATION_MODE_LENGTH; ++i)
    {
        settings.cardOrder[i] = _cardOrder[i];
    }
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, cardOrder), settings.cardOrder);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit card order setting");
    }
}

void AppSettings::setCardDurations(uint16_t _cardDurations[OPERATION_MODE_LENGTH])
{
    for (int i = 0; i < OPERATION_MODE_LENGTH; ++i)
    {
        // Validate duration (minimum 1 second, maximum 300 seconds = 5 minutes)
        if (_cardDurations[i] < 1) {
            settings.cardDurations[i] = 1;
        } else if (_cardDurations[i] > 300) {
            settings.cardDurations[i] = 300;
        } else {
            settings.cardDurations[i] = _cardDurations[i];
        }
    }
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, cardDurations), settings.cardDurations);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit card durations setting");
    }
}

void AppSettings::setMdnsDomain(const char _mdnsDomain[MDNS_DOMAIN_BUFFER_SIZE])
{
    if (strlen(_mdnsDomain) >= MDNS_DOMAIN_BUFFER_SIZE) {
        Serial.println("MDNS domain too long, truncated");
    }
    strncpy(settings.network.mdnsDomain, _mdnsDomain, MDNS_DOMAIN_BUFFER_SIZE - 1);
    settings.network.mdnsDomain[MDNS_DOMAIN_BUFFER_SIZE - 1] = '\0'; // Ensure null termination
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, network.mdnsDomain), settings.network.mdnsDomain);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit MDNS domain setting");
    }
}

void AppSettings::setMessage(const char _message[MESSAGE_BUFFER_SIZE])
{
    if (strlen(_message) >= MESSAGE_BUFFER_SIZE) {
        Serial.println("Message too long, truncated");
    }
    strncpy(settings.message.content, _message, MESSAGE_BUFFER_SIZE - 1);
    settings.message.content[MESSAGE_BUFFER_SIZE - 1] = '\0'; // Ensure null termination
    EEPROM.put(BASE_EEPROM_ADDR + offsetof(_AppSettings, message.content), settings.message.content);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit message setting");
    }
}


void AppSettings::factoryReset()
{
    Serial.println("Performing factory reset - clearing all settings");
    
    // Clear the entire EEPROM settings structure
    memset(&settings, 0, sizeof(_AppSettings));
    
    // Set default values
    setDefaultValues();
    
    // Write the reset settings to EEPROM
    EEPROM.put(BASE_EEPROM_ADDR, settings);
    if (!EEPROM.commit()) {
        Serial.println("Failed to commit factory reset settings to EEPROM");
    } else {
        Serial.println("Factory reset completed successfully");
    }
}

void AppSettings::setDefaultValues()
{
    settings.magic = MAGIC_NUMBER;
    strcpy(settings.time.timezone, "America/New_York");
    strcpy(settings.stock.apiKey, "\0");
    strcpy(settings.network.mdnsDomain, "digiclk");
    strcpy(settings.message.content, "Hello World!");
    settings.display.brightness = 0xf;
    settings.display.flipped = false;
    settings.weather.latitude = 40.7128f;  // New York City latitude
    settings.weather.longitude = -74.0060f; // New York City longitude
    settings.weather.units = 'f';
    
    // Disable all cards by default - this will trigger the WiFi configuration message
    // when WiFi is not connected, or show IP address when WiFi is connected
    for (int i = 0; i < OPERATION_MODE_LENGTH; ++i)
    {
        settings.activeCards[i] = false;
        settings.cardOrder[i] = i; // Default order: 0, 1, 2, 3, 4, 5, 6
        
        // Set default durations (in seconds)
        switch (i) {
            case 0: // CLOCK
                settings.cardDurations[i] = 10; // 10 seconds
                break;
            case 1: // DATE
                settings.cardDurations[i] = 5; // 5 seconds
                break;
            case 2: // WEATHER
                settings.cardDurations[i] = 5; // 5 seconds
                break;
            case 3: // SNAKE
                settings.cardDurations[i] = 5; // 5 seconds
                break;
            case 4: // MESSAGE
                settings.cardDurations[i] = 5; // 5 seconds
                break;
            case 5: // IP_ADDRESS
                settings.cardDurations[i] = 5; // 5 seconds
                break;
            case 6: // RAIN
                settings.cardDurations[i] = 5; // 5 seconds
                break;
            default:
                settings.cardDurations[i] = 5; // Default 5 seconds
                break;
        }
    }
    // Don't enable any cards by default - let the main loop handle the display logic
}

void AppSettings::toJson(JsonDocument &doc)
{
    doc["timezone"] = settings.time.timezone;
    doc["brightness"] = settings.display.brightness;
    doc["flipped"] = settings.display.flipped;
    doc["latitude"] = settings.weather.latitude;
    doc["longitude"] = settings.weather.longitude;
    doc["weatherUnits"] = settings.weather.units == 'f' ? "f" : "c";
    doc["message"] = settings.message.content;
    
    // Ensure MDNS domain is valid for JSON output
    const char* mdnsDomain = settings.network.mdnsDomain;
    if (strlen(mdnsDomain) == 0) {
        doc["mdnsDomain"] = "digiclk";
    } else {
        doc["mdnsDomain"] = mdnsDomain;
    }
    
    doc["activeCards"].to<JsonArray>();
    for (int i = 0; i < OPERATION_MODE_LENGTH; ++i)
    {
        if (settings.activeCards[i])
        {
            doc["activeCards"].add(OperationModeStr[i]);
        }
    }
    
    doc["cardOrder"].to<JsonArray>();
    for (int i = 0; i < OPERATION_MODE_LENGTH; ++i)
    {
        doc["cardOrder"].add(settings.cardOrder[i]);
    }
    
    doc["cardDurations"].to<JsonArray>();
    for (int i = 0; i < OPERATION_MODE_LENGTH; ++i)
    {
        doc["cardDurations"].add(settings.cardDurations[i]);
    }
}