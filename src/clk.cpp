#include <ArduinoJson.h>
#include <Timezone.h>
#include <MD_MAX72xx.h>
#include <WiFi.h>
#include <HttpClient.h>

#include "clk.h"
#include "utils.h"

long epoch = 0;

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule myDST = {"EDT", Second, Sun, Mar, 2, -240}; // Daylight time = UTC - 4 hours
TimeChangeRule mySTD = {"EST", First, Sun, Nov, 2, -300};  // Standard time = UTC - 5 hours
Timezone myTZ(myDST, mySTD);

void setupClk()
{
  PRINTS("getting time from server");

  static WiFiClient wifiClient = WiFiClient();
  HttpClient client = HttpClient(wifiClient, "worldtimeapi.org");
  client.get("/api/timezone/America/New_York");
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

  long unixtime = doc["unixtime"];
  PRINT("Unix time: ", unixtime);
  epoch = unixtime - (millis() / 1000);
}

bool getTime(bool reset, char *buffer)
{
  bool newValue = false;

  static enum { S_IDLE,
                S_RESET } state;
  static long time = epoch + (millis() / 1000);

  if (reset)
  {
    state = S_RESET;
  }

  switch (state)
  {
  case S_RESET:
    PRINTS("S_RESET");
    if (WiFi.status() == WL_CONNECTED && epoch == 0)
    {
      setupClk();
    }
    state = S_IDLE;
    break;
  case S_IDLE:
    const uint8_t charSize = 5;

    long newTime = epoch + (millis() / 1000);
    if (newTime != time)
    {
      time = newTime;
      if (WiFi.status() == WL_CONNECTED && (newTime % 3600) == 0) // sync time every hour
      {
        setupClk();
      }

      char timeStr[sizeof("hh:mm:ss")];
      time_t nyTime = myTZ.toLocal(newTime);
      strftime((timeStr), sizeof(timeStr), "%T", gmtime(&nyTime));
      memcpy(buffer, timeStr, sizeof(timeStr));
      newValue = true;
    }
    break;
  }
  return newValue;
}
