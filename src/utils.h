#ifndef MAIN_UTILS_H
#define MAIN_UTILS_H

#include <stdint.h>
#include <Arduino.h>

#define PRINT(s, v)     \
  {                     \
    Serial.print(F(s)); \
    Serial.println(v);  \
  }
#define PRINTS(s)         \
  {                       \
    Serial.println(F(s)); \
  }

#endif