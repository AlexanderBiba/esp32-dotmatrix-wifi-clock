#include <ArduinoJson.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <WiFiClientSecure.h>

#include "utils.h"
#include "stock.h"

StockData stockData = {0};
char ticker[STOCK_BUFFER_SIZE] = "";
char apiKey[STOCK_API_KEY_BUFFER_SIZE] = "";

void setupStocks(AppSettings *settings)
{
  if (settings && strlen(settings->stockApiKey))
  {
    strcpy(apiKey, settings->stockApiKey);
  }
}

bool updateStockData()
{
  WiFiClientSecure wifiClient;
  wifiClient.setInsecure();
  HttpClient client = HttpClient(wifiClient, "api.finazon.io", 443);
  char url[128] = "/latest/time_series?dataset=us_stocks_essential&interval=1m&page=0&page_size=1&adjust=all&ticker=";
  strcat(url, ticker);
  char authHeader[128] = {0};
  strcat(authHeader, "apikey ");
  strcat(authHeader, apiKey);
  PRINT("url: ", url);
  PRINT("authHeader: ", authHeader);
  client.beginRequest();
  client.get(url);
  client.sendHeader("Authorization", authHeader);
  client.endRequest();
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  if (statusCode == 429)
  {
    PRINTS("Rate limited");
    return false;
  }

  PRINT("Status code: ", statusCode);
  PRINT("Response: ", response);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error)
  {
    PRINT("deserializeJson() failed: ", error.f_str());
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

bool getQuote(char quoteBuffer[STOCK_BUFFER_SIZE])
{
  static long lastRefresh = 0;
  if (strlen(apiKey) == 0 || strlen(ticker) == 0)
  {
    strcpy(quoteBuffer, "No API key or ticker set");
    return true;
  }
  if (millis() - lastRefresh > 60000 && updateStockData())
  {
    lastRefresh = millis();
    sprintf(quoteBuffer, "%s: H:%.2f L:%.2f O:%.2f C:%.2f V:%ld", stockData.ticker, stockData.high, stockData.low, stockData.open, stockData.close, stockData.volume);
    return true;
  }
  return false;
}

void setApiKey(const char newApiKey[STOCK_API_KEY_BUFFER_SIZE])
{
  strcpy(apiKey, newApiKey);
}

void setTicker(const char newTicker[STOCK_BUFFER_SIZE])
{
  strcpy(ticker, newTicker);
}