#ifndef RENDERER_H
#define RENDERER_H

#include "clk.h"

#define MAX_DEVICES 4

enum struct ControlRequest
{
    Intensity
};

void setupRenderer(AppSettings *settings = nullptr);
void scrollText(void);
void setMessage(const char *message);
void setScrollContent(bool val);
void controlRenderer(ControlRequest controlRequest, int controlValue);
void setScrollDelayMs(uint16_t delay);
void setRaw(uint8_t rawBuffer[MAX_DEVICES * 8]);
void parseTime(char timeBuffer[TIME_BUFFER_SIZE], uint8_t rawClkBuffer[MAX_DEVICES * 8]);

#endif