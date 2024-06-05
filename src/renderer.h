#ifndef RENDERER_H
#define RENDERER_H

#include "clk.h"

#define MAX_DEVICES 4

void setupRenderer(void);
void scrollText(void);
void setMessage(char *message);
void setClock(char timeBuffer[TIME_BUFFER_SIZE]);
void setScrollContent(bool val);

#endif