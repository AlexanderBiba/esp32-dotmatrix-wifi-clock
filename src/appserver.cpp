#include <WiFiServer.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

#include "appserver.h"
#include "webpage.h"

#define DEFAULT_MDNS_DOMAIN "digiclk"

AppServer::AppServer(AppSettings *settings) : settings(settings)
{
  server = new WiFiServer(80);

  // Configure WiFiManager for non-blocking operation
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setBreakAfterConfig(true);

  // Set up WiFi connection callback to reboot after configuration
  wifiManager.setSaveConfigCallback([]() {
    Serial.println("WiFi configuration saved - rebooting device");
    delay(2000); // Give time for the response to be sent
    ESP.restart();
  });

  // Try to connect to WiFi, or start config portal if no credentials
  if (!wifiManager.autoConnect("DotMatrix Clock")) {
    Serial.println("Config portal running - connect to 'DotMatrix Clock' AP");
  } else {
    startMDNS();
  }

  server->begin();
}

void AppServer::startMDNS()
{
  const char* mdnsDomain = settings->getMdnsDomain();
  if (strlen(mdnsDomain) == 0) {
    mdnsDomain = DEFAULT_MDNS_DOMAIN;
  }
  
  if (!MDNS.begin(mdnsDomain)) {
    MDNS.begin(DEFAULT_MDNS_DOMAIN);
  }
}

void AppServer::processWiFi()
{
  // Process WiFiManager (keeps captive portal alive)
  wifiManager.process();
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
  size_t bufferSize = REQUEST_BUFFER_SIZE;
  size_t remainingSpace = bufferSize;

  while (pStart != pEnd && remainingSpace > 1)
  {
    if ((*pStart == '%') && isxdigit(*(pStart + 1)) && remainingSpace > 2)
    {
      // replace %xx hex code with the ASCII character
      char c = 0;
      pStart++;
      c += (htoi(*pStart++) << 4);
      c += htoi(*pStart++);
      *psz++ = c;
      remainingSpace--;
    }
    else if (remainingSpace > 1)
    {
      *psz++ = *pStart++;
      remainingSpace--;
    }
    else
    {
      break; // Buffer full, stop processing
    }
  }

  *psz = '\0'; // terminate the string
}

const char* formatUptime(unsigned long uptime) {
  static char uptimeString[32];
  unsigned long seconds = uptime / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  
  if (days > 0) {
    snprintf(uptimeString, sizeof(uptimeString), "%ldd %ldh %ldm", days, hours % 24, minutes % 60);
  } else if (hours > 0) {
    snprintf(uptimeString, sizeof(uptimeString), "%ldh %ldm %lds", hours, minutes % 60, seconds % 60);
  } else if (minutes > 0) {
    snprintf(uptimeString, sizeof(uptimeString), "%ldm %lds", minutes, seconds % 60);
  } else {
    snprintf(uptimeString, sizeof(uptimeString), "%lds", seconds);
  }
  
  return uptimeString;
}

