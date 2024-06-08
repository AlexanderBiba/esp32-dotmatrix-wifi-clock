#include <ctime>
#include <cstring>
#include <ctype.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#include "main.h"
#include "clk.h"
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
  EEPROM.begin(0x200);

  readSettings(&settings);

  setupRenderer(&settings);

  char buffer[MAX_MSG_SIZE];
  setupWiFi(buffer);
  // setMessage(buffer);

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
}

void loop(void)
{
  static OperationMode operationMode = OperationMode::CLK;

  static char requestBuffer[1024];
  static char timeBuffer[TIME_BUFFER_SIZE];
  static char msgBuffer[MAX_MSG_SIZE];

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
    setupClk();
    break;
  case AppRequestMode::CNTL:
    handleControlRequest(requestBuffer);
    break;
  default:
    break;
  }

  bool reset = false;
  if (prevOperationMode != operationMode)
  {
    setScrollContent(true);
    reset = true;
  }

  switch (operationMode)
  {
  case OperationMode::MSG:
    break;
  case OperationMode::CLK:
    if (getTime(timeBuffer))
    {
      PRINT("Time: ", timeBuffer);
      setClock(timeBuffer);
    }
    break;
  }

  scrollText();
}
