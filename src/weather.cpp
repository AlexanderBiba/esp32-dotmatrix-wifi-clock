
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
  renderer->alignBitmapContentToCenter(bitmap, rawWeather);
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

  Serial.println("Updating weather data");
  Serial.printf("Latitude: %lf\n", settings->getLatitude());
  Serial.printf("Longitude: %lf\n", settings->getLongitude());
  if (
      !(
          -90 < settings->getLatitude() && settings->getLatitude() < 90 &&
          -180 < settings->getLongitude() && settings->getLongitude() < 180))
  {
    Serial.println("Invalid location");
    renderer->alignBitmapContentToCenter(bitmap, renderer->loadStringToBitmap("No LOC", bitmap));
    Serial.println("location done");
    return true;
  }

  WiFiClientSecure wifiClient;
  wifiClient.setInsecure();
  
  // Set timeout for the client
  wifiClient.setTimeout(10000); // 10 second timeout
  
  HttpClient client = HttpClient(wifiClient, "api.open-meteo.com", 443);
  char url[128] = {0};
  int urlLen = snprintf(url, sizeof(url),
          "/v1/forecast?latitude=%lf&longitude=%lf&temperature_unit=%s&current=temperature",
          settings->getLatitude(),
          settings->getLongitude(),
          settings->getWeatherUnits() == 'c' ? "celsius" : "fahrenheit");
  
  // Check if URL was truncated
  if (urlLen >= sizeof(url)) {
    Serial.println("URL too long, truncated");
    return false;
  }
  
  client.get(url);
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  if (statusCode == 429)
  {
    Serial.println("Rate limited");
    return false;
  }
  
  if (statusCode != 200) {
    Serial.printf("Weather API error: %d\n", statusCode);
    return false;
  }

  Serial.printf("Status code: %d\n", statusCode);
  Serial.printf("Response: %s\n", response);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error)
  {
    Serial.printf("deserializeJson() failed: %s\n", error.f_str());
    return false;
  }

  // Validate response structure
  if (!doc["current"] || !doc["current"]["temperature"]) {
    Serial.println("Invalid weather data structure");
    return false;
  }

  weatherData = {doc["current"]["temperature"]};

  sprintf(localBuffer, "%.0f", weatherData.temp);
  loadBitmap(localBuffer);

  return true;
}
