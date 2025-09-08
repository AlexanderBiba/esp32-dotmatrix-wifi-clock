#ifndef APP_WIFI_H
#define APP_WIFI_H

#include <WiFiServer.h>
#include <WiFiManager.h>

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
        FACTORY_RESET,
        CARD_ORDER,
        CARD_DURATIONS,
        FLIP_DISPLAY
    };

    AppServer(AppSettings *settings);
    void setupWiFi(char *localIp);
    RequestMode handleWiFi(char requestBuffer[1024], boolean activeCards[OPERATION_MODE_LENGTH]);
    void getLocalIp(char localIp[64]);
    void processWiFi();

private:
    void startMDNS();
    
    WiFiServer *server;
    AppSettings *settings;
    WiFiManager wifiManager;
};

#endif