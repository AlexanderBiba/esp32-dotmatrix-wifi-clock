#include <ArduinoJson.h>
#include <MD_MAX72xx.h>
#include <WiFi.h>
#include <HttpClient.h>

#include "clock.h"
#include "utils.h"
#include "settings.h"

void Clock::updateTime()
{
  static WiFiClient wifiClient = WiFiClient();
  HttpClient client = HttpClient(wifiClient, "worldtimeapi.org");
  char url[128] = "/api/timezone/";
  strcat(url, settings->getTimezone());
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

#define CLOCK_DIGIT_WIDTH 5
const uint8_t digitCharMap[10][CLOCK_DIGIT_WIDTH] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x04, 0x02, 0x7F, 0x00}, // 1
    {0x71, 0x49, 0x49, 0x49, 0x46}, // 2
    {0x41, 0x49, 0x49, 0x49, 0x36}, // 3
    {0x0F, 0x08, 0x08, 0x08, 0x7F}, // 4
    {0x4F, 0x49, 0x49, 0x49, 0x31}, // 5
    {0x3E, 0x49, 0x49, 0x49, 0x30}, // 6
    {0x03, 0x01, 0x01, 0x01, 0x7F}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x49, 0x3E}, // 9
};

int writeCharToBuffer(char c, uint8_t *buffer)
{
  uint8_t c1[CLOCK_DIGIT_WIDTH];

  for (uint8_t i = 0; i < CLOCK_DIGIT_WIDTH; i++)
  {
#if BOTTOM_ALIGN_CLOCK
    buffer[i] = digitCharMap[c - '0'][i] * 2;
#else
    buffer[i] = digitCharMap[c - '0'][i];
#endif
  }
  return CLOCK_DIGIT_WIDTH;
}

#define SMALL_DIGIT_LEN 3
const uint8_t smallDigitCharMap[10][SMALL_DIGIT_LEN] = {
    {0x1F, 0x11, 0x1F}, // 0
    {0x00, 0x00, 0x1F}, // 1
    {0x1D, 0x15, 0x17}, // 2
    {0x15, 0x15, 0x1F}, // 3
    {0x07, 0x04, 0x1F}, // 4
    {0x17, 0x15, 0x1D}, // 5
    {0x1F, 0x15, 0x1D}, // 6
    {0x01, 0x01, 0x1F}, // 7
    {0x1F, 0x15, 0x1F}, // 8
    {0x17, 0x15, 0x1F}, // 9
};

int writeSmallCharToBuffer(char c, uint8_t *buffer)
{
  for (uint8_t i = 0; i < SMALL_DIGIT_LEN; i++)
  {
#if BOTTOM_ALIGN_CLOCK
    buffer[i] = smallDigitCharMap[c - '0'][i] * 8;
#else
    buffer[i] = smallDigitCharMap[c - '0'][i];
#endif
  }
  return SMALL_DIGIT_LEN;
}
const uint8_t *Clock::getTime()
{
  static long time = epoch + (millis() / 1000);

  long newTime = epoch + (millis() / 1000);
  if (newTime != time)
  {
    time = newTime;
    if (WiFi.status() == WL_CONNECTED && (time % 3600) == 0)
    {
      updateTime();
    }

    strftime(clockBuffer, TIME_BUFFER_SIZE, "%T", gmtime(&time));

    uint8_t *p = rawClockBuffer;

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

    return rawClockBuffer;
  }

  return nullptr;
}
