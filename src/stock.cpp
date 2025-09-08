#include <ArduinoJson.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <WiFiClientSecure.h>

#include "stock.h"

bool Stock::updateStockData()
{
  WiFiClientSecure wifiClient;
  wifiClient.setInsecure();
  
  // Set timeout for the client
  wifiClient.setTimeout(10000); // 10 second timeout
  
  HttpClient client = HttpClient(wifiClient, "api.finazon.io", 443);
  char url[128] = "/latest/time_series?dataset=us_stocks_essential&interval=1m&page=0&page_size=1&adjust=all&ticker=";
  
  // Check if adding ticker would exceed buffer size
  if (strlen(url) + strlen(ticker) >= sizeof(url)) {
    Serial.println("URL too long with ticker");
    return false;
  }
  strcat(url, ticker);
  
  char authHeader[128] = {0};
  strcat(authHeader, "apikey ");
  
  // Check if adding API key would exceed buffer size
  if (strlen(authHeader) + strlen(settings->getStockApiKey()) >= sizeof(authHeader)) {
    Serial.println("Auth header too long");
    return false;
  }
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
  
  if (statusCode != 200) {
    Serial.printf("Stock API error: %d\n", statusCode);
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
  if (!doc["data"] || !doc["data"].is<JsonArray>() || doc["data"].size() == 0) {
    Serial.println("Invalid stock data structure");
    return false;
  }
  
  JsonObject dataItem = doc["data"][0];
  if (!dataItem["t"] || !dataItem["o"] || 
      !dataItem["c"] || !dataItem["h"] || 
      !dataItem["l"] || !dataItem["v"]) {
    Serial.println("Missing required stock data fields");
    return false;
  }

  stockData = {
      ticker,
      dataItem["t"],
      dataItem["o"],
      dataItem["c"],
      dataItem["h"],
      dataItem["l"],
      dataItem["v"]};

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