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

void loop(void)
{

  OpearationMode prevOperationMode = operationMode;

  handleWiFi(getText);
  switch (operationMode)
  {
  case MSG:
    scrollText();
    break;
  case CLK:
    scrollText();
    if (printTime(prevOperationMode != operationMode, buffer))
    {
      setMessage(buffer);
    }
    break;
  }
}
