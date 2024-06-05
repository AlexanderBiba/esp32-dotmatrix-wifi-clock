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

bool getTime(char timeBuffer[TIME_BUFFER_SIZE])
{
  static long time = epoch + (millis() / 1000);

  long newTime = epoch + (millis() / 1000);
  if (newTime != time)
  {
    time = newTime;
    if (WiFi.status() == WL_CONNECTED && (newTime % 3600) == 0) // sync time every hour
    {
      setupClk();
    }

    time_t nyTime = myTZ.toLocal(newTime);
    strftime(timeBuffer, TIME_BUFFER_SIZE, "%T", gmtime(&nyTime));
    return true;
  }

  return false;
}
