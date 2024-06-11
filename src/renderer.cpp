#include <MD_MAX72xx.h>
#include <algorithm>

#include "utils.h"
#include "renderer.h"
#include "main.h"
#include "app_settings.h"

#define HARDWARE_TYPE MD_MAX72XX::DR1CR0RR0_HW

// GPIO pins
#define CLK_PIN 14  // VSPI_SCK
#define DATA_PIN 13 // VSPI_MOSI
#define CS_PIN 12   // VSPI_SS

const uint8_t CHAR_SPACING = 1;

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

#define MIN_SCROLL_DELAY (uint16_t)25
#define MAX_SCROLL_DELAY (uint16_t)10000

enum struct RenderMode
{
  MSG,
  RAW
};
RenderMode mode = RenderMode::MSG;

char curMessage[REQUEST_BUFFER_SIZE] = {0};
char newMessage[REQUEST_BUFFER_SIZE] = {0};
bool newMessageAvailable = false;

uint8_t curRaw[MAX_DEVICES * 8] = {0};
uint8_t newRaw[MAX_DEVICES * 8] = {0};
bool newRawAvailable = false;

bool scrollContent = true;
uint16_t scrollDelay = 75;

void scrollText(void)
{
  static uint32_t prevTime = 0;
  if (scrollContent && (millis() - prevTime >= min(max(scrollDelay, MIN_SCROLL_DELAY), MAX_SCROLL_DELAY)))
  {
    mx.transform(MD_MAX72XX::TSL);
    prevTime = millis();
  }
}

uint8_t scrollDataIn(uint8_t dev, MD_MAX72XX::transformType_t t)
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
      PRINTS("newMessageAvailable");
      strcpy(curMessage, newMessage); // copy it in
      newMessageAvailable = false;
      state = S_NEXT_CHAR;
      mode = RenderMode::MSG;
    }
    else if (newRawAvailable)
    {
      PRINTS("newRawAvailable");
      memcpy(curRaw, newRaw, sizeof(curRaw));
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

  case S_NEXT_CHAR: // Load the next character from the font table
    PRINT("S_NEXT_CHAR ", *p);
    if (*p == '\0')
      state = S_IDLE;
    else
    {
      showLen = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
      curLen = 0;
      state = S_SHOW_CHAR;
    }
    break;

  case S_SHOW_CHAR: // display the next part of the character
    PRINTS("S_SHOW_CHAR");
    colData = cBuf[curLen++];
    static char buffer[8];
    sprintf(buffer, "%02X", colData);
    PRINT("scrollDataIn: ", buffer);
    if (curLen < showLen)
      break;

    // set up the inter character spacing
    showLen = (*p != '\0' ? CHAR_SPACING : (MAX_DEVICES * COL_SIZE) / 2);
    curLen = 0;
    state = S_SHOW_SPACE;
    // fall through

  case S_SHOW_SPACE: // display inter-character spacing (blank column)
    PRINT("S_SHOW_SPACE: ", curLen);
    PRINT("/", showLen);
    curLen++;
    if (curLen == showLen)
      state = S_NEXT_CHAR;
    break;

  case S_NEXT_RAW:
    PRINTS("S_NEXT_RAW");
    colData = curRaw[curLen++];
    if (curLen == showLen)
    {
      PRINT("showLen ", showLen);
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

void setScrollContent(bool val)
{
  scrollContent = val;
}

void setMessage(const char *message)
{
  strcpy(newMessage, message);
  newMessageAvailable = true;
  newRawAvailable = false;
}

void setRaw(uint8_t rawBuffer[MAX_DEVICES * 8])
{
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
      mx.setColumn(MAX_DEVICES * 8 - i - 1, rawBuffer[i]);
    }
  }
}

#define CLK_DIGIT_WIDTH 5
int writeCharToBuffer(char c, uint8_t *buffer)
{

  uint8_t c1[CLK_DIGIT_WIDTH];
  uint8_t len = mx.getChar(c, CLK_DIGIT_WIDTH, c1);

  for (uint8_t i = 0; i < len; i++)
  {
#ifdef BOTTOM_ALIGN_CLOCK
    buffer[i] = c1[i] * 2;
#else
    buffer[i] = c1[i];
#endif
  }
  for (uint8_t i = len; i < CLK_DIGIT_WIDTH; i++)
  {
    buffer[i] = '\0';
  }
  return CLK_DIGIT_WIDTH;
}

const uint8_t smallCharMap[10][3] = {
    {0x1F, 0x11, 0x1F}, // 0
    {0x00, 0x00, 0x1F}, // 1
    {0x1D, 0x15, 0x17}, // 2
    {0x15, 0x15, 0x1F}, // 3
    {0x07, 0x04, 0x1F}, // 4
    {0x17, 0x15, 0x1D}, // 5
    {0x1F, 0x15, 0x1D}, // 6
    {0x01, 0x01, 0x1F}, // 7
    {0x1F, 0x15, 0x1F}, // 8
    {0x17, 0x15, 0x1F}, // 9
};

int writeSmallCharToBuffer(char c, uint8_t *buffer)
{
  const uint8_t len = 3;
  for (uint8_t i = 0; i < len; i++)
  {
#ifdef BOTTOM_ALIGN_CLOCK
    buffer[i] = smallCharMap[c - '0'][i] * 8;
#else
    buffer[i] = smallCharMap[c - '0'][i];
#endif
  }
  for (uint8_t i = len; i < CLK_DIGIT_WIDTH; i++)
  {
    buffer[i] = '\0';
  }
  return 3;
}

void parseTime(char timeBuffer[TIME_BUFFER_SIZE], uint8_t rawClkBuffer[MAX_DEVICES * 8])
{
  uint8_t *p = rawClkBuffer;

#ifdef SMALL_SECONDS_CLOCK
  p += writeCharToBuffer(timeBuffer[0], p);
  p += writeCharToBuffer(timeBuffer[1], p);
  *p++ = 0; // empty column between hours and minutes
  *p++ = 0; // empty column between hours and minutes
  p += writeCharToBuffer(timeBuffer[3], p);
  p += writeCharToBuffer(timeBuffer[4], p);
  *p++ = 0; // empty column between minutes and seconds
  *p++ = 0; // empty column between minutes and seconds
  p += writeSmallCharToBuffer(timeBuffer[6], p);
  *p++ = 0; // empty column
  p += writeSmallCharToBuffer(timeBuffer[7], p);
#else
  p += writeCharToBuffer(timeBuffer[0], p);
  p += writeCharToBuffer(timeBuffer[1], p);
  *p++ = 0; // empty column between hours and minutes
  p += writeCharToBuffer(timeBuffer[3], p);
  p += writeCharToBuffer(timeBuffer[4], p);
  *p++ = 0; // empty column between minutes and seconds
  p += writeCharToBuffer(timeBuffer[6], p);
  p += writeCharToBuffer(timeBuffer[7], p);
#endif
}

void setupRenderer(AppSettings *settings)
{
  PRINTS("Initializing Display");
  mx.begin();
  mx.setShiftDataInCallback(scrollDataIn);
  if (settings)
  {
    mx.control(MD_MAX72XX::INTENSITY, settings->brightness);
    scrollDelay = settings->scrollDelay;
  }
  curMessage[0] = newMessage[0] = '\0';
}

void controlRenderer(ControlRequest controlRequest, int controlValue)
{
  switch (controlRequest)
  {
  case ControlRequest::Intensity:
    mx.control(MD_MAX72XX::INTENSITY, controlValue);
    break;
  }
}

void setScrollDelayMs(uint16_t delay)
{
  scrollDelay = delay;
}