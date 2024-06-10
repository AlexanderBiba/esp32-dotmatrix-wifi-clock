#ifndef STOCK_H
#define STOCK_H

#include "app_settings.h"

#define STOCK_API_KEY_BUFFER_SIZE 128
#define STOCK_BUFFER_SIZE 128

typedef struct
{
    char *ticker;
    long time;
    float open;
    float close;
    float high;
    float low;
    long volume;
} StockData;

void setupStocks(AppSettings *settings);
bool getQuote(char quoteBuffer[STOCK_BUFFER_SIZE]);
void setApiKey(const char *newApiKey);
void setTicker(const char *newTicker);

#endif // STOCK_H