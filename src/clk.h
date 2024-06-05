#ifndef CLK_H
#define CLK_H

#define TIME_BUFFER_SIZE sizeof("hh:mm:ss")

void setupClk();
bool getTime(char timeBuffer[TIME_BUFFER_SIZE]);

#endif