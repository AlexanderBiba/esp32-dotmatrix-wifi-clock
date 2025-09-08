#include "rain.h"

void Rain::initRain()
{
    clearBitmap();
    activeDrops = 0;
    
    // Initialize all drops as inactive
    for (uint8_t i = 0; i < MAX_DROPS; i++) {
        drops[i].x = 0;
        drops[i].y = 0;
        drops[i].length = 0;
        drops[i].speed = 0;
        drops[i].counter = 0;
    }
    
    // Add initial drops - start with more for immediate coverage
    for (uint8_t i = 0; i < 8; i++) {
        addNewDrop();
    }
}

void Rain::cleanupRain()
{
    clearBitmap();
    activeDrops = 0;
}

void Rain::clearBitmap()
{
    for (uint8_t i = 0; i < RAW_RAIN_BITMAP_SIZE; i++) {
        bitmap[i] = 0;
    }
}

void Rain::addNewDrop()
{
    if (activeDrops >= MAX_DROPS) {
        return; // No space for new drops
    }
    
    // Find an inactive drop slot
    for (uint8_t i = 0; i < MAX_DROPS; i++) {
        if (drops[i].length == 0) {
            // Initialize new drop - start above the visible area
            drops[i].x = random(0, RAW_RAIN_BITMAP_SIZE);
            drops[i].y = -random(2, 5); // Start above screen (negative y)
            drops[i].length = random(2, 6); // Drop length between 2-5 pixels for denser look
            drops[i].speed = random(1, 3); // Speed between 1-2 (frames per pixel)
            drops[i].counter = 0;
            activeDrops++;
            break;
        }
    }
}

void Rain::removeDrop(uint8_t index)
{
    if (index >= MAX_DROPS || drops[index].length == 0) {
        return;
    }
    
    // Clear the drop from bitmap - only the visible portion
    for (uint8_t i = 0; i < drops[index].length; i++) {
        int8_t y = drops[index].y + i;
        if (y >= 0 && y < 8) { // Only clear if within visible area
            bitmap[drops[index].x] &= ~(1 << y);
        }
    }
    
    // Reset drop
    drops[index].x = 0;
    drops[index].y = 0;
    drops[index].length = 0;
    drops[index].speed = 0;
    drops[index].counter = 0;
    activeDrops--;
}

void Rain::updateRain()
{
    clearBitmap();
    
    // Update each active drop
    for (uint8_t i = 0; i < MAX_DROPS; i++) {
        if (drops[i].length == 0) {
            continue; // Skip inactive drops
        }
        
        // Increment counter
        drops[i].counter++;
        
        // Check if it's time to move the drop
        if (drops[i].counter >= drops[i].speed) {
            drops[i].counter = 0;
            drops[i].y++; // Move down
            
            // Check if drop has fallen off the bottom
            if (drops[i].y >= 8) {
                removeDrop(i);
                continue;
            }
        }
        
        // Draw the drop on the bitmap - only the visible portion
        for (uint8_t j = 0; j < drops[i].length; j++) {
            int8_t y = drops[i].y + j;
            if (y >= 0 && y < 8) { // Only draw if within visible area
                bitmap[drops[i].x] |= (1 << y);
            }
        }
    }
    
    // Aggressively add new drops to maintain high density
    if (random(0, 100) < 35 && activeDrops < MAX_DROPS) { // 35% chance each frame
        addNewDrop();
    }
    
    // If we're running low on drops, be even more aggressive
    if (activeDrops < MAX_DROPS / 2 && random(0, 100) < 60) { // 60% chance if less than half capacity
        addNewDrop();
    }
}

uint8_t *Rain::getRain()
{
    updateRain();
    return bitmap;
}
