#ifndef MAIN_UTILS_H
#define MAIN_UTILS_H

#include <stdint.h>

#define PRINT(s, v)     \
  {                     \
    Serial.print(F(s)); \
    Serial.print(v);    \
  }
#define PRINTS(s)       \
  {                     \
    Serial.print(F(s)); \
  }

#define MAX_MSG_SIZE 255

#endif