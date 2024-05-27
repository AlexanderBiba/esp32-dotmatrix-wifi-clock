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
#include "app_wifi.h"
#include "utils.h"
#include "renderer.h"

#define PRINT_CALLBACK 0
#define DEBUG 1
#define LED_HEARTBEAT 0

enum OpearationMode
{
  MSG,
  CLK
};

OpearationMode operationMode = MSG;

uint8_t htoi(char c)
{
  c = toupper(c);
  if ((c >= '0') && (c <= '9'))
    return (c - '0');
  if ((c >= 'A') && (c <= 'F'))
    return (c - 'A' + 0xa);
  return (0);
}

void getText(char *szMesg, uint8_t len)
{
  static char buffer[MAX_MSG_SIZE];

  bool isValid = false;
  char *pStart, *pEnd, *psz = buffer;

  // handle message mode
  pStart = strstr(szMesg, "/&MSG=");

  if (pStart != NULL)
  {
    pStart += 6; // skip to start of data
    pEnd = strstr(pStart, "/&");

    if (pEnd != NULL)
    {
      while (pStart != pEnd)
      {
        if ((*pStart == '%') && isxdigit(*(pStart + 1)))
        {
          // replace %xx hex code with the ASCII character
          char c = 0;
          pStart++;
          c += (htoi(*pStart++) << 4);
          c += htoi(*pStart++);
          *psz++ = c;
        }
        else
          *psz++ = *pStart++;
      }

      *psz = '\0'; // terminate the string
      operationMode = MSG;
      isValid = true;
    }
  }

  // handle clock mode
  pStart = strstr(szMesg, "/&CLK");

  if (pStart != NULL)
  {
    operationMode = CLK;
    isValid = true;
  }

  if (isValid)
  {
    setMessage(buffer);
  }
}

char buffer[MAX_MSG_SIZE];
uint8_t rawMessageBuffer[MAX_DEVICES * 8];

void setup(void)
{
#if DEBUG
  Serial.begin(115200);
  PRINTS("\n[MD_MAX72XX WiFi Message Display]\nType a message for the scrolling display from your internet browser");
#endif

  buffer[0] = '\0';

  setupRenderer();

  setupWiFi(buffer);
  setMessage(buffer);

  setupClk();
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
  OpearationMode prevOperationMode = operationMode;

  handleWiFi(getText);

  if (prevOperationMode != operationMode)
  {
    PRINT("\nOperation Mode: ", operationMode);
    setStopScroll(false);
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
