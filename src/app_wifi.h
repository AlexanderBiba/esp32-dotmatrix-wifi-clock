#ifndef APP_WIFI_H
#define APP_WIFI_H

#include "main.h"

void setupWiFi(char *localIp);
OperationMode handleWiFi(char msgBuffer[MAX_MSG_SIZE]);
void setDefaultOperationMode(OperationMode defaultOperationMode);

#endif