// Use the MD_MAX72XX library to scroll text on the display
// received through the ESP32 WiFi interface.
//
// IP address for the ESP32 is displayed on the scrolling display
// after startup initialization and connected to the WiFi network.
//

#include <ctime>
#include <cstring>
#include <ctype.h>
#include <Arduino.h>

#include "clk.h"
#include "utils.h"
#include "renderer.h"
#include "app_matter.h"

#define PRINT_CALLBACK 0
#define DEBUG 1
#define LED_HEARTBEAT 0

enum OpearationMode
{
  MSG,
  CLK
};

OpearationMode operationMode = CLK;

void setup(void)
{
  Serial.begin(115200);

  setupRenderer();
  setupMatter();
  // setupClk();
}

void writeChar(char c, uint8_t *buffer)
{
  uint8_t c1[5];
  uint8_t len = getChar(c, 5, c1);
  for (uint8_t i = 0; i < len; i++)
  {
    buffer[i] = c1[i];
  }
  if (len < 5)
  {
    for (uint8_t i = len; i < 5; i++)
    {
      buffer[i] = '\0';
    }
  }
}

void loop(void)
{
  static char buffer[MAX_MSG_SIZE] = "\0";
  static uint8_t rawMessageBuffer[MAX_DEVICES * 8];

  OpearationMode prevOperationMode = operationMode;

  operationMode = getMatterSwitchVal() ? MSG : CLK;

  if (prevOperationMode != operationMode)
  {
    PRINT("Operation Mode: ", operationMode);
    setStopScroll(false);
    switch (operationMode)
    {
    case MSG:
      setMessage("Hello World!");
      break;
    }
  }

  switch (operationMode)
  {
  case MSG:
    scrollText();
    break;
  case CLK:
    if (getTime(prevOperationMode != operationMode, buffer))
    {
      uint8_t *p = rawMessageBuffer;
      writeChar(buffer[0], p);
      p += 5;
      writeChar(buffer[1], p);
      p += 5;
      *p++ = 0;
      writeChar(buffer[3], p);
      p += 5;
      writeChar(buffer[4], p);
      p += 5;
      *p++ = 0;
      writeChar(buffer[6], p);
      p += 5;
      writeChar(buffer[7], p);
    }
    if (getStopScroll())
    {
      renderRaw(rawMessageBuffer);
    }
    else
    {
      setRawMessage(rawMessageBuffer);
      scrollText();
    }
    break;
  }
}
