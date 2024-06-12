#ifndef RENDERER_H
#define RENDERER_H

#include <MD_MAX72xx.h>

#include "clock.h"
#include "main.h"

#define MAX_DEVICES 4

class Renderer
{
public:
    Renderer(AppSettings *settings);

    void scrollText(void);
    void setMessage(const char *message);
    void setScrollContent(bool val);
    void updateBrightness();
    void setRaw(const uint8_t rawBuffer[MAX_DEVICES * 8]);

private:
    enum struct RenderMode
    {
        MSG,
        RAW
    };

    AppSettings *settings;
    RenderMode mode = RenderMode::MSG;

    char curMessage[REQUEST_BUFFER_SIZE] = {0};
    char newMessage[REQUEST_BUFFER_SIZE] = {0};
    bool newMessageAvailable = false;

    uint8_t curRaw[MAX_DEVICES * 8] = {0};
    uint8_t newRaw[MAX_DEVICES * 8] = {0};
    bool newRawAvailable = false;

    bool scrollContent = true;

    uint8_t scrollDataIn(uint8_t dev, MD_MAX72XX::transformType_t t);

    MD_MAX72XX *mx;
    int writeCharToBuffer(char c, uint8_t *buffer);
};

#endif