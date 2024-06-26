#include <ctime>
#include <cstring>
#include <ctype.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#include "main.h"
#include "clock.h"
#include "stock.h"
#include "appserver.h"
#include "renderer.h"
#include "settings.h"
#include "weather.h"
#include "card.h"

#define PRINT_CALLBACK 0
#define DEBUG 1
#define LED_HEARTBEAT 0

AppSettings *settings;
Clock *clk;
AppServer *appServer;
Renderer *renderer;
Stock *stock;
Weather *weather;

void setup(void)
{
  Serial.begin(115200);
  EEPROM.begin(0x400);

  settings = new AppSettings();
  appServer = new AppServer();
  renderer = new Renderer(settings);
  clk = new Clock(settings);
  weather = new Weather(settings);
  stock = new Stock(settings);
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
  if (doc.containsKey("finazonApiKey"))
  {
    settings->setStockApiKey(doc["finazonApiKey"]);
  }

  if (updateWeather)
  {
    weather->updateWeatherData();
  }
}

void loop(void)
{
  static Card *cards[] = {
      new Card(OperationMode::CLOCK, 10000),
      new Card(OperationMode::DATE, 5000),
      new Card(OperationMode::WEATHER, 5000)};
  static uint8_t currentState = 0;
  static OperationMode operationMode = cards[currentState]->getOperationMode();

  OperationMode prevOperationMode = operationMode;

  static long curCardTime = 0;
  static long cardSwitchTime = 5000;
  if (millis() - curCardTime > cardSwitchTime)
  {
    printf("Switching card\n");
    curCardTime = millis();
    currentState = (currentState + 1) % (sizeof(cards) / sizeof(cards[0]));
    operationMode = cards[currentState]->getOperationMode();
    cardSwitchTime = cards[currentState]->getCardSwitchTime();
    printf("New card: %d\n", operationMode);
    printf("New card switch time: %ld\n", cardSwitchTime);
  }

  static char requestBuffer[REQUEST_BUFFER_SIZE];
  switch (appServer->handleWiFi(requestBuffer))
  {
  // case AppServer::RequestMode::MESSAGE:
  //   operationMode = OperationMode::MESSAGE;
  //   renderer->setMessage(requestBuffer);
  //   break;
  // case AppServer::RequestMode::CLOCK:
  //   operationMode = OperationMode::CLOCK;
  //   break;
  // case AppServer::RequestMode::WEATHER:
  //   operationMode = OperationMode::WEATHER;
  //   break;
  // case AppServer::RequestMode::STOCK:
  //   operationMode = OperationMode::STOCK;
  //   stock->setTicker(requestBuffer);
  //   renderer->setMessage(">>");
  //   break;
  case AppServer::RequestMode::CNTL:
    handleControlRequest(requestBuffer);
    break;
  default:
    break;
  }

  bool reset = prevOperationMode != operationMode;
  if (reset)
  {
    renderer->setScrollContent(true);
  }

  static long prevTime = 0;
  if (reset || millis() - prevTime > 200)
  {
    prevTime = millis();
    switch (operationMode)
    {
    case OperationMode::MESSAGE:
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
    case OperationMode::STOCK:
      renderer->setMessage(stock->getQuote());
      break;
    }
  }

  renderer->scrollText();
}
