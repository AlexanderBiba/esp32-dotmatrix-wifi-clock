#include <WiFiServer.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>

#include "app_wifi.h"
#include "app_web_page.h"
#include "utils.h"

// WiFi Server object and parameters
WiFiServer server(80);

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

AppRequestMode extractHttpContent(char *szMesg, char requestBuffer[MAX_MSG_SIZE])
{
  AppRequestMode requestMode = AppRequestMode::NONE;

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
      requestMode = AppRequestMode::MSG;
      isValid = true;
    }
  }

  // handle clock mode
  pStart = strstr(szMesg, "/&CLK");

  if (pStart != NULL)
  {
    requestMode = AppRequestMode::CLK;
    isValid = true;
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
      requestMode = AppRequestMode::CNTL;
      isValid = true;
    }
  }

  // if (!isValid)
  // {
  //   throw new std::invalid_argument("Invalid message received");
  // }

  return requestMode;
}

AppRequestMode handleWiFi(char requestBuffer[MAX_MSG_SIZE])
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

  AppRequestMode appRequestMode = AppRequestMode::NONE;

  switch (state)
  {
  case S_IDLE: // initialize
    PRINTS("S_IDLE");
    idxBuf = 0;
    state = S_WAIT_CONN;
    break;

  case S_WAIT_CONN: // waiting for connection
  {
    client = server.accept();
    if (!client)
      break;
    if (!client.connected())
      break;

#if DEBUG
    char szTxt[20];
    sprintf(szTxt, "%d:%d:%d:%d", client.remoteIP()[0], client.remoteIP()[1], client.remoteIP()[2], client.remoteIP()[3]);
    PRINT("New client @ ", szTxt);
#endif

    timeStart = millis();
    state = S_READ;
  }
  break;

  case S_READ: // get the first line of data
    PRINTS("S_READ");
    while (client.available())
    {
      char c = client.read();
      if ((c == '\r') || (c == '\n'))
      {
        szBuf[idxBuf] = '\0';
        client.flush();
        PRINT("Recv: ", szBuf);
        state = S_EXTRACT;
      }
      else
        szBuf[idxBuf++] = (char)c;
    }
    if (millis() - timeStart > 1000)
    {
      PRINTS("Wait timeout");
      state = S_DISCONN;
    }
    break;

  case S_EXTRACT: // extract data
    PRINTS("S_EXTRACT");
    // Extract the string from the message if there is one
    appRequestMode = extractHttpContent(szBuf, requestBuffer);
    state = S_RESPONSE;
    break;

  case S_RESPONSE: // send the response to the client
    PRINTS("S_RESPONSE");
    // Return the response to the client (web page)
    client.print(WebResponse);
    client.print(WebPage);
    state = S_DISCONN;
    break;

  case S_DISCONN: // disconnect client
    PRINTS("S_DISCONN");
    client.flush();
    client.stop();
    state = S_IDLE;
    break;

  default:
    state = S_IDLE;
  }

  return appRequestMode;
}

const char *err2Str(wl_status_t code)
{
  switch (code)
  {
  case WL_IDLE_STATUS:
    return ("IDLE");
    break; // WiFi is in process of changing between statuses
  case WL_NO_SSID_AVAIL:
    return ("NO_SSID_AVAIL");
    break; // case configured SSID cannot be reached
  case WL_CONNECTED:
    return ("CONNECTED");
    break; // successful connection is established
  case WL_CONNECT_FAILED:
    return ("CONNECT_FAILED");
    break; // password is incorrect
  case WL_DISCONNECTED:
    return ("CONNECT_FAILED");
    break; // module is not configured in station mode
  default:
    return ("??");
  }
}

#define MDNS_DOMAIN "esp32"
void setupWiFi(char *localIp)
{
  WiFiManager wifiManager;
  // wifiManager.resetSettings();
  wifiManager.autoConnect("DotMatrix Clock");
  PRINTS("WIFI Connected");

  // Initialize mDNS
  bool mdnsSuccess = MDNS.begin(MDNS_DOMAIN);
  if (!mdnsSuccess)
  {
    PRINTS("Error setting up MDNS responder!");
  }

  // Start the server
  PRINTS("Starting Server");
  server.begin();

  if (mdnsSuccess)
  {
    sprintf(localIp, "%d:%d:%d:%d - %s.local", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3], MDNS_DOMAIN);
    PRINT("Assigned IP ", localIp);
    PRINT("MDNS ", MDNS_DOMAIN);
  }
  else
  {
    sprintf(localIp, "%d:%d:%d:%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    PRINT("Assigned IP ", localIp);
  }
}
