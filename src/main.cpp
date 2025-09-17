#include <ctime>
#include <cstring>
#include <ctype.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <esp_task_wdt.h>
#include <WiFi.h>

#include "main.h"
#include "clock.h"
#include "stock.h"
#include "appserver.h"
#include "renderer.h"
#include "settings.h"
#include "weather.h"
#include "card.h"
#include "snake.h"
#include "rain.h"

#define PRINT_CALLBACK 0
#define DEBUG 1
#define LED_HEARTBEAT 0
#define WATCHDOG_TIMEOUT 30 // 30 seconds

AppSettings *settings;
Clock *clk;
AppServer *appServer;
Renderer *renderer;
Stock *stock;
Weather *weather;
Snake *snake;
Rain *rain;

void setup(void)
{
    Serial.begin(115200);
    delay(1000);

    EEPROM.begin(0x400);
    esp_task_wdt_init(WATCHDOG_TIMEOUT, true);
    esp_task_wdt_add(NULL);

    settings = new AppSettings();
    renderer = new Renderer(settings);
    appServer = new AppServer(settings);

    // WiFi-dependent components created when WiFi connects
    clk = nullptr;
    weather = nullptr;
    stock = nullptr;
    snake = nullptr;
    rain = nullptr;

    esp_task_wdt_reset();
}

// Cleanup function to prevent memory leaks
void cleanup(void)
{
    if (rain)
        delete rain;
    if (snake)
        delete snake;
    if (stock)
        delete stock;
    if (weather)
        delete weather;
    if (clk)
        delete clk;
    if (renderer)
        delete renderer;
    if (appServer)
        delete appServer;
    if (settings)
        delete settings;
}

void handleControlRequest(char *requestBuffer)
{
    Serial.printf("Control request: %s\n", requestBuffer);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, requestBuffer);
    if (error)
    {
        Serial.printf("Control deserializeJson() failed: %s\n", error.f_str());
        return;
    }

    bool updateWeather = false;

    if (doc["brightness"])
    {
        settings->setBrightness(doc["brightness"]);
        renderer->updateBrightness();
    }
    if (doc["timezone"])
    {
        settings->setTimezone(doc["timezone"]);
        if (clk != nullptr)
        {
            clk->updateTimezone();
        }
    }
    if (doc["latitude"])
    {
        settings->setLatitude(doc["latitude"]);
        updateWeather = true;
    }
    if (doc["longitude"])
    {
        settings->setLongitude(doc["longitude"]);
        updateWeather = true;
    }
    if (doc["weatherUnits"])
    {
        settings->setWeatherUnits(!strcmp(doc["weatherUnits"], "c") ? 'c' : 'f');
        updateWeather = true;
    }
    if (doc["dateFormat"])
    {
        settings->setDateFormat(!strcmp(doc["dateFormat"], "ddmm") ? 'd' : 'm');
    }
    if (doc["mdnsDomain"])
    {
        settings->setMdnsDomain(doc["mdnsDomain"]);
    }
    if (doc["message"])
    {
        settings->setMessage(doc["message"]);
    }

    if (updateWeather && weather != nullptr)
    {
        weather->updateWeatherData();
    }
}


// Helper function to identify scrolling modes
bool isScrollingMode(OperationMode mode)
{
    return mode == OperationMode::MESSAGE || mode == OperationMode::IP_ADDRESS;
}

