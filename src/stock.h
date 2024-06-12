#ifndef STOCK_H
#define STOCK_H

#include "settings.h"

#define STOCK_API_KEY_BUFFER_SIZE 128
#define STOCK_BUFFER_SIZE 128

class Stock
{
public:
    Stock(AppSettings *settings) : settings(settings){};
    const char *getQuote();
    void setTicker(const char *newTicker);

private:
    AppSettings *settings;
    char ticker[STOCK_BUFFER_SIZE] = {0};
    struct
    {
        char *ticker;
        long time;
        float open;
        float close;
        float high;
        float low;
        long volume;
    } stockData;
    bool updateStockData();
    char quoteBuffer[STOCK_BUFFER_SIZE];
};

#endif // STOCK_H