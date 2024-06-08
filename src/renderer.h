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
void setMessage(char *message);
void setClock(char timeBuffer[TIME_BUFFER_SIZE]);
void setScrollContent(bool val);
void controlRenderer(ControlRequest controlRequest, int controlValue);
void setScrollDelayMs(uint16_t delay);

#endif