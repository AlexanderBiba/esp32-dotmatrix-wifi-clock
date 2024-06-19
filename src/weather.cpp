
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <HttpClient.h>

#include "weather.h"
#include "charmaps.h"
#include "main.h"
#include "renderer.h"

#define WEATHER_UPDATE_INTERVAL 10 * 60 * 1000

extern Renderer *renderer;

void Weather::loadBitmap(const char *weather)
{
  uint8_t *rawWeather = bitmap;
  *rawWeather++ = 0;
  *rawWeather++ = 0;
  rawWeather = renderer->loadStringToBitmap(weather, rawWeather);
  *rawWeather++ = 0;
  if (settings->getWeatherUnits() == 'c')
  {
    *rawWeather++ = 0xE0;
    *rawWeather++ = 0xA0;
    *rawWeather++ = 0xA0;
  }
  else if (settings->getWeatherUnits() == 'f')
  {
    *rawWeather++ = 0xF0;
    *rawWeather++ = 0x50;
    *rawWeather++ = 0x10;
  }
  renderer->alightBitmapContentToCenter(bitmap, rawWeather);
}

uint8_t *Weather::getWeather()
{
  static long lastRefresh = 0;

  if ((millis() - lastRefresh > WEATHER_UPDATE_INTERVAL) && updateWeatherData())
  {
    lastRefresh = millis();
    updateWeatherData();
  }

  return bitmap;
}

bool Weather::updateWeatherData()
{
  static char localBuffer[RAW_WEATHER_BITMAP_SIZE] = {0};

  printf("Updating weather data\n");
  printf("Latitude: %lf\n", settings->getLatitude());
  printf("Longitude: %lf\n", settings->getLongitude());
  if (
      !(
          -90 < settings->getLatitude() && settings->getLatitude() < 90 &&
          -180 < settings->getLongitude() && settings->getLongitude() < 180))
  {
    printf("Invalid location\n");
    renderer->alightBitmapContentToCenter(bitmap, renderer->loadStringToBitmap("No LOC", bitmap));
    printf("location done\n");
    return true;
  }

  WiFiClientSecure wifiClient;
  wifiClient.setInsecure();
  HttpClient client = HttpClient(wifiClient, "api.open-meteo.com", 443);
  char url[128] = {0};
  sprintf(url,
          "/v1/forecast?latitude=%lf&longitude=%lf&temperature_unit=%s&current=temperature",
          settings->getLatitude(),
          settings->getLongitude(),
          settings->getWeatherUnits() == 'c' ? "celsius" : "fahrenheit");
  client.get(url);
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  if (statusCode == 429)
  {
    Serial.println("Rate limited");
    return false;
  }

  printf("Status code: %d\n", statusCode);
  printf("Response: %s\n", response);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error)
  {
    printf("deserializeJson() failed: %s\n", error.f_str());
    return false;
  }

  weatherData = {doc["current"]["temperature"]};

  sprintf(localBuffer, "%.0f", weatherData.temp);
  loadBitmap(localBuffer);

  return true;
}
