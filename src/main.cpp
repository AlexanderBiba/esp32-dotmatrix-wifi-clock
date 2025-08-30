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
  // No printf here. For some reason nothing is printed during setup.
  Serial.begin(115200);
  EEPROM.begin(0x400);

  // Initialize watchdog timer
  esp_task_wdt_init(WATCHDOG_TIMEOUT, true);
  esp_task_wdt_add(NULL);

  settings = new AppSettings();
  appServer = new AppServer(settings);
  renderer = new Renderer(settings);
  clk = new Clock(settings);
  weather = new Weather(settings);
  stock = new Stock(settings);
  snake = new Snake(settings);
  
  // Feed watchdog after initialization
  esp_task_wdt_reset();
}

// Cleanup function to prevent memory leaks
void cleanup(void)
{
  if (snake) delete snake;
  if (stock) delete stock;
  if (weather) delete weather;
  if (clk) delete clk;
  if (renderer) delete renderer;
  if (appServer) delete appServer;
  if (settings) delete settings;
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

  if (doc.containsKey("brightness"))
  {
    settings->setBrightness(doc["brightness"]);
    renderer->updateBrightness();
  }
  if (doc.containsKey("timezone"))
  {
    settings->setTimezone(doc["timezone"]);
    clk->updateTime();
  }
  if (doc.containsKey("latitude"))
  {
    settings->setLatitude(doc["latitude"]);
    updateWeather = true;
  }
  if (doc.containsKey("longitude"))
  {
    settings->setLongitude(doc["longitude"]);
    updateWeather = true;
  }
  if (doc.containsKey("weatherUnits"))
  {
    settings->setWeatherUnits(!strcmp(doc["weatherUnits"], "c") ? 'c' : 'f');
    updateWeather = true;
  }
  // Alarm control removed - was tied to buzzer functionality

  // update weather if required
  if (updateWeather)
  {
    weather->updateWeatherData();
  }
}

void loop(void)
{
  // Feed watchdog timer to prevent reset
  esp_task_wdt_reset();
  
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
    for (uint8_t i = 0; i < numCards; i++) {
      if (activeCards[static_cast<int>(cards[i]->getOperationMode())]) {
        activeCardIndices[activeCardCount] = i;
        activeCardCount++;
      }
    }
    
    // If no cards are active, default to clock
    if (activeCardCount == 0) {
      currentState = 0;
      operationMode = cards[currentState]->getOperationMode();
      cardSwitchTime = cards[currentState]->getCardSwitchTime();
    }
    // If only one card is active, stay on it
    else if (activeCardCount == 1) {
      currentState = activeCardIndices[0];
    operationMode = cards[currentState]->getOperationMode();
    cardSwitchTime = cards[currentState]->getCardSwitchTime();
    }
    // If multiple cards are active, cycle through them
    else {
      // Find current position in active cards list
      uint8_t currentActiveIndex = 0;
      for (uint8_t i = 0; i < activeCardCount; i++) {
        if (activeCardIndices[i] == currentState) {
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
  
  try {
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
  } catch (...) {
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
    try {
    switch (operationMode)
    {
    case OperationMode::MESSAGE:
      renderer->setMessage(requestBuffer);
      break;
    case OperationMode::CLOCK:
      renderer->setRaw(clk->getTime());
      break;
    case OperationMode::DATE:
      renderer->setRaw(clk->getDate());
      break;
    case OperationMode::WEATHER:
      renderer->setRaw(weather->getWeather());
      break;
    case OperationMode::SNAKE:
      renderer->setRaw(snake->getSnake());
      break;
      case OperationMode::IP_ADDRESS:
        renderer->setMessage(WiFi.localIP().toString().c_str());
        break;
      }
    } catch (...) {
      printf("Error handling operation mode: %d\n", static_cast<int>(operationMode));
      // Fallback to clock mode on error
      operationMode = OperationMode::CLOCK;
    }
  }

  // scroll if required
  try {
  renderer->scrollText();
  } catch (...) {
    printf("Error in text scrolling\n");
  }
}