AppServer::RequestMode extractHttpContent(char *szMesg, char requestBuffer[REQUEST_BUFFER_SIZE], boolean activeCards[OPERATION_MODE_LENGTH])
{
  AppServer::RequestMode requestMode = AppServer::RequestMode::NONE;
  boolean _activeCards[OPERATION_MODE_LENGTH] = {0};

  char *pStart, *pEnd, *psz = requestBuffer;

  // STOP request handling removed - was tied to buzzer functionality

  // handle get settings
  pStart = strstr(szMesg, "/&SETT");

  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::SETT;
  }

  // handle get system info
  pStart = strstr(szMesg, "/&SYSINFO");

  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::SYSINFO;
  }

  // handle message mode
  pStart = strstr(szMesg, "/&MSG=");

  if (pStart != NULL)
  {
    pStart += 6; // skip to start of data
    pEnd = strstr(pStart, "/&");

    if (pEnd != NULL)
    {
      extractPayload(pStart, pEnd, psz);
      requestMode = AppServer::RequestMode::MODE;
      _activeCards[static_cast<int>(OperationMode::MESSAGE)] = true;
    }
  }

  // handle countdown mode
  pStart = strstr(szMesg, "/&COUNTDOWN=");

  if (pStart != NULL)
  {
    pStart += 12; // skip to start of data
    pEnd = strstr(pStart, "/&");

    if (pEnd != NULL)
    {
      extractPayload(pStart, pEnd, psz);
      requestMode = AppServer::RequestMode::MODE;
      _activeCards[static_cast<int>(OperationMode::COUNTDOWN)] = true;
    }
  }

  // handle clock mode
  pStart = strstr(szMesg, "/&CLOCK");

  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::MODE;
    _activeCards[static_cast<int>(OperationMode::CLOCK)] = true;
  }

  // handle date mode
  pStart = strstr(szMesg, "/&DATE");

  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::MODE;
    _activeCards[static_cast<int>(OperationMode::DATE)] = true;
  }

  // handle weather mode
  pStart = strstr(szMesg, "/&WEATHER");

  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::MODE;
    _activeCards[static_cast<int>(OperationMode::WEATHER)] = true;
  }

  // handle snake mode
  pStart = strstr(szMesg, "/&SNAKE");

  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::MODE;
    _activeCards[static_cast<int>(OperationMode::SNAKE)] = true;
  }

  // handle rain mode
  pStart = strstr(szMesg, "/&RAIN");

  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::MODE;
    _activeCards[static_cast<int>(OperationMode::RAIN)] = true;
  }

  // handle ip address mode
  pStart = strstr(szMesg, "/&IP");

  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::MODE;
    _activeCards[static_cast<int>(OperationMode::IP_ADDRESS)] = true;
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
    }
    else
    {
      // If no /& found, extract to end of string
      extractPayload(pStart, pStart + strlen(pStart), psz);
      requestMode = AppServer::RequestMode::CNTL;
    }
  }

  // handle reboot request
  pStart = strstr(szMesg, "/&REBOOT");
  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::REBOOT;
  }


  // handle factory reset request
  pStart = strstr(szMesg, "/&FACTORY_RESET");
  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::FACTORY_RESET;
  }

  // handle card order request
  pStart = strstr(szMesg, "/&CARD_ORDER");
  if (pStart != NULL)
  {
    pStart += 12; // skip to start of data
    // Skip the '=' character if present
    if (*pStart == '=')
    {
      pStart++;
    }
    pEnd = strstr(pStart, "/&");

    if (pEnd != NULL)
    {
      extractPayload(pStart, pEnd, psz);
      requestMode = AppServer::RequestMode::CARD_ORDER;
    }
    else
    {
      // If no /& found, extract to end of string
      extractPayload(pStart, pStart + strlen(pStart), psz);
      requestMode = AppServer::RequestMode::CARD_ORDER;
    }
  }

  // handle card durations request
  pStart = strstr(szMesg, "/&CARD_DURATIONS");
  if (pStart != NULL)
  {
    pStart += 16; // skip to start of data
    // Skip the '=' character if present
    if (*pStart == '=')
    {
      pStart++;
    }
    pEnd = strstr(pStart, "/&");

    if (pEnd != NULL)
    {
      extractPayload(pStart, pEnd, psz);
      requestMode = AppServer::RequestMode::CARD_DURATIONS;
    }
    else
    {
      // If no /& found, extract to end of string
      extractPayload(pStart, pStart + strlen(pStart), psz);
      requestMode = AppServer::RequestMode::CARD_DURATIONS;
    }
  }

  // handle flip display request
  pStart = strstr(szMesg, "/&FLIP_DISPLAY");
  if (pStart != NULL)
  {
    requestMode = AppServer::RequestMode::FLIP_DISPLAY;
  }

  if (requestMode == AppServer::RequestMode::MODE)
  {
    for (int i = 0; i < OPERATION_MODE_LENGTH; i++)
    {
      activeCards[i] = _activeCards[i];
    }
  }

  return requestMode;
}

