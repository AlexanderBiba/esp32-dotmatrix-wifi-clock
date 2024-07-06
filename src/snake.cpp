#include "snake.h"

void Snake::initBitmap()
{
    memset(occupied, false, sizeof(occupied));

    uint8_t head_x;
    uint8_t head_y;
    while (head_x == food_x && head_y == food_y)
    {
        uint8_t head_x = random(0, RAW_SNAKE_BITMAP_SIZE);
        uint8_t head_y = random(0, 8);
    }
    head = new Node(head_x, head_y);
    tail = head;

    occupied[head->x][head->y] = true;

    uint8_t *rawSnake = bitmap;
    while (rawSnake < bitmap + RAW_SNAKE_BITMAP_SIZE)
    {
        *rawSnake++ = 0;
    }
    bitmap[food_x] = 1 << food_y;
    bitmap[head->x] = 1 << head->y;
}

boolean Snake::moveSnake(Direction direction)
{
    printf("direction: %d, head->x: %d, head->y: %d\n", direction, head->x, head->y);
    printf("direction: %d, tail->x: %d, tail->y: %d\n", direction, tail->x, tail->y);
    occupied[tail->x][tail->y] = false;
    switch (direction)
    {
    case UP:
        if (head->y == 7 || occupied[head->x][head->y + 1])
        {
            return false;
        }
        break;
    case DOWN:
        if (head->y == 0 || occupied[head->x][head->y - 1])
        {
            return false;
        }
        break;
    case LEFT:
        if (head->x == 0 || occupied[head->x - 1][head->y])
        {
            return false;
        }
        break;
    case RIGHT:
        if (head->x == (RAW_SNAKE_BITMAP_SIZE - 1) || occupied[head->x + 1][head->y])
        {
            return false;
        }
        break;
    }

    Node *temp = head;
    head = new Node(head->x, head->y);
    head->next = temp;
    head->next->prev = head;

    switch (direction)
    {
    case UP:
        head->y++;
        break;
    case DOWN:
        head->y--;
        break;
    case LEFT:
        head->x--;
        break;
    case RIGHT:
        head->x++;
        break;
    }
    if (grow)
    {
        grow = false;
    }
    else
    {
        bitmap[tail->x] &= ~(1 << tail->y);
        temp = tail;
        tail = tail->prev;
        tail->next = nullptr;
        delete temp;
    }
    bitmap[head->x] |= 1 << head->y;

    occupied[head->x][head->y] = true;

    return true;
}

uint8_t *Snake::getSnake()
{
    if (food_x == head->x && food_y == head->y)
    {
        while (occupied[food_x][food_y])
        {
            food_x = random(0, RAW_SNAKE_BITMAP_SIZE);
            food_y = random(0, 8);
        }
        bitmap[food_x] |= 1 << food_y;
        grow = true;
    }

    boolean success = false;
    if (food_x < head->x)
    {
        success = moveSnake(Direction::LEFT);
        if (success)
        {
            return bitmap;
        }
    }
    if (food_x > head->x)
    {
        success = moveSnake(Direction::RIGHT);
        if (success)
        {
            return bitmap;
        }
    }
    if (food_y < head->y)
    {
        success = moveSnake(Direction::DOWN);
        if (success)
        {
            return bitmap;
        }
    }
    if (food_y > head->y)
    {
        success = moveSnake(Direction::UP);
        if (success)
        {
            return bitmap;
        }
    }
    if (!success)
    {
        printf("failed to move to food\n");
        if (moveSnake(Direction::LEFT))
        {
            return bitmap;
        }
        if (moveSnake(Direction::RIGHT))
        {
            return bitmap;
        }
        if (moveSnake(Direction::DOWN))
        {
            return bitmap;
        }
        if (moveSnake(Direction::UP))
        {
            return bitmap;
        }

        printf("failed to move at all\n");
        while (head->next)
        {
            Node *temp = head;
            head = head->next;
            delete temp;
        }
        delete head;
        initBitmap();
    }

    return bitmap;
}
