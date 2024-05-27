#include <MD_MAX72xx.h>

#include "utils.h"

#define HARDWARE_TYPE MD_MAX72XX::DR1CR0RR0_HW
#define MAX_DEVICES 4

// GPIO pins
#define CLK_PIN 14  // VSPI_SCK
#define DATA_PIN 13 // VSPI_MOSI
#define CS_PIN 12   // VSPI_SS

const uint8_t CHAR_SPACING = 1;
const uint16_t SCROLL_DELAY = 75;

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

char curMessage[MAX_MSG_SIZE];
char newMessage[MAX_MSG_SIZE];
bool newMessageAvailable = false;

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
                S_SHOW_SPACE } state = S_IDLE;
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
    }
    state = S_NEXT_CHAR;
    break;

  case S_NEXT_CHAR: // Load the next character from the font table
    PRINT("\nS_NEXT_CHAR ", *p);
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
    PRINTS("\nS_SHOW_CHAR");
    colData = cBuf[curLen++];
    static char buffer[8];
    sprintf(buffer, "%02X", colData);
    PRINT("\nscrollDataIn: ", buffer);
    if (curLen < showLen)
      break;

    // set up the inter character spacing
    showLen = (*p != '\0' ? CHAR_SPACING : (MAX_DEVICES * COL_SIZE) / 2);
    curLen = 0;
    state = S_SHOW_SPACE;
    // fall through

  case S_SHOW_SPACE: // display inter-character spacing (blank column)
    PRINT("\nS_SHOW_SPACE: ", curLen);
    PRINT("/", showLen);
    curLen++;
    if (curLen == showLen)
      state = S_NEXT_CHAR;
    break;

  default:
    state = S_IDLE;
  }

  return (colData);
}

void scrollDataOut(uint8_t dev, MD_MAX72XX::transformType_t t, uint8_t colData)
{
  static char buffer[8];
  sprintf(buffer, "%02X", colData);
  PRINT("\nscrollDataOut: ", buffer);
}

void setMessage(char *message)
{
  strcpy(newMessage, message);
  newMessageAvailable = true;
}

void setupRenderer()
{
  // Display initialization
  PRINTS("\nInitializing Display");
  mx.begin();
  mx.setShiftDataInCallback(scrollDataIn);
  mx.setShiftDataOutCallback(scrollDataOut);

  curMessage[0] = newMessage[0] = '\0';
}
