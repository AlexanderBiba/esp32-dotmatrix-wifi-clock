#include <MD_MAX72xx.h>

#include "utils.h"
#include "renderer.h"

#define HARDWARE_TYPE MD_MAX72XX::DR1CR0RR0_HW

// GPIO pins
#define CLK_PIN 14  // VSPI_SCK
#define DATA_PIN 13 // VSPI_MOSI
#define CS_PIN 12   // VSPI_SS

const uint8_t CHAR_SPACING = 1;
const uint16_t SCROLL_DELAY = 75;

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

enum Mode
{
  MSG,
  RAW
};

Mode mode = MSG;
char curMessage[MAX_MSG_SIZE] = {0};
char newMessage[MAX_MSG_SIZE] = {0};
bool newMessageAvailable = false;

uint8_t curRaw[MAX_DEVICES * 8] = {0};
uint8_t newRaw[MAX_DEVICES * 8] = {0};
bool newRawAvailable = false;

bool stopScroll = false;

void scrollText(void)
{
  static uint32_t prevTime = 0;

  // Is it time to scroll the text?
  if (millis() - prevTime >= SCROLL_DELAY)
  {
    mx.transform(MD_MAX72XX::TSL); // scroll along - the callback will load all the data
    prevTime = millis();           // starting point for next time
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
      mode = MSG;
    }
    else if (newRawAvailable)
    {
      PRINTS("newRawAvailable");
      memcpy(curRaw, newRaw, sizeof(curRaw));
      newRawAvailable = false;
      state = S_NEXT_RAW;
      curLen = 0;
      showLen = sizeof(curRaw) / sizeof(curRaw[0]);
      mode = RAW;
    }

    if (mode == MSG)
    {
      state = S_NEXT_CHAR;
    }
    else if (mode == RAW)
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
      stopScroll = true;
      state = S_IDLE;
    }
    break;

  default:
    state = S_IDLE;
  }

  return (colData);
}

void scrollDataOut(uint8_t dev, MD_MAX72XX::transformType_t t, uint8_t colData)
{
  // static char buffer[8];
  // sprintf(buffer, "%02X", colData);
  // PRINT("scrollDataOut: ", buffer);
}

uint8_t getChar(char c, size_t size, uint8_t *buffer)
{
  return mx.getChar(c, size, buffer);
}

void setStopScroll(bool val)
{
  PRINT("setStopScroll: ", val);
  stopScroll = val;
}

bool getStopScroll(void)
{
  return stopScroll;
}

void setMessage(char *message)
{
  strcpy(newMessage, message);
  newMessageAvailable = true;
  newRawAvailable = false;
}

void setRawMessage(uint8_t *raw)
{
  PRINTS("setRawMessage");
  memcpy(newRaw, raw, sizeof(newRaw));
  newRawAvailable = true;
  newMessageAvailable = false;
}

void renderRaw(uint8_t rawColumns[MAX_DEVICES * 8])
{
  for (uint8_t i = 0; i < MAX_DEVICES * 8; i++)
  {
    mx.setColumn(MAX_DEVICES * 8 - i - 1, rawColumns[i]);
  }
}

void setupRenderer()
{
  // Display initialization
  PRINTS("Initializing Display");
  mx.begin();
  mx.setShiftDataInCallback(scrollDataIn);
  mx.setShiftDataOutCallback(scrollDataOut);

  curMessage[0] = newMessage[0] = '\0';
}
