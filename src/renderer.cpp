#include <MD_MAX72xx.h>
#include <algorithm>

#include "utils.h"
#include "renderer.h"
#include "main.h"
#include "settings.h"

// GPIO pins
#define CLK_PIN 14  // VSPI_SCK
#define DATA_PIN 13 // VSPI_MOSI
#define CS_PIN 12   // VSPI_SS

#define MIN_SCROLL_DELAY (uint16_t)25
#define MAX_SCROLL_DELAY (uint16_t)10000

static std::function<uint8_t(uint8_t, MD_MAX72XX::transformType_t)> lambdaHolder;
uint8_t lambdaWrapper(uint8_t dev, MD_MAX72XX::transformType_t t)
{
  return lambdaHolder(dev, t);
}

Renderer::Renderer(AppSettings *settings)
{
  this->settings = settings;

  lambdaHolder = [this](uint8_t dev, MD_MAX72XX::transformType_t t) -> uint8_t
  {
    return this->scrollDataIn(dev, t);
  };

  mx = new MD_MAX72XX(MD_MAX72XX::DR1CR0RR0_HW, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
  mx->begin();
  mx->setShiftDataInCallback(lambdaWrapper);
  mx->control(MD_MAX72XX::INTENSITY, settings->getBrightness());
}

void Renderer::scrollText(void)
{
  static uint32_t prevTime = 0;
  if (scrollContent && (millis() - prevTime >= min(max(settings->getScrollDelay(), MIN_SCROLL_DELAY), MAX_SCROLL_DELAY)))
  {
    mx->transform(MD_MAX72XX::TSL);
    prevTime = millis();
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

  // finite state machine to control what we do on the callback
  switch (state)
  {
  case S_IDLE:               // reset the message pointer and check for new message to load
    p = curMessage;          // reset the pointer to start of message
    if (newMessageAvailable) // there is a new message waiting
    {
      strcpy(curMessage, newMessage); // copy it in
      newMessageAvailable = false;
      state = S_NEXT_CHAR;
      mode = RenderMode::MSG;
    }
    else if (newRawAvailable)
    {
      PRINT("newRawAvailable: ", newRawAvailable);
      PRINT("before newRaw[0]: ", newRaw[0]);
      PRINT("befoer curRaw[0]: ", curRaw[0]);
      memcpy(curRaw, newRaw, sizeof(curRaw));
      newRawAvailable = false;
      state = S_NEXT_RAW;
      curLen = 0;
      showLen = sizeof(curRaw) / sizeof(curRaw[0]);
      mode = RenderMode::RAW;
      PRINT("after newRaw[0]: ", newRaw[0]);
      PRINT("after curRaw[0]: ", curRaw[0]);
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

  case S_NEXT_CHAR: // Load the next character from the font table
    if (*p == '\0')
      state = S_IDLE;
    else
    {
      showLen = mx->getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
      curLen = 0;
      state = S_SHOW_CHAR;
    }
    break;

  case S_SHOW_CHAR: // display the next part of the character
    colData = cBuf[curLen++];
    static char buffer[8];
    sprintf(buffer, "%02X", colData);
    if (curLen < showLen)
      break;

    // set up the inter character spacing
    showLen = (*p != '\0' ? 1 : (MAX_DEVICES * COL_SIZE) / 2);
    curLen = 0;
    state = S_SHOW_SPACE;
    // fall through

  case S_SHOW_SPACE: // display inter-character spacing (blank column)
    curLen++;
    if (curLen == showLen)
      state = S_NEXT_CHAR;
    break;

  case S_NEXT_RAW:
    colData = curRaw[curLen++];
    if (curLen == showLen)
    {
      curLen = 0;
      scrollContent = false;
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
}

void Renderer::setRaw(const uint8_t rawBuffer[MAX_DEVICES * 8])
{
  if (rawBuffer == nullptr)
  {
    return;
  }

  if (scrollContent)
  {
    memcpy(newRaw, rawBuffer, sizeof(newRaw));
    newRawAvailable = true;
    newMessageAvailable = false;
  }
  else
  {
    for (uint8_t i = 0; i < MAX_DEVICES * 8; i++)
    {
      mx->setColumn(MAX_DEVICES * 8 - i - 1, rawBuffer[i]);
    }
  }
}

void Renderer::updateBrightness()
{
  mx->control(MD_MAX72XX::INTENSITY, settings->getBrightness());
}
