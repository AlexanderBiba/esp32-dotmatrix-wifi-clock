#ifndef __SNAKE_H__
#define __SNAKE_H__

#include <Arduino.h>
#include "settings.h"

#define RAW_SNAKE_BITMAP_SIZE 32

class Node
{
public:
    Node(uint8_t x, uint8_t y) : x(x), y(y){};
    uint8_t x;
    uint8_t y;
    Node *next = NULL;
    Node *prev = NULL;
};

class Snake
{
public:
    Snake(AppSettings *settings) : settings(settings) { initBitmap(); };
    uint8_t *getSnake();

private:
    AppSettings *settings;
    uint8_t bitmap[RAW_SNAKE_BITMAP_SIZE];

    // uint8_t food_x = 0;
    // uint8_t food_y = 0;
    uint8_t food_x = random(0, RAW_SNAKE_BITMAP_SIZE);
    uint8_t food_y = random(0, 8);

    typedef enum
    {
        UP,
        DOWN,
        LEFT,
        RIGHT
    } Direction;

    Node *head;
    Node *tail;

    boolean moveSnake(Direction direction);
    boolean grow = false;

    void initBitmap();
    boolean occupied[RAW_SNAKE_BITMAP_SIZE][8] = {0};
};

#endif // __SNAKE_H__