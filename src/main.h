#ifndef APP_MAIN_H
#define APP_MAIN_H

#define BOTTOM_ALIGN 1

#define REQUEST_BUFFER_SIZE 255

enum struct OperationMode
{
    MESSAGE,
    CLOCK,
    DATE,
    WEATHER,
    SNAKE,
    OPERATION_MODE_COUNT // This needs to be the last element because it is used to determine the number of elements in the enum
};

#define OPERATION_MODE_LENGTH static_cast<int>(OperationMode::OPERATION_MODE_COUNT)

const char OperationModeStr[OPERATION_MODE_LENGTH][8] = {
    "MESSAGE",
    "CLOCK",
    "DATE",
    "WEATHER",
    "SNAKE"};

#endif