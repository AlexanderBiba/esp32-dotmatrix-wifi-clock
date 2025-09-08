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

  esp_task_wdt_reset();
}

// Cleanup function to prevent memory leaks
void cleanup(void)
{
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
  printf("Control request: %s\n", requestBuffer);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, requestBuffer);
  if (error)
  {
    printf("Control deserializeJson() failed: %s\n", error.f_str());
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
    if (clk != nullptr) {
      clk->updateTime();
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
  if (doc["mdnsDomain"])
  {
    settings->setMdnsDomain(doc["mdnsDomain"]);
    // Note: MDNS restart would require WiFi restart, which is complex
    // For now, the new domain will take effect on next reboot
  }
  // Alarm control removed - was tied to buzzer functionality

  // update weather if required
  if (updateWeather && weather != nullptr)
  {
    weather->updateWeatherData();
  }
}

void loop(void)
{
  // Feed watchdog timer to prevent reset
  esp_task_wdt_reset();

  // Process WiFiManager (keeps captive portal alive)
  appServer->processWiFi();

  // CRITICAL: Check WiFi status first to prevent crashes
  // If WiFi is not connected, show configuration message and skip all other operations
  if (WiFi.status() != WL_CONNECTED)
  {

    static long lastWiFiCheck = 0;
    static bool setWifiMessage = false;
    if (!setWifiMessage && (millis() - lastWiFiCheck > 1000)) // Check every second
    {
      lastWiFiCheck = millis();
      renderer->setMessage("Connect to 'DotMatrix Clock' AP wifi network to configure device");
      setWifiMessage = true;
    }

    // Skip all other operations when WiFi is not connected
    renderer->scrollText();
    return;
  }

  // Create WiFi-dependent components when WiFi connects
  static bool componentsCreated = false;
  if (!componentsCreated) {
    clk = new Clock(settings);
    weather = new Weather(settings);
    stock = new Stock(settings);
    snake = new Snake(settings);
    componentsCreated = true;
  }

  static Card *cards[] = {
      new Card(OperationMode::CLOCK, 10000),
      new Card(OperationMode::DATE, 5000),
      new Card(OperationMode::WEATHER, 5000),
      new Card(OperationMode::SNAKE, 5000),
      new Card(OperationMode::MESSAGE, 5000),
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

    // Count active cards and find the next active one
    bool *activeCards = settings->getActiveCards();
    uint8_t activeCardCount = 0;
    uint8_t activeCardIndices[OPERATION_MODE_LENGTH];

    // Build list of active card indices
    for (uint8_t i = 0; i < numCards; i++)
    {
      if (activeCards[static_cast<int>(cards[i]->getOperationMode())])
      {
        activeCardIndices[activeCardCount] = i;
        activeCardCount++;
      }
    }

    // If no cards are active, show IP address
    if (activeCardCount == 0)
    {
      operationMode = OperationMode::IP_ADDRESS;
      cardSwitchTime = 5000; // Switch every 5 seconds
    }
    // If only one card is active, stay on it
    else if (activeCardCount == 1)
    {
      currentState = activeCardIndices[0];
      operationMode = cards[currentState]->getOperationMode();
      cardSwitchTime = cards[currentState]->getCardSwitchTime();
    }
    // If multiple cards are active, cycle through them
    else
    {
      // Find current position in active cards list
      uint8_t currentActiveIndex = 0;
      for (uint8_t i = 0; i < activeCardCount; i++)
      {
        if (activeCardIndices[i] == currentState)
        {
          currentActiveIndex = i;
          break;
        }
      }

      // Move to next active card
      currentActiveIndex = (currentActiveIndex + 1) % activeCardCount;
      currentState = activeCardIndices[currentActiveIndex];
      operationMode = cards[currentState]->getOperationMode();
      cardSwitchTime = cards[currentState]->getCardSwitchTime();
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
      break;
    }
    case AppServer::RequestMode::CNTL:
      handleControlRequest(requestBuffer);
      break;
    case AppServer::RequestMode::SETT:
      // Settings request handled by AppServer
      break;
    default:
      break;
    }
  }
  catch (...)
  {
    printf("Error handling WiFi request\n");
  }

  // Alarm handling removed - was tied to buzzer functionality

  // handle cards
  bool reset = prevOperationMode != operationMode;
  if (reset)
  {
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
        // WiFi status is already checked at the top of the loop
        // This only handles custom messages when WiFi is connected
        if (strlen(requestBuffer) == 0)
        {
          // Use default message if requestBuffer is empty
          renderer->setMessage("Hello World!");
        }
        else
        {
          // Use custom message
          renderer->setMessage(requestBuffer);
        }
        break;
      case OperationMode::CLOCK:
        if (clk != nullptr) {
          renderer->setRaw(clk->getTime());
        } else {
          renderer->setMessage("Clock not available");
        }
        break;
      case OperationMode::DATE:
        if (clk != nullptr) {
          renderer->setRaw(clk->getDate());
        } else {
          renderer->setMessage("Date not available");
        }
        break;
      case OperationMode::WEATHER:
        if (weather != nullptr) {
          renderer->setRaw(weather->getWeather());
        } else {
          renderer->setMessage("Weather not available");
        }
        break;
      case OperationMode::SNAKE:
        if (snake != nullptr) {
          renderer->setRaw(snake->getSnake());
        } else {
          renderer->setMessage("Snake not available");
        }
        break;
      case OperationMode::IP_ADDRESS:
      {
        // Create message with mDNS.local URL and IP address
        static char ipMessage[64];
        snprintf(ipMessage, sizeof(ipMessage), "http://%s.local     %s",
                 settings->getMdnsDomain(), WiFi.localIP().toString().c_str());
        renderer->setMessage(ipMessage);
      }
      break;
      }
    }
    catch (...)
    {
      printf("Error handling operation mode: %d\n", static_cast<int>(operationMode));
      // Fallback to clock mode on error
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
    printf("Error in text scrolling\n");
  }
}
