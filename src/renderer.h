#ifndef RENDERER_H
#define RENDERER_H

#include <MD_MAX72xx.h>

#include "clock.h"
#include "main.h"

#define MAX_DEVICES 4
#define SCROLL_DELAY (uint16_t)25

class Renderer
{
public:
    Renderer(AppSettings *settings);

    void scrollText(void);
    void setMessage(const char *message);
    void setScrollContent(bool val);
    void updateBrightness();
    void setRaw(uint8_t rawBuffer[MAX_DEVICES * 8]);
    uint8_t *loadStringToBitmap(const char *str, uint8_t *bitmap, bool smallNumbers = false);
    void alignBitmapContentToCenter(uint8_t *bitmap, uint8_t *endPtr);
    uint8_t writeCharToBuffer(char c, uint8_t *buffer);
    uint8_t writeSmallCharToBuffer(char c, uint8_t *buffer);
    void stopMessage();

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
    bool _stopMessage = false;

    uint8_t scrollDataIn(uint8_t dev, MD_MAX72XX::transformType_t t);

    MD_MAX72XX *mx;
};

#endif