#include <ArduinoJson.h>
#include <MD_MAX72xx.h>
#include <WiFi.h>
#include <HttpClient.h>

#include "clk.h"
#include "utils.h"
#include "app_settings.h"

long epoch = 0;
char timezone[128] = "America/New_York";

void setupClk(AppSettings *settings)
{
  PRINTS("getting time from server");

  if (settings && settings->timezone[0] != '\0')
  {
    strcpy(timezone, settings->timezone);
  }

  static WiFiClient wifiClient = WiFiClient();
  HttpClient client = HttpClient(wifiClient, "worldtimeapi.org");
  char url[128] = "/api/timezone/";
  strcat(url, timezone);
  PRINT("url: ", url);
  client.get(url);
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  PRINT("Status code: ", statusCode);
  PRINT("Response: ", response);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error)
  {
    PRINT("deserializeJson() failed: ", error.f_str());
    return;
  }

  epoch = (long)doc["unixtime"] + (long)doc["raw_offset"] + (doc["dst"] ? (long)doc["dst_offset"] : 0) - (millis() / 1000);
}

bool getTime(char timeBuffer[TIME_BUFFER_SIZE])
{
  static long time = epoch + (millis() / 1000);

  long newTime = epoch + (millis() / 1000);
  if (newTime != time)
  {
    time = newTime;
    if (WiFi.status() == WL_CONNECTED && (time % 3600) == 0) // sync time every hour
    {
      setupClk();
    }

    strftime(timeBuffer, TIME_BUFFER_SIZE, "%T", gmtime(&time));
    return true;
  }

  return false;
}

void setTimezone(const char *newTimezone)
{
  strcpy(timezone, newTimezone);
  setupClk();
}