void loop(void)
{
    esp_task_wdt_reset();

    appServer->processWiFi();

    if (WiFi.status() != WL_CONNECTED)
    {
        static long lastWiFiCheck = 0;
        static bool setWifiMessage = false;
        if (!setWifiMessage && (millis() - lastWiFiCheck > 1000))
        {
            lastWiFiCheck = millis();
            renderer->setMessage("Connect to 'DotMatrix Clock' AP wifi network to configure device");
            setWifiMessage = true;
        }
        renderer->scrollText();
        return;
    }

    static bool componentsCreated = false;
    if (!componentsCreated)
    {
        clk = new Clock(settings);
        weather = new Weather(settings);
        stock = new Stock(settings);
        snake = new Snake(settings);
        rain = new Rain(settings);
        componentsCreated = true;
    }

    static Card *cards[] = {
        new Card(OperationMode::CLOCK, 10000),
        new Card(OperationMode::DATE, 5000),
        new Card(OperationMode::WEATHER, 5000),
        new Card(OperationMode::MESSAGE, 5000),
        new Card(OperationMode::SNAKE, 5000),
        new Card(OperationMode::RAIN, 5000),
        new Card(OperationMode::IP_ADDRESS, 5000)};
    static const size_t numCards = sizeof(cards) / sizeof(cards[0]);
    static uint8_t currentState = 0;
    static OperationMode operationMode = cards[currentState]->getOperationMode();

    OperationMode prevOperationMode = operationMode;

    // card switching logic
    static long curCardTime = 0;
    static long cardSwitchTime = 5000;
    if (millis() - curCardTime > cardSwitchTime)
    {
        curCardTime = millis();

        bool *activeCards = settings->getActiveCards();
        uint8_t *cardOrder = settings->getCardOrder();
        uint8_t activeCardCount = 0;
        uint8_t activeCardIndices[OPERATION_MODE_LENGTH];

        Serial.printf("Current card order: ");
        for (int i = 0; i < OPERATION_MODE_LENGTH; i++)
        {
            Serial.printf("%d ", cardOrder[i]);
        }
        Serial.println();

        Serial.printf("Active cards: ");
        for (int i = 0; i < OPERATION_MODE_LENGTH; i++)
        {
            Serial.printf("%d ", activeCards[i]);
        }
        Serial.println();

        for (uint8_t orderIndex = 0; orderIndex < OPERATION_MODE_LENGTH; orderIndex++)
        {
            uint8_t cardIndex = cardOrder[orderIndex];
            if (cardIndex < numCards && activeCards[static_cast<int>(cards[cardIndex]->getOperationMode())])
            {
                activeCardIndices[activeCardCount] = cardIndex;
                activeCardCount++;
            }
        }

        Serial.printf("Active card count: %d\n", activeCardCount);

        if (activeCardCount == 0)
        {
            operationMode = OperationMode::IP_ADDRESS;
            cardSwitchTime = 5000;
        }
        else if (activeCardCount == 1)
        {
            currentState = activeCardIndices[0];
            operationMode = cards[currentState]->getOperationMode();
            uint16_t *cardDurations = settings->getCardDurations();
            cardSwitchTime = cardDurations[static_cast<int>(operationMode)] * 1000;
        }
        else
        {
            uint8_t currentActiveIndex = 0;
            for (uint8_t i = 0; i < activeCardCount; i++)
            {
                if (activeCardIndices[i] == currentState)
                {
                    currentActiveIndex = i;
                    break;
                }
            }

            currentActiveIndex = (currentActiveIndex + 1) % activeCardCount;
            currentState = activeCardIndices[currentActiveIndex];
            operationMode = cards[currentState]->getOperationMode();
            uint16_t *cardDurations = settings->getCardDurations();
            cardSwitchTime = cardDurations[static_cast<int>(operationMode)] * 1000;
        }
    }

    // handle control requests
    static char requestBuffer[REQUEST_BUFFER_SIZE];
    bool activeCards[OPERATION_MODE_LENGTH] = {0};

    try
    {
        switch (appServer->handleWiFi(requestBuffer, activeCards))
        {
        case AppServer::RequestMode::MODE:
        {
            settings->setActiveCards(activeCards);
            // If MESSAGE card is active and we have message content, store it
            if (activeCards[static_cast<int>(OperationMode::MESSAGE)] && strlen(requestBuffer) > 0)
            {
                settings->setMessage(requestBuffer);
            }
            break;
        }
        case AppServer::RequestMode::CNTL:
            handleControlRequest(requestBuffer);
            break;
        case AppServer::RequestMode::SETT:
        case AppServer::RequestMode::CARD_ORDER:
        case AppServer::RequestMode::CARD_DURATIONS:
            break;
        default:
            break;
        }
    }
    catch (...)
    {
        Serial.println("Error handling WiFi request");
    }

    // handle cards
    bool reset = prevOperationMode != operationMode;
    if (reset)
    {
        // If the previous card was a long-running scrolling message,
        // tell it to finish up gracefully.
        if (isScrollingMode(prevOperationMode))
        {
            renderer->stopMessage();
        }

        // For ANY transition to a new card, we want to trigger the slide-in effect.
        // Setting scrollContent to true enables the animation engine.
        renderer->setScrollContent(true);
    }

    static long prevTime = 0;
    if (reset || millis() - prevTime > 100)
    {
        prevTime = millis();
        try
        {
            switch (operationMode)
            {
            case OperationMode::MESSAGE:
                renderer->setMessage(settings->getMessage());
                break;
            case OperationMode::CLOCK:
                if (clk != nullptr)
                {
                    renderer->setRaw(clk->getTime());
                }
                else
                {
                    renderer->setMessage("Clock not available");
                }
                break;
            case OperationMode::DATE:
                if (clk != nullptr)
                {
                    renderer->setRaw(clk->getDate());
                }
                else
                {
                    renderer->setMessage("Date not available");
                }
                break;
            case OperationMode::WEATHER:
                if (weather != nullptr)
                {
                    renderer->setRaw(weather->getWeather());
                }
                else
                {
                    renderer->setMessage("Weather not available");
                }
                break;
            case OperationMode::SNAKE:
                if (snake != nullptr)
                {
                    renderer->setRaw(snake->getSnake());
                }
                else
                {
                    renderer->setMessage("Snake not available");
                }
                break;
            case OperationMode::RAIN:
                if (rain != nullptr)
                {
                    renderer->setRaw(rain->getRain());
                }
                else
                {
                    renderer->setMessage("Rain not available");
                }
                break;
            case OperationMode::IP_ADDRESS:
            {
                static char ipMessage[64];
                snprintf(ipMessage, sizeof(ipMessage), "http://%s.local      %s",
                         settings->getMdnsDomain(), WiFi.localIP().toString().c_str());
                renderer->setMessage(ipMessage);
            }
            break;
            }
        }
        catch (...)
        {
            Serial.printf("Error handling operation mode: %d\n", static_cast<int>(operationMode));
            operationMode = OperationMode::CLOCK;
        }
    }

    // scroll if required
    try
    {
        renderer->scrollText();
    }
    catch (...)
    {
        Serial.println("Error in text scrolling");
    }
}