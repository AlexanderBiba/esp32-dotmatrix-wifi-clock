#ifndef CLK_H
#define CLK_H

#include "app_settings.h"

#define TIME_BUFFER_SIZE sizeof("hh:mm:ss")

void setupClk(AppSettings *settings = nullptr);
bool getTime(char timeBuffer[TIME_BUFFER_SIZE]);
void setTimezone(const char *newTimezone);

#endif