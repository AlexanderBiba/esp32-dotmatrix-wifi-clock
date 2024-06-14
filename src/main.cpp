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
#include "utils.h"
#include "renderer.h"
#include "settings.h"
#include "weather.h"

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
  PRINT("Control request: ", requestBuffer);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, requestBuffer);
  if (error)
  {
    PRINT("Control deserializeJson() failed: ", error.f_str());
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
  if (doc.containsKey("scrollDelay"))
  {
    settings->setScrollDelay(doc["scrollDelay"]);
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
  static OperationMode operationMode = OperationMode::WEATHER;
  static char requestBuffer[REQUEST_BUFFER_SIZE];

  OperationMode prevOperationMode = operationMode;

  switch (appServer->handleWiFi(requestBuffer))
  {
  case AppServer::RequestMode::MESSAGE:
    operationMode = OperationMode::MESSAGE;
    renderer->setMessage(requestBuffer);
    break;
  case AppServer::RequestMode::CLOCK:
    operationMode = OperationMode::CLOCK;
    break;
  case AppServer::RequestMode::WEATHER:
    operationMode = OperationMode::WEATHER;
    renderer->setMessage(">>");
    break;
  case AppServer::RequestMode::STOCK:
    operationMode = OperationMode::STOCK;
    stock->setTicker(requestBuffer);
    renderer->setMessage(">>");
    break;
  case AppServer::RequestMode::CNTL:
    handleControlRequest(requestBuffer);
    break;
  default:
    break;
  }

  if (prevOperationMode != operationMode)
  {
    renderer->setScrollContent(true);
  }

  static long prevTime = 0;
  if (millis() - prevTime > 200)
  {
    prevTime = millis();
    switch (operationMode)
    {
    case OperationMode::MESSAGE:
      break;
    case OperationMode::CLOCK:
      renderer->setRaw(clk->getTime());
      break;
    case OperationMode::WEATHER:
      // alternate between showing time and weather
      static bool showTime = false;
      static long prevSwitchTime = 0;
      if (showTime)
      {
        renderer->setRaw(clk->getTime());
      }
      if (millis() - prevSwitchTime > 5000)
      {
        renderer->setScrollContent(true);
        showTime = !showTime;
        prevSwitchTime = millis();
        if (showTime)
        {
          renderer->setRaw(clk->getTime());
        }
        else
        {
          renderer->setRaw(weather->getWeather());
        }
      }
      break;
    case OperationMode::STOCK:
      renderer->setMessage(stock->getQuote());
      break;
    }
  }

  renderer->scrollText();
}
