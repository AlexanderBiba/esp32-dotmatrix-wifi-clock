#ifndef APP_MAIN_H
#define APP_MAIN_H

#define BOTTOM_ALIGN 1

#define REQUEST_BUFFER_SIZE 255

enum struct OperationMode
{
    CLOCK,         // Index 0
    DATE,          // Index 1
    WEATHER,       // Index 2
    SNAKE,         // Index 3
    MESSAGE,       // Index 4
    IP_ADDRESS,    // Index 5 - Renamed from SYSTEM_STATUS
    RAIN,          // Index 6
    OPERATION_MODE_COUNT
};

#define OPERATION_MODE_LENGTH static_cast<int>(OperationMode::OPERATION_MODE_COUNT)

const char OperationModeStr[OPERATION_MODE_LENGTH][32] = {
    "CLOCK",
    "DATE",
    "WEATHER",
    "SNAKE",
    "MESSAGE",
    "IP", // Renamed from SYSTEM
    "RAIN"};

#endif