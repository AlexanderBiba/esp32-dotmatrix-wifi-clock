#ifndef __RAIN_H__
#define __RAIN_H__

#include <Arduino.h>
#include "settings.h"

#define RAW_RAIN_BITMAP_SIZE 32

class Rain
{
public:
    Rain(AppSettings *settings) : settings(settings) { initRain(); };
    ~Rain() { cleanupRain(); };
    uint8_t *getRain();

private:
    AppSettings *settings;
    uint8_t bitmap[RAW_RAIN_BITMAP_SIZE];
    
    // Rain drop structure
    struct RainDrop {
        uint8_t x;        // Column position
        int8_t y;         // Row position (can be negative for off-screen drops)
        uint8_t length;    // Length of the drop
        uint8_t speed;     // Speed of falling (frames per pixel)
        uint8_t counter;   // Counter for speed control
    };
    
    static const uint8_t MAX_DROPS = 16;  // High density - no blank screens
    RainDrop drops[MAX_DROPS];
    uint8_t activeDrops;
    
    void initRain();
    void cleanupRain();
    void updateRain();
    void addNewDrop();
    void removeDrop(uint8_t index);
    void clearBitmap();
};

#endif // __RAIN_H__
