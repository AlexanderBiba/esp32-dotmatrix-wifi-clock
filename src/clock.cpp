#include <ArduinoJson.h>
#include <MD_MAX72xx.h>
#include <WiFi.h>
#include <HttpClient.h>

#include "clock.h"
#include "settings.h"
#include "charmaps.h"
#include "main.h"
#include "renderer.h"

#define UPDATE_TIME_INTERVAL 3600 * 1000

extern Renderer *renderer;

void Clock::updateTime()
{
  static WiFiClient wifiClient = WiFiClient();
  
  // Set timeout for the client
  wifiClient.setTimeout(10000); // 10 second timeout
  
  HttpClient client = HttpClient(wifiClient, "worldtimeapi.org");
  char url[128] = "/api/timezone/";
  
  // Check if timezone is valid
  if (strlen(settings->getTimezone()) == 0) {
    printf("No timezone set, using default\n");
    return;
  }
  
  // Check if adding timezone would exceed buffer size
  if (strlen(url) + strlen(settings->getTimezone()) >= sizeof(url)) {
    printf("URL too long with timezone\n");
    return;
  }
  
  strcat(url, settings->getTimezone());
  client.get(url);
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  printf("Status code: %d\n", statusCode);
  printf("Response: %s\n", response);

  if (statusCode != 200) {
    printf("Failed to get time data, status: %d\n", statusCode);
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error)
  {
    printf("deserializeJson() failed: %s\n", error.f_str());
    return;
  }

  // Validate the response data
  if (!doc.containsKey("unixtime") || !doc.containsKey("raw_offset")) {
    printf("Invalid time data received\n");
    return;
  }

  epoch = (long)doc["unixtime"] + (long)doc["raw_offset"] + (doc["dst"] ? (long)doc["dst_offset"] : 0) - (millis() / 1000);
  currentTime = epoch + (millis() / 1000);

  loadClockBitmap();
}

// getTimeStruct() method removed - was tied to buzzer functionality

void Clock::loadDateBitmap()
{
  static char timeBuffer[TIME_BUFFER_SIZE] = {0};
  uint8_t *p = dateBitmap;
  strftime(timeBuffer, TIME_BUFFER_SIZE, "%a", gmtime(&currentTime));
  p = renderer->loadStringToBitmap(timeBuffer, p);
  *p++ = 0;
  *p++ = 0;
#ifdef MMDD_DATE_FORMAT
  strftime(timeBuffer, TIME_BUFFER_SIZE, "%m", gmtime(&currentTime));
  p = renderer->loadStringToBitmap(timeBuffer, p, true);
  *p++ = 0x20;
  strftime(timeBuffer, TIME_BUFFER_SIZE, "%d", gmtime(&currentTime));
  p = renderer->loadStringToBitmap(timeBuffer, p, true);
#else
  strftime(timeBuffer, TIME_BUFFER_SIZE, "%d", gmtime(&currentTime));
  p = renderer->loadStringToBitmap(timeBuffer, p, true);
  *p++ = 0x10;
  strftime(timeBuffer, TIME_BUFFER_SIZE, "%m", gmtime(&currentTime));
  p = renderer->loadStringToBitmap(timeBuffer, p, true);
#endif
  renderer->alightBitmapContentToCenter(dateBitmap, p);
}

void Clock::loadClockBitmap()
{
  static char timeBuffer[TIME_BUFFER_SIZE] = {0};
  strftime(timeBuffer, TIME_BUFFER_SIZE, "%T", gmtime(&currentTime));
  uint8_t *p = clockBitmap;

#if SMALL_SECONDS_CLOCK
  p += renderer->writeCharToBuffer(timeBuffer[0], p);
  p += renderer->writeCharToBuffer(timeBuffer[1], p);
  *p++ = 0; // empty column between hours and minutes
  *p++ = 0; // empty column between hours and minutes
  p += renderer->writeCharToBuffer(timeBuffer[3], p);
  p += renderer->writeCharToBuffer(timeBuffer[4], p);
  *p++ = 0; // empty column between minutes and seconds
  *p++ = 0; // empty column between minutes and seconds
  p += renderer->writeSmallCharToBuffer(timeBuffer[6], p);
  *p++ = 0; // empty column
  p += renderer->writeSmallCharToBuffer(timeBuffer[7], p);
#else
  p += renderer->writeCharToBuffer(timeBuffer[0], p);
  p += renderer->writeCharToBuffer(timeBuffer[1], p);
  *p++ = 0; // empty column between hours and minutes
  p += renderer->writeCharToBuffer(timeBuffer[3], p);
  p += renderer->writeCharToBuffer(timeBuffer[4], p);
  *p++ = 0; // empty column between minutes and seconds
  p += renderer->writeCharToBuffer(timeBuffer[6], p);
  p += renderer->writeCharToBuffer(timeBuffer[7], p);
#endif
}

uint8_t *Clock::getTime()
{
  static long prevUpdate = 0;
  if ((millis() - prevUpdate > UPDATE_TIME_INTERVAL) && WiFi.status() == WL_CONNECTED)
  {
    prevUpdate = millis();
    updateTime();
  }

  long newTime = epoch + (millis() / 1000);
  if (newTime != currentTime)
  {
    currentTime = newTime;
    loadClockBitmap();
  }

  return clockBitmap;
}

uint8_t *Clock::getDate()
{
  static long prevUpdate = 0;
  if ((millis() - prevUpdate > UPDATE_TIME_INTERVAL) && WiFi.status() == WL_CONNECTED)
  {
    prevUpdate = millis();
    updateTime();
  }

  long newTime = epoch + (millis() / 1000);
  if (newTime != currentTime)
  {
    currentTime = newTime;
    loadDateBitmap();
  }

  return dateBitmap;
}
