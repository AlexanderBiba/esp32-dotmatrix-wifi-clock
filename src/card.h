#ifndef DISPLAY_CARD_H
#define DISPLAY_CARD_H

#include "main.h"

#define DEFAULT_SWITCH_TIME 10 * 1000

class Card
{
public:
    Card(OperationMode operationMode, long switchTime) : operationMode(operationMode), switchTime(switchTime){};
    Card(OperationMode operationMode) : operationMode(operationMode), switchTime(DEFAULT_SWITCH_TIME){};
    OperationMode getOperationMode() { return operationMode; };
    long getCardSwitchTime() { return switchTime; };

private:
    OperationMode operationMode;
    long switchTime;
};

#endif // DISPLAY_CARD_H