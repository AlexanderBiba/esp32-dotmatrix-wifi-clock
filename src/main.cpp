#include <ctime>
#include <cstring>
#include <ctype.h>
#include <Arduino.h>

#include "main.h"
#include "clk.h"
#include "app_wifi.h"
#include "utils.h"
#include "renderer.h"

#define PRINT_CALLBACK 0
#define DEBUG 1
#define LED_HEARTBEAT 0

void setup(void)
{
  Serial.begin(115200);

  setDefaultOperationMode(OperationMode::CLK);

  setupRenderer();

  char buffer[MAX_MSG_SIZE];
  setupWiFi(buffer);
  setMessage(buffer);

  setupClk();
}

void loop(void)
{
  static OperationMode operationMode;

  static char timeBuffer[TIME_BUFFER_SIZE];
  static char msgBuffer[MAX_MSG_SIZE];

  OperationMode prevOperationMode = operationMode;

  operationMode = handleWiFi(msgBuffer);

  bool reset = false;
  if (prevOperationMode != operationMode)
  {
    setScrollContent(true);
    reset = true;
  }

  switch (operationMode)
  {
  case OperationMode::MSG:
    if (reset)
    {
      PRINTS("Operation Mode MSG");
      setMessage(msgBuffer);
    }
    break;
  case OperationMode::CLK:
    if (reset)
    {
      PRINTS("Operation Mode CLK");
      setupClk();
    }
    if (getTime(timeBuffer))
    {
      PRINT("Time: ", timeBuffer);
      setClock(timeBuffer);
    }
    break;
  }

  scrollText();
}
