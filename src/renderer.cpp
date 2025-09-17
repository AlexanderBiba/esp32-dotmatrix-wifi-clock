#include <MD_MAX72xx.h>
#include <algorithm>

#include "renderer.h"
#include "main.h"
#include "settings.h"
#include "charmaps.h"

#define ESP32S2_PINOUT 1

// GPIO pins
#ifdef ESP32_PINOUT
#define CLK_PIN 14  // VSPI_SCK
#define DATA_PIN 13 // VSPI_MOSI
#define CS_PIN 12   // VSPI_SS
#endif

#ifdef ESP32S2_PINOUT
#define CLK_PIN 33  // VSPI_SCK
#define DATA_PIN 16 // VSPI_MOSI
#define CS_PIN 18   // VSPI_SS
#endif

static std::function<uint8_t(uint8_t, MD_MAX72XX::transformType_t)> lambdaHolder;
uint8_t lambdaWrapper(uint8_t dev, MD_MAX72XX::transformType_t t)
{
    return lambdaHolder(dev, t);
}

Renderer::Renderer(AppSettings *settings)
{
    this->settings = settings;

    lambdaHolder = [this](uint8_t dev, MD_MAX72XX::transformType_t t) -> uint8_t {
        return this->scrollDataIn(dev, t);
    };

    mx = new MD_MAX72XX(settings->getFlipped() ? MD_MAX72XX::DR1CR0RR1_HW : MD_MAX72XX::DR1CR0RR0_HW, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
    mx->begin();
    mx->setShiftDataInCallback(lambdaWrapper);
    mx->control(MD_MAX72XX::INTENSITY, settings->getBrightness());
}

void Renderer::scrollText(void)
{
    static uint32_t prevTime = 0;
    if (scrollContent && (millis() - prevTime >= SCROLL_DELAY))
    {
        mx->transform(settings->getFlipped() ? MD_MAX72XX::TSR : MD_MAX72XX::TSL);
        prevTime = millis();
    }
}

void Renderer::stopMessage()
{
    _stopMessage = true;
}

void horizontalFlip(uint8_t arr[], int n)
{
    int left = 0;
    int right = n - 1;
    int temp;

    while (left < right)
    {
        temp = arr[left];
        arr[left] = arr[right];
        arr[right] = temp;
        left++;
        right--;
    }
}

uint8_t Renderer::scrollDataIn(uint8_t dev, MD_MAX72XX::transformType_t t)
{
    static enum { S_IDLE,
                  S_NEXT_CHAR,
                  S_SHOW_CHAR,
                  S_SHOW_SPACE,
                  S_NEXT_RAW } state = S_IDLE;
    static char *p;
    static uint16_t curLen, showLen;
    static uint8_t cBuf[8];
    uint8_t colData = 0;

    switch (state)
    {
    case S_IDLE:
        p = curMessage;
        if (newMessageAvailable)
        {
            strcpy(curMessage, newMessage);
            newMessageAvailable = false;
            _stopMessage = false; // Reset the stop flag for the new message
            state = S_NEXT_CHAR;
            mode = RenderMode::MSG;
        }
        else if (newRawAvailable)
        {
            uint8_t temp[MAX_DEVICES * 8];
            memcpy(temp, newRaw, sizeof(newRaw));
            if (settings->getFlipped())
            {
                horizontalFlip(temp, MAX_DEVICES * 8);
            }
            memcpy(curRaw, temp, sizeof(curRaw));
            newRawAvailable = false;
            state = S_NEXT_RAW;
            curLen = 0;
            showLen = sizeof(curRaw) / sizeof(curRaw[0]);
            mode = RenderMode::RAW;
        }

        if (mode == RenderMode::MSG)
        {
            state = S_NEXT_CHAR;
        }
        else if (mode == RenderMode::RAW)
        {
            state = S_NEXT_RAW;
        }
        break;

    case S_NEXT_CHAR:
        // Check for the end of the string OR our new stop flag
        if (*p == '\0' || _stopMessage)
        {
            _stopMessage = false; // Reset the flag for next time
            state = S_IDLE;
        }
        else
        {
            showLen = mx->getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
            curLen = 0;
            state = S_SHOW_CHAR;
        }
        break;

    case S_SHOW_CHAR:
        colData = cBuf[curLen++];
        if (curLen < showLen)
            break;

        showLen = (*p != '\0' ? 1 : (MAX_DEVICES * COL_SIZE) / 2);
        curLen = 0;
        state = S_SHOW_SPACE;
        // fall through

    case S_SHOW_SPACE:
        curLen++;
        if (curLen == showLen)
            state = S_NEXT_CHAR;
        break;

    case S_NEXT_RAW:
        colData = curRaw[curLen++];
        if (curLen == showLen)
        {
            curLen = 0;
            scrollContent = false; // Stop scrolling when raw bitmap is fully displayed
            state = S_IDLE;
        }
        break;

    default:
        state = S_IDLE;
    }

    return (colData);
}

void Renderer::setScrollContent(bool val)
{
    scrollContent = val;
}

void Renderer::setMessage(const char *message)
{
    if (message == nullptr)
    {
        return;
    }
    strcpy(newMessage, message);
    newMessageAvailable = true;
    newRawAvailable = false;
    scrollContent = true;
}

void Renderer::setRaw(uint8_t rawBuffer[MAX_DEVICES * 8])
{
    if (rawBuffer == nullptr)
        return;

    uint8_t temp[MAX_DEVICES * 8];
    memcpy(temp, rawBuffer, sizeof(temp));
    if (settings->getFlipped())
    {
        horizontalFlip(temp, MAX_DEVICES * 8);
    }

    if (scrollContent)
    {
        memcpy(newRaw, temp, sizeof(newRaw));
        newRawAvailable = true;
        newMessageAvailable = false;
    }
    else
    {
        if (!mx->setBuffer(mx->getColumnCount() - 1, MAX_DEVICES * 8, temp))
        {
            Serial.println("Failed to set buffer");
        }
    }
}

void Renderer::updateBrightness()
{
    mx->control(MD_MAX72XX::INTENSITY, settings->getBrightness());
}

void shiftRight(uint8_t *bitmap, uint8_t size, uint8_t shiftAmount)
{
    if (shiftAmount == 0)
    {
        return;
    }
    for (uint8_t i = size - 1; i >= shiftAmount; --i)
    {
        bitmap[i] = bitmap[i - shiftAmount];
    }
    for (uint8_t i = 0; i < shiftAmount; ++i)
    {
        bitmap[i] = 0;
    }
}

uint8_t Renderer::writeCharToBuffer(char c, uint8_t *buffer)
{
    for (uint8_t i = 0; i < CLOCK_DIGIT_WIDTH; i++)
    {
#if BOTTOM_ALIGN
        buffer[i] = digitCharMap[c - '0'][i] * 2;
#else
        buffer[i] = digitCharMap[c - '0'][i];
#endif
    }
    return CLOCK_DIGIT_WIDTH;
}

uint8_t Renderer::writeSmallCharToBuffer(char c, uint8_t *buffer)
{
    for (uint8_t i = 0; i < SMALL_DIGIT_LEN; i++)
    {
#if BOTTOM_ALIGN
        buffer[i] = smallDigitCharMap[c - '0'][i] * 8;
#else
        buffer[i] = smallDigitCharMap[c - '0'][i];
#endif
    }
    return SMALL_DIGIT_LEN;
}

#define RAW_BITMAP_SIZE MAX_DEVICES * 8
uint8_t *Renderer::loadStringToBitmap(const char *str, uint8_t *bitmap, bool smallNumbers)
{
    if (!str || !bitmap)
    {
        return bitmap;
    }

    const int length = strlen(str);
    uint8_t *curr = bitmap;
    const uint8_t *bitmapEnd = bitmap + RAW_BITMAP_SIZE;

    for (int i = 0; i < length; ++i)
    {
        if (curr >= bitmapEnd)
        {
            Serial.println("Buffer overflow prevented in loadStringToBitmap");
            return curr;
        }

        if (str[i] == ' ')
        {
            *curr++ = 0;
            if (curr < bitmapEnd)
                *curr++ = 0;
        }
        else if ('0' <= str[i] && str[i] <= '9')
        {
            if (smallNumbers)
            {
                const uint8_t *digit = smallDigitCharMap[str[i] - '0'];
                for (int j = 0; j < SMALL_DIGIT_LEN; ++j)
                {
                    if (curr >= bitmapEnd)
                    {
                        Serial.println("Buffer overflow prevented in loadStringToBitmap (small numbers)");
                        return curr;
                    }
#ifdef BOTTOM_ALIGN
                    *curr++ = *digit++ * 8;
#else
                    *curr++ = *digit++;
#endif
                }
                if (curr < bitmapEnd)
                    *curr++ = 0;
            }
            else
            {
                const uint8_t *digit = digitCharMap[str[i] - '0'];
                for (int j = 0; j < CLOCK_DIGIT_WIDTH; ++j)
                {
                    if (curr >= bitmapEnd)
                    {
                        Serial.println("Buffer overflow prevented in loadStringToBitmap (regular numbers)");
                        return curr;
                    }
#ifdef BOTTOM_ALIGN
                    *curr++ = *digit++ * 2;
#else
                    *curr++ = *digit++;
#endif
                }
                if (curr < bitmapEnd)
                    *curr++ = 0;
            }
        }
        else
        {
            const uint8_t *charBitmap = appCharMap[str[i] - '!'];
            for (int j = 0; j < charBitmap[0]; ++j)
            {
                if (curr >= bitmapEnd)
                {
                    Serial.println("Buffer overflow prevented in loadStringToBitmap (characters)");
                    return curr;
                }
#ifdef BOTTOM_ALIGN
                *curr++ = charBitmap[j + 1] * 2;
#else
                *curr++ = charBitmap[j + 1];
#endif
            }
        }
    }
    return curr;
}

void Renderer::alignBitmapContentToCenter(uint8_t *bitmap, uint8_t *endPtr)
{
    const uint16_t margin = &bitmap[RAW_BITMAP_SIZE] - endPtr;
    if (margin == 0)
    {
        return;
    }
    shiftRight(bitmap, RAW_BITMAP_SIZE, margin - (margin / 2));
    if (margin / 2 == 0)
    {
        return;
    }
    for (int i = 0; i < margin / 2; ++i)
    {
        bitmap[(RAW_BITMAP_SIZE - margin / 2) + i] = 0;
    }
}