#ifndef APP_WIFI_H
#define APP_WIFI_H


void setupWiFi(char *localIp);
void handleWiFi(void (*getText)(char *szMesg, uint8_t len));

#endif