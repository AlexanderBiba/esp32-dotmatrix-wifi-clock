#include <WiFiServer.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>

#include "appserver.h"
#include "webpage.h"

#define MDNS_DOMAIN "digiclk"
AppServer::AppServer()
{
  server = new WiFiServer(80);

  WiFiManager wifiManager;
  wifiManager.autoConnect("DotMatrix Clock");
  Serial.println("WIFI Connected");

  bool mdnsSuccess = MDNS.begin(MDNS_DOMAIN);
  if (!mdnsSuccess)
  {
    Serial.println("Error setting up MDNS responder!");
  }

  Serial.println("Starting Server");
  server->begin();
}

uint8_t htoi(char c)
{
  c = toupper(c);
  if ((c >= '0') && (c <= '9'))
    return (c - '0');
  if ((c >= 'A') && (c <= 'F'))
    return (c - 'A' + 0xa);
  return (0);
}

void extractPayload(char *pStart, char *pEnd, char *buffer)
{
  char *psz = buffer;

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
}

AppServer::RequestMode extractHttpContent(char *szMesg, char requestBuffer[REQUEST_BUFFER_SIZE])
{
  AppServer::RequestMode requestMode = AppServer::RequestMode::NONE;

  bool isValid = false;
  char *pStart, *pEnd, *psz = requestBuffer;

  // handle message mode
  pStart = strstr(szMesg, "/&MSG=");

  if (pStart != NULL)
  {
    pStart += 6; // skip to start of data
    pEnd = strstr(pStart, "/&");

    if (pEnd != NULL)
    {
      extractPayload(pStart, pEnd, psz);
      requestMode = AppServer::RequestMode::MESSAGE;
      isValid = true;
    }
  }

  // handle clock mode
  pStart = strstr(szMesg, "/&CLK");

  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::CLOCK;
    isValid = true;
  }

  // handle weather mode
  pStart = strstr(szMesg, "/&WEATHER");

  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::WEATHER;
    isValid = true;
  }

  // handle stock mode
  pStart = strstr(szMesg, "/&TICKER");

  if (pStart != NULL)
  {
    pStart += 9; // skip to start of data
    pEnd = strstr(pStart, "/&");

    if (pEnd != NULL)
    {
      extractPayload(pStart, pEnd, psz);
      requestMode = AppServer::RequestMode::STOCK;
      isValid = true;
    }
  }

  // handle control mode
  pStart = strstr(szMesg, "/&CNTL");

  if (pStart != NULL)
  {
    pStart += 7; // skip to start of data
    pEnd = strstr(pStart, "/&");

    if (pEnd != NULL)
    {
      extractPayload(pStart, pEnd, psz);
      requestMode = AppServer::RequestMode::CNTL;
      isValid = true;
    }
  }

  return requestMode;
}

AppServer::RequestMode AppServer::handleWiFi(char requestBuffer[REQUEST_BUFFER_SIZE])
{
  static enum { S_IDLE,
                S_WAIT_CONN,
                S_READ,
                S_EXTRACT,
                S_RESPONSE,
                S_DISCONN } state = S_IDLE;
  static char szBuf[1024];
  static uint16_t idxBuf = 0;
  static WiFiClient client;
  static uint32_t timeStart;

  RequestMode appRequestMode = RequestMode::NONE;

  switch (state)
  {
  case S_IDLE: // initialize
    idxBuf = 0;
    state = S_WAIT_CONN;
    break;

  case S_WAIT_CONN: // waiting for connection
  {
    client = server->accept();
    if (!client)
      break;
    if (!client.connected())
      break;

    timeStart = millis();
    state = S_READ;
  }
  break;

  case S_READ: // get the first line of data
    while (client.available())
    {
      char c = client.read();
      if ((c == '\r') || (c == '\n'))
      {
        szBuf[idxBuf] = '\0';
        client.flush();
        state = S_EXTRACT;
      }
      else
        szBuf[idxBuf++] = (char)c;
    }
    if (millis() - timeStart > 1000)
    {
      state = S_DISCONN;
    }
    break;

  case S_EXTRACT: // extract data
    // Extract the string from the message if there is one
    appRequestMode = extractHttpContent(szBuf, requestBuffer);
    state = S_RESPONSE;
    break;

  case S_RESPONSE: // send the response to the client
    // Return the response to the client (web page)
    client.print("HTTP/1.1 200 OK\nContent-Type: text/html\n\n");
    client.print(WebPage);
    state = S_DISCONN;
    break;

  case S_DISCONN: // disconnect client
    client.flush();
    client.stop();
    state = S_IDLE;
    break;

  default:
    state = S_IDLE;
  }

  return appRequestMode;
}
