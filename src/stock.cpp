#include <ArduinoJson.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <WiFiClientSecure.h>

#include "stock.h"

bool Stock::updateStockData()
{
  WiFiClientSecure wifiClient;
  wifiClient.setInsecure();
  HttpClient client = HttpClient(wifiClient, "api.finazon.io", 443);
  char url[128] = "/latest/time_series?dataset=us_stocks_essential&interval=1m&page=0&page_size=1&adjust=all&ticker=";
  strcat(url, ticker);
  char authHeader[128] = {0};
  strcat(authHeader, "apikey ");
  strcat(authHeader, settings->getStockApiKey());
  client.beginRequest();
  client.get(url);
  client.sendHeader("Authorization", authHeader);
  client.endRequest();
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

  stockData = {
      ticker,
      doc["data"][0]["t"],
      doc["data"][0]["o"],
      doc["data"][0]["c"],
      doc["data"][0]["h"],
      doc["data"][0]["l"],
      doc["data"][0]["v"]};

  return true;
}

const char *Stock::getQuote()
{
  static long lastRefresh = 0;
  if (strlen(settings->getStockApiKey()) == 0 || strlen(ticker) == 0)
  {
    strcpy(quoteBuffer, "No API key or ticker set");
    return quoteBuffer;
  }
  if (millis() - lastRefresh > 30000 && updateStockData())
  {
    lastRefresh = millis();
    sprintf(quoteBuffer, "%s: H:%.2f L:%.2f O:%.2f C:%.2f V:%ld", stockData.ticker, stockData.high, stockData.low, stockData.open, stockData.close, stockData.volume);
    return quoteBuffer;
  }
  return nullptr;
}

void Stock::setTicker(const char newTicker[STOCK_BUFFER_SIZE])
{
  strcpy(ticker, newTicker);
}