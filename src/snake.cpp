#include "snake.h"

void Snake::initBitmap()
{
    memset(occupied, false, sizeof(occupied));
    
    // Initialize food position first
    food_x = random(0, RAW_SNAKE_BITMAP_SIZE);
    food_y = random(0, 8);
    
    // Initialize head position, ensuring it's different from food
    uint8_t head_x = random(0, RAW_SNAKE_BITMAP_SIZE);
    uint8_t head_y = random(0, 8);
    while (head_x == food_x && head_y == food_y)
    {
        head_x = random(0, RAW_SNAKE_BITMAP_SIZE);
        head_y = random(0, 8);
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

void Snake::cleanupSnake()
{
    Node* current = head;
    while (current != nullptr)
    {
        Node* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
    tail = nullptr;
}

boolean Snake::moveSnake(Direction direction)
{
    Serial.printf("direction: %d, head->x: %d, head->y: %d\n", direction, head->x, head->y);
    Serial.printf("direction: %d, tail->x: %d, tail->y: %d\n", direction, tail->x, tail->y);
    
    // Check bounds and collisions first
    uint8_t new_x = head->x;
    uint8_t new_y = head->y;
    
    switch (direction)
    {
    case UP:
        if (head->y == 7 || occupied[head->x][head->y + 1])
        {
            return false;
        }
        new_y = head->y + 1;
        break;
    case DOWN:
        if (head->y == 0 || occupied[head->x][head->y - 1])
        {
            return false;
        }
        new_y = head->y - 1;
        break;
    case LEFT:
        if (head->x == 0 || occupied[head->x - 1][head->y])
        {
            return false;
        }
        new_x = head->x - 1;
        break;
    case RIGHT:
        if (head->x == (RAW_SNAKE_BITMAP_SIZE - 1) || occupied[head->x + 1][head->y])
        {
            return false;
        }
        new_x = head->x + 1;
        break;
    }

    // Create new head
    Node *newHead = new Node(new_x, new_y);
    newHead->next = head;
    if (head) {
        head->prev = newHead;
    }
    head = newHead;

    // Update bitmap and occupied array
    bitmap[head->x] |= 1 << head->y;
    occupied[head->x][head->y] = true;

    if (grow)
    {
        grow = false;
    }
    else
    {
        // Remove tail
        bitmap[tail->x] &= ~(1 << tail->y);
        occupied[tail->x][tail->y] = false;
        
        if (tail->prev) {
            tail->prev->next = nullptr;
            Node* oldTail = tail;
            tail = tail->prev;
            delete oldTail;
        }
    }

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
        Serial.println("failed to move to food");
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

        Serial.println("failed to move at all");
        // Clean up and restart
        cleanupSnake();
        initBitmap();
    }

    return bitmap;
}
