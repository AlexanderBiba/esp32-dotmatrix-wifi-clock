#ifndef APP_WIFI_H
#define APP_WIFI_H

#include "main.h"

enum struct AppRequestMode
{
    NONE,
    MSG,
    CLK,
    STOCK,
    CNTL
};

void setupWiFi(char *localIp);
AppRequestMode handleWiFi(char requestBuffer[1024]);

#endif