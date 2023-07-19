#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

#if defined(DEBUG)

#define LED_COLOR_SUCCESS 1 // green
#define LED_COLOR_ERROR 0 // red
#define INIT_LED InitializeLED()
#define CHANGE_LED_COLOR(color) ChangeLEDColor(color)

/*
    PB12 - Error (Red)
    PB13 - Success (Green)
*/
void InitializeLED();
void ChangeLEDColor(uint8_t color);

#else

#define LED_COLOR_SUCCESS
#define LED_COLOR_ERROR
#define INIT_LED
#define CHANGE_LED_COLOR(color) 

#endif