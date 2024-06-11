#include <ctime>
#include <cstring>
#include <ctype.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#include "main.h"
#include "clk.h"
#include "stock.h"
#include "app_wifi.h"
#include "utils.h"
#include "renderer.h"
#include "app_settings.h"

#define PRINT_CALLBACK 0
#define DEBUG 1
#define LED_HEARTBEAT 0

AppSettings settings;

void setup(void)
{
  Serial.begin(115200);
  EEPROM.begin(0x400);

  readSettings(&settings);

  setupRenderer(&settings);

  char buffer[REQUEST_BUFFER_SIZE];
  setupWiFi(buffer);
  // setMessage(buffer);

  setupStocks(&settings);
  setupClk(&settings);
}

void handleControlRequest(char *requestBuffer)
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, requestBuffer);
  if (error)
  {
    PRINT("Control deserializeJson() failed: ", error.f_str());
    return;
  }
  if (doc.containsKey("brightness"))
  {
    uint8_t brightness = doc["brightness"];
    controlRenderer(ControlRequest::Intensity, brightness);
    persistBrightness(brightness);
  }
  if (doc.containsKey("timezone"))
  {
    const char *timezone = doc["timezone"];
    setTimezone(timezone);
    persistTimezone(timezone);
  }
  if (doc.containsKey("scrollDelay"))
  {
    uint16_t delay = doc["scrollDelay"];
    setScrollDelayMs(delay);
    persistScrollDelay(delay);
  }
  if (doc.containsKey("finazonApiKey"))
  {
    const char *apiKey = doc["finazonApiKey"];
    setApiKey(apiKey);
    persistStockApiKey(apiKey);
  }
}

void loop(void)
{
  static OperationMode operationMode = OperationMode::CLK;

  static char requestBuffer[1024];
  static char stockBuffer[STOCK_BUFFER_SIZE];
  static char timeBuffer[TIME_BUFFER_SIZE];
  static char msgBuffer[REQUEST_BUFFER_SIZE];

  static bool once = true;

  if (once)
  {
    once = false;
    PRINT("Settings: ", settings.timezone);
    PRINT("Settings: ", settings.stockApiKey);
    PRINT("Settings: ", settings.brightness);
    PRINT("Settings: ", settings.scrollDelay);
    PRINT("Settings: ", settings.magic);
  }

  OperationMode prevOperationMode = operationMode;

  AppRequestMode incomingRequest = handleWiFi(requestBuffer);

  switch (incomingRequest)
  {
  case AppRequestMode::MSG:
    operationMode = OperationMode::MSG;
    strcpy(msgBuffer, requestBuffer);
    setMessage(msgBuffer);
    break;
  case AppRequestMode::CLK:
    operationMode = OperationMode::CLK;
    break;
  case AppRequestMode::STOCK:
    operationMode = OperationMode::STOCK;
    setTicker(requestBuffer);
    setMessage("Getting data...");
    break;
  case AppRequestMode::CNTL:
    handleControlRequest(requestBuffer);
    break;
  default:
    break;
  }

  if (prevOperationMode != operationMode)
  {
    setScrollContent(true);
  }

  static long prevTime = 0;
  if (millis() - prevTime > 200)
  {
    prevTime = millis();
    switch (operationMode)
    {
    case OperationMode::MSG:
      break;
    case OperationMode::CLK:
      if (getTime(timeBuffer))
      {
        PRINT("Time: ", timeBuffer);
        static uint8_t rawClkBuffer[MAX_DEVICES * 8];
        parseTime(timeBuffer, rawClkBuffer);
        setRaw(rawClkBuffer);
      }
      break;
    case OperationMode::STOCK:
      if (getQuote(stockBuffer))
      {
        PRINT("Quote: ", stockBuffer);
        setMessage(stockBuffer);
      }
      break;
    }
  }

  scrollText();
}
