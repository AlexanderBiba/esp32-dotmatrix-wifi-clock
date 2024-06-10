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
  // char authHeader[128] = {0};
  // strcat(authHeader, "apikey ");
  // strcat(authHeader, apiKey);
  String authHeader = "apikey ";
  authHeader += apiKey;
  PRINT("url: ", url);
  PRINT("authHeader: ", authHeader);
  client.sendHeader("Authorization", "apikey fc690b409a944cb5930a5848cae5bc28dh");
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
  static long prevTime = 0;
  if (millis() - prevTime > 5000)
  {
    PRINT("apiKey: ", apiKey);
    PRINT("ticker: ", ticker);

    prevTime = millis();
    if (strlen(apiKey) == 0 || strlen(ticker) == 0)
    {
      strcpy(quoteBuffer, "No API key or ticker set");
      return true;
    }

    bool newVal = false;
    if (millis() - stockData.time > 10000 && updateStockData())
    {
      sprintf(quoteBuffer, "%s: h:%.2f l:%.2f o:%.2f c:%.2f v:%ld", stockData.ticker, stockData.high, stockData.low, stockData.open, stockData.close, stockData.volume);
      return true;
    }
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