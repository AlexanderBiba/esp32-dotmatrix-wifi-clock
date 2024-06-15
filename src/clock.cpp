#include <ArduinoJson.h>
#include <MD_MAX72xx.h>
#include <WiFi.h>
#include <HttpClient.h>

#include "clock.h"
#include "settings.h"
#include "charmaps.h"
#include "main.h"

#define UPDATE_TIME_INTERVAL 3600 * 1000

void Clock::updateTime()
{
  static WiFiClient wifiClient = WiFiClient();
  HttpClient client = HttpClient(wifiClient, "worldtimeapi.org");
  char url[128] = "/api/timezone/";
  strcat(url, settings->getTimezone());
  client.get(url);
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  printf("Status code: %d\n", statusCode);
  printf("Response: %s\n", response);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error)
  {
    printf("deserializeJson() failed: %s\n", error.f_str());
    return;
  }

  epoch = (long)doc["unixtime"] + (long)doc["raw_offset"] + (doc["dst"] ? (long)doc["dst_offset"] : 0) - (millis() / 1000);
  currentTime = epoch + (millis() / 1000);

  loadBitmap();
}

int writeCharToBuffer(char c, uint8_t *buffer)
{
  uint8_t c1[CLOCK_DIGIT_WIDTH];
  for (uint8_t i = 0; i < CLOCK_DIGIT_WIDTH; i++)
  {
#if BOTTOM_ALIGN
    buffer[i] = digitCharMap[c - '0'][i] * 2;
#else
    buffer[i] = digitCharMap[c - '0'][i];
#endif
  }
  return CLOCK_DIGIT_WIDTH;
}

int writeSmallCharToBuffer(char c, uint8_t *buffer)
{
  for (uint8_t i = 0; i < SMALL_DIGIT_LEN; i++)
  {
#if BOTTOM_ALIGN
    buffer[i] = smallDigitCharMap[c - '0'][i] * 8;
#else
    buffer[i] = smallDigitCharMap[c - '0'][i];
#endif
  }
  return SMALL_DIGIT_LEN;
}

void Clock::loadBitmap()
{
  strftime(clockBuffer, TIME_BUFFER_SIZE, "%T", gmtime(&currentTime));
  uint8_t *p = bitmap;

#if SMALL_SECONDS_CLOCK
  p += writeCharToBuffer(clockBuffer[0], p);
  p += writeCharToBuffer(clockBuffer[1], p);
  *p++ = 0; // empty column between hours and minutes
  *p++ = 0; // empty column between hours and minutes
  p += writeCharToBuffer(clockBuffer[3], p);
  p += writeCharToBuffer(clockBuffer[4], p);
  *p++ = 0; // empty column between minutes and seconds
  *p++ = 0; // empty column between minutes and seconds
  p += writeSmallCharToBuffer(clockBuffer[6], p);
  *p++ = 0; // empty column
  p += writeSmallCharToBuffer(clockBuffer[7], p);
#else
  p += writeCharToBuffer(clockBuffer[0], p);
  p += writeCharToBuffer(clockBuffer[1], p);
  *p++ = 0; // empty column between hours and minutes
  p += writeCharToBuffer(clockBuffer[3], p);
  p += writeCharToBuffer(clockBuffer[4], p);
  *p++ = 0; // empty column between minutes and seconds
  p += writeCharToBuffer(clockBuffer[6], p);
  p += writeCharToBuffer(clockBuffer[7], p);
#endif
}

const uint8_t *Clock::getTime()
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
    loadBitmap();
  }

  return bitmap;
}
