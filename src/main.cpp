#include <ctime>
#include <cstring>
#include <ctype.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#include "main.h"
#include "clock.h"
#include "stock.h"
#include "appserver.h"
#include "renderer.h"
#include "settings.h"
#include "weather.h"
#include "card.h"
#include "snake.h"
#include "buzzer.h"

#define PRINT_CALLBACK 0
#define DEBUG 1
#define LED_HEARTBEAT 0

AppSettings *settings;
Clock *clk;
AppServer *appServer;
Renderer *renderer;
Stock *stock;
Weather *weather;
Snake *snake;
Buzzer *buzzer;

void setup(void)
{
  // No printf here. For some reason nothing is printed during setup.
  Serial.begin(115200);
  EEPROM.begin(0x400);

  settings = new AppSettings();
  appServer = new AppServer(settings);
  renderer = new Renderer(settings);
  clk = new Clock(settings);
  weather = new Weather(settings);
  stock = new Stock(settings);
  snake = new Snake(settings);
  buzzer = new Buzzer(settings);
}

void handleControlRequest(char *requestBuffer)
{
  printf("Control request: %s\n", requestBuffer);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, requestBuffer);
  if (error)
  {
    printf("Control deserializeJson() failed: %s\n", error.f_str());
    return;
  }

  bool updateWeather = false;

  if (doc.containsKey("brightness"))
  {
    settings->setBrightness(doc["brightness"]);
    renderer->updateBrightness();
  }
  if (doc.containsKey("timezone"))
  {
    settings->setTimezone(doc["timezone"]);
    clk->updateTime();
  }
  if (doc.containsKey("latitude"))
  {
    settings->setLatitude(doc["latitude"]);
    updateWeather = true;
  }
  if (doc.containsKey("longitude"))
  {
    settings->setLongitude(doc["longitude"]);
    updateWeather = true;
  }
  if (doc.containsKey("weatherUnits"))
  {
    settings->setWeatherUnits(!strcmp(doc["weatherUnits"], "c") ? 'c' : 'f');
    updateWeather = true;
  }
  if (doc.containsKey("alarmHour") && doc.containsKey("alarmMinute") && doc.containsKey("alarmEnabled"))
  {
    settings->setAlarm(doc["alarmHour"], doc["alarmMinute"], doc["alarmEnabled"]);
  }

  // updarte weather if required
  if (updateWeather)
  {
    weather->updateWeatherData();
  }
}

void loop(void)
{
  static Card *cards[] = {
      new Card(OperationMode::CLOCK, 10000),
      new Card(OperationMode::DATE, 5000),
      new Card(OperationMode::WEATHER, 5000),
      new Card(OperationMode::SNAKE, 5000),
      new Card(OperationMode::MESSAGE, 5000)};
  static uint8_t currentState = 0;
  static OperationMode operationMode = cards[currentState]->getOperationMode();

  OperationMode prevOperationMode = operationMode;

  // card switching logic
  static long curCardTime = 0;
  static long cardSwitchTime = 5000;
  if (millis() - curCardTime > cardSwitchTime)
  {
    curCardTime = millis();
    do
    {
      currentState = (currentState + 1) % (sizeof(cards) / sizeof(cards[0]));
    } while (!settings->getActiveCards()[static_cast<int>(cards[currentState]->getOperationMode())]);
    operationMode = cards[currentState]->getOperationMode();
    cardSwitchTime = cards[currentState]->getCardSwitchTime();
  }

  // handle control requests
  static char requestBuffer[REQUEST_BUFFER_SIZE];
  bool activeCards[OPERATION_MODE_LENGTH] = {0};
  switch (appServer->handleWiFi(requestBuffer, activeCards))
  {
  case AppServer::RequestMode::STOP:
  {
    buzzer->stop();
    break;
  }
  case AppServer::RequestMode::MODE:
  {
    settings->setActiveCards(activeCards);
    break;
  }
  case AppServer::RequestMode::CNTL:
    handleControlRequest(requestBuffer);
    break;
  default:
    break;
  }

  // handle buzzer
  if (settings->getAlarmEnabled())
  {
    AlarmTime currentTime = clk->getTimeStruct();
    if (currentTime.hour == settings->getAlarmHour() && currentTime.minute == settings->getAlarmMinute())
    {
      buzzer->beep();
    }
  }

  // handle cards
  bool reset = prevOperationMode != operationMode;
  if (reset)
  {
    renderer->setScrollContent(true);
  }

  static long prevTime = 0;
  if (reset || millis() - prevTime > 100)
  {
    prevTime = millis();
    switch (operationMode)
    {
    case OperationMode::MESSAGE:
      renderer->setMessage(requestBuffer);
      break;
    case OperationMode::CLOCK:
      renderer->setRaw(clk->getTime());
      break;
    case OperationMode::DATE:
      renderer->setRaw(clk->getDate());
      break;
    case OperationMode::WEATHER:
      renderer->setRaw(weather->getWeather());
      break;
    case OperationMode::SNAKE:
      renderer->setRaw(snake->getSnake());
      break;
    }
  }

  // scroll if required
  renderer->scrollText();
}
