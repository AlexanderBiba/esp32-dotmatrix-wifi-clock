#ifndef APP_WIFI_H
#define APP_WIFI_H

#include <WiFiServer.h>

#include "main.h"
#include "settings.h"

class AppServer
{
public:
    enum struct RequestMode
    {
        NONE,
        MODE,
        CNTL,
        SETT,
        SYSINFO,
        REBOOT,
        CLEAR_SETTINGS,
        FACTORY_RESET
    };

    AppServer(AppSettings *settings);
    void setupWiFi(char *localIp);
    RequestMode handleWiFi(char requestBuffer[1024], boolean activeCards[OPERATION_MODE_LENGTH]);
    void getLocalIp(char localIp[64]);

private:
    WiFiServer *server;
    AppSettings *settings;
};

#endif