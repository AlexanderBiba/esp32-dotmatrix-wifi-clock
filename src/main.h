#ifndef APP_MAIN_H
#define APP_MAIN_H

#define BOTTOM_ALIGN 1

#define REQUEST_BUFFER_SIZE 255

enum struct OperationMode
{
    CLOCK,         // Index 0
    DATE,          // Index 1
    WEATHER,       // Index 2
    MESSAGE,       // Index 3
    COUNTDOWN,     // Index 4
    SNAKE,         // Index 5
    RAIN,          // Index 6
    IP_ADDRESS,    // Index 7 - Renamed from SYSTEM_STATUS
    OPERATION_MODE_COUNT
};

#define OPERATION_MODE_LENGTH static_cast<int>(OperationMode::OPERATION_MODE_COUNT)

const char OperationModeStr[OPERATION_MODE_LENGTH][32] = {
    "CLOCK",
    "DATE",
    "WEATHER",
    "MESSAGE",
    "COUNTDOWN",
    "SNAKE",
    "RAIN",
    "IP"}; // Renamed from SYSTEM

#endif