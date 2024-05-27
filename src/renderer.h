#ifndef RENDERER_H
#define RENDERER_H

#define MAX_DEVICES 4

void setupRenderer(void);
void scrollText(void);
void setMessage(char *message);
void setRawMessage(uint8_t *raw);
uint8_t getChar(char c, size_t size, uint8_t *buffer);
bool getStopScroll();
void setStopScroll(bool val);
void renderRaw(uint8_t rawColumns[MAX_DEVICES * 8]);

#endif