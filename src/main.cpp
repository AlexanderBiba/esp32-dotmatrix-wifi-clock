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

#define PRINT_CALLBACK 0
#define DEBUG 1
#define LED_HEARTBEAT 0

AppSettings *settings;
Clock *clk;
AppServer *appServer;
Renderer *renderer;
Stock *stock;

void setup(void)
{
  Serial.begin(115200);
  EEPROM.begin(0x400);

  settings = new AppSettings();
  appServer = new AppServer();
  renderer = new Renderer(settings);
  stock = new Stock(settings);
  clk = new Clock(settings);
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
  if (doc.containsKey("finazonApiKey"))
  {
    settings->setStockApiKey(doc["finazonApiKey"]);
  }
}

void loop(void)
{
  static OperationMode operationMode = OperationMode::CLOCK;
  static char requestBuffer[REQUEST_BUFFER_SIZE];

  OperationMode prevOperationMode = operationMode;

  switch (appServer->handleWiFi(requestBuffer))
  {
  case AppServer::RequestMode::MSG:
    operationMode = OperationMode::MSG;
    renderer->setMessage(requestBuffer);
    break;
  case AppServer::RequestMode::CLOCK:
    operationMode = OperationMode::CLOCK;
    break;
  case AppServer::RequestMode::STOCK:
    operationMode = OperationMode::STOCK;
    stock->setTicker(requestBuffer);
    renderer->setMessage("Getting data...");
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
    case OperationMode::MSG:
      break;
    case OperationMode::CLOCK:
      renderer->setRaw(clk->getTime());
      break;
    case OperationMode::STOCK:
      renderer->setMessage(stock->getQuote());
      break;
    }
  }

  renderer->scrollText();
}
