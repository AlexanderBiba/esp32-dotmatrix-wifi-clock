#include <Arduino.h>
#include <ctime>
#include <cmath>
#include <ezTime.h>

#include "countdown.h"
#include "charmaps.h"
#include "main.h"
#include "renderer.h"

#define COUNTDOWN_UPDATE_INTERVAL 60 * 1000 // Update every minute

extern Renderer *renderer;
extern Timezone myTimezone;

void Countdown::loadBitmap(const char *countdown)
{
  uint8_t *rawCountdown = bitmap;
  *rawCountdown++ = 0;
  *rawCountdown++ = 0;
  rawCountdown = renderer->loadStringToBitmap(countdown, rawCountdown);
  *rawCountdown++ = 0;
  // Add "days" text
  *rawCountdown++ = 0xF0;
  *rawCountdown++ = 0x90;
  *rawCountdown++ = 0x90;
  *rawCountdown++ = 0x60;
  renderer->alignBitmapContentToCenter(bitmap, rawCountdown);
}

uint8_t *Countdown::getCountdown()
{
  static long lastRefresh = 0;

  if ((millis() - lastRefresh > COUNTDOWN_UPDATE_INTERVAL) && updateCountdownData())
  {
    lastRefresh = millis();
    updateCountdownData();
  }

  return bitmap;
}

bool Countdown::updateCountdownData()
{
  static char localBuffer[RAW_COUNTDOWN_BITMAP_SIZE] = {0};

  Serial.println("Updating countdown data");
  
  // Get target date from settings
  time_t targetTime = settings->getCountdownTargetDate();
  
  if (targetTime == 0) {
    Serial.println("No countdown target date set");
    renderer->alignBitmapContentToCenter(bitmap, renderer->loadStringToBitmap("No Date", bitmap));
    return true;
  }

  // Get current time using the same timezone system as the clock
  time_t now = myTimezone.now();
  
  if (now == 0) {
    Serial.println("Failed to get current time from timezone");
    renderer->alignBitmapContentToCenter(bitmap, renderer->loadStringToBitmap("No Time", bitmap));
    return true;
  }

  // Calculate difference in days
  double diffSeconds = difftime(targetTime, now);
  int days = static_cast<int>(ceil(diffSeconds / (24 * 60 * 60)));
  
  Serial.printf("Target time: %ld, Current time: %ld, Days remaining: %d\n", targetTime, now, days);

  // Handle past dates
  if (days < 0) {
    days = 0; // Show 0 days for past dates
  }

  countdownData = {days};

  sprintf(localBuffer, "%d", countdownData.days);
  loadBitmap(localBuffer);

  return true;
}