AppServer::RequestMode AppServer::handleWiFi(char requestBuffer[REQUEST_BUFFER_SIZE], boolean activeCards[OPERATION_MODE_LENGTH])
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
  static char responseBuffer[1024];
  static const char *responseHeader;
  static const char *response = WebPage;

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
        break;
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
    appRequestMode = extractHttpContent(szBuf, requestBuffer, activeCards);
    if (appRequestMode == RequestMode::NONE)
    {
      responseHeader = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
      response = WebPage;
    }
    else if (appRequestMode == RequestMode::SETT)
    {
      JsonDocument doc;
      settings->toJson(doc);
      serializeJson(doc, responseBuffer);
      responseHeader = "HTTP/1.1 200 OK\nContent-Type: application/json\n\n";
      response = responseBuffer;
    }
    else if (appRequestMode == RequestMode::SYSINFO)
    {
      JsonDocument doc;
      
      // WiFi info
      doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
      doc["ip_address"] = WiFi.localIP().toString();
      doc["wifi_rssi"] = WiFi.RSSI();
      doc["wifi_ssid"] = WiFi.SSID();
      
      // System info
      doc["uptime_seconds"] = millis() / 1000;
      doc["uptime_formatted"] = formatUptime(millis());
      
      // Memory info
      doc["free_heap"] = ESP.getFreeHeap();
      doc["total_heap"] = ESP.getHeapSize();
      doc["min_free_heap"] = ESP.getMinFreeHeap();
      doc["max_alloc_heap"] = ESP.getMaxAllocHeap();
      
      // Storage info
      doc["free_psram"] = ESP.getFreePsram();
      doc["flash_size"] = ESP.getFlashChipSize();
      
      // Chip info
      doc["chip_model"] = ESP.getChipModel();
      doc["chip_revision"] = ESP.getChipRevision();
      doc["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
      doc["sdk_version"] = ESP.getSdkVersion();
      
      serializeJson(doc, responseBuffer);
      responseHeader = "HTTP/1.1 200 OK\nContent-Type: application/json\n\n";
      response = responseBuffer;
    }
    else if (appRequestMode == RequestMode::REBOOT)
    {
      responseHeader = "HTTP/1.1 200 OK\nContent-Type: application/json\n\n";
      response = "{\"status\":\"rebooting\"}";
      
      // Schedule reboot after sending response
      delay(1000); // Give time for response to be sent
      ESP.restart();
    }
    else if (appRequestMode == RequestMode::FACTORY_RESET)
    {
      responseHeader = "HTTP/1.1 200 OK\nContent-Type: application/json\n\n";
      response = "{\"status\":\"factory_reset_complete\"}";
      
      // Complete factory reset with WiFi clearing
      Serial.println("Starting complete factory reset...");
      
      // Clear WiFi credentials first
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      esp_wifi_restore();
      nvs_flash_erase();
      
      // Reset settings
      settings->factoryReset();
      
      Serial.println("Factory reset completed - rebooting...");
      
      // Schedule reboot after sending response
      delay(1000);
      ESP.restart();
    }
    else if (appRequestMode == RequestMode::CARD_ORDER)
    {
      responseHeader = "HTTP/1.1 200 OK\nContent-Type: application/json\n\n";
      response = "{\"status\":\"card_order_updated\"}";
      
      // Parse the card order from JSON
      Serial.printf("Card order request buffer: %s\n", requestBuffer);
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, requestBuffer);
      if (error)
      {
        Serial.printf("Card order deserializeJson() failed: %s\n", error.f_str());
        response = "{\"status\":\"error\",\"message\":\"Invalid JSON\"}";
      }
      else
      {
        // Extract card order array
        if (doc.containsKey("cardOrder") && doc["cardOrder"].is<JsonArray>())
        {
          uint8_t newCardOrder[OPERATION_MODE_LENGTH];
          JsonArray orderArray = doc["cardOrder"];
          
          // Validate array size
          if (orderArray.size() == OPERATION_MODE_LENGTH)
          {
            for (int i = 0; i < OPERATION_MODE_LENGTH; i++)
            {
              newCardOrder[i] = orderArray[i].as<uint8_t>();
            }
            
            // Validate that all indices are present and unique
            bool validOrder = true;
            // Serial.printf("Received card order: ");
            // for (int i = 0; i < OPERATION_MODE_LENGTH; i++)
            // {
            //   Serial.printf("%d ", newCardOrder[i]);
            // }
            // Serial.println();
            
            for (int i = 0; i < OPERATION_MODE_LENGTH; i++)
            {
              bool found = false;
              for (int j = 0; j < OPERATION_MODE_LENGTH; j++)
              {
                if (newCardOrder[j] == i)
                {
                  found = true;
                  break;
                }
              }
              if (!found)
              {
                Serial.printf("Missing index %d in card order\n", i);
                validOrder = false;
                break;
              }
            }
            
            if (validOrder)
            {
              settings->setCardOrder(newCardOrder);
              Serial.println("Card order updated successfully");
            }
            else
            {
              Serial.println("Invalid card order - not all indices present");
              response = "{\"status\":\"error\",\"message\":\"Invalid card order\"}";
            }
          }
          else
          {
            Serial.println("Invalid card order array size");
            response = "{\"status\":\"error\",\"message\":\"Invalid array size\"}";
          }
        }
        else
        {
          Serial.println("Missing or invalid cardOrder field");
          response = "{\"status\":\"error\",\"message\":\"Missing cardOrder field\"}";
        }
      }
    }
    else if (appRequestMode == RequestMode::CARD_DURATIONS)
    {
      responseHeader = "HTTP/1.1 200 OK\nContent-Type: application/json\n\n";
      response = "{\"status\":\"card_durations_updated\"}";
      
      // Parse the card durations from JSON
      Serial.printf("Card durations request buffer: %s\n", requestBuffer);
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, requestBuffer);
      if (error)
      {
        Serial.printf("Card durations deserializeJson() failed: %s\n", error.f_str());
        response = "{\"status\":\"error\",\"message\":\"Invalid JSON\"}";
      }
      else
      {
        // Extract card durations array
        if (doc.containsKey("cardDurations") && doc["cardDurations"].is<JsonArray>())
        {
          uint16_t newCardDurations[OPERATION_MODE_LENGTH];
          JsonArray durationsArray = doc["cardDurations"];
          
          // Validate array size
          if (durationsArray.size() == OPERATION_MODE_LENGTH)
          {
            for (int i = 0; i < OPERATION_MODE_LENGTH; i++)
            {
              newCardDurations[i] = durationsArray[i].as<uint16_t>();
            }
            
            settings->setCardDurations(newCardDurations);
            Serial.println("Card durations updated successfully");
          }
          else
          {
            Serial.println("Invalid card durations array size");
            response = "{\"status\":\"error\",\"message\":\"Invalid array size\"}";
          }
        }
        else
        {
          Serial.println("Missing or invalid cardDurations field");
          response = "{\"status\":\"error\",\"message\":\"Missing cardDurations field\"}";
        }
      }
    }
    else if (appRequestMode == RequestMode::FLIP_DISPLAY)
    {
      responseHeader = "HTTP/1.1 200 OK\nContent-Type: application/json\n\n";
      response = "{\"status\":\"display_flipped\"}";
      
      // Toggle the flipped setting
      bool currentFlipped = settings->getFlipped();
      settings->setFlipped(!currentFlipped);
      
      Serial.printf("Display orientation flipped to: %s\n", !currentFlipped ? "flipped" : "normal");
      
      // Schedule reboot after sending response
      delay(1000); // Give time for response to be sent
      ESP.restart();
    }
    state = S_RESPONSE;
    break;

  case S_RESPONSE: // send the response to the client
    // Return the response to the client (web page)
    client.print(responseHeader);
    client.print(response);
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
