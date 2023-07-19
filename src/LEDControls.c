#if !defined(LED_CONTROLS_H)
#include "LEDControls.h"
#define LED_CONTROLS_H
#endif

#if !defined(RCC_H)
#include "rcc.h"
#define RCC_H
#endif

#if !defined(GPIO_H)
#include "gpio.h"
#define GPIO_H
#endif

void InitializeLED()
{
    RCC_AHB1ENR |= 0b10; // enable the GPIOB clock
    GPIOB_REGISTERS->MODER |= 0b0101 << 24; // set PB12 and PB13 as output
    GPIOB_REGISTERS->ODR |= 0b1 << 13; // set the initial LED color to green
}

void ChangeLEDColor(uint8_t color)
{
    switch (color)
    {
    case LED_COLOR_ERROR:
        GPIOB_REGISTERS->ODR &= ~(0b1 << 13);
        GPIOB_REGISTERS->ODR |= 0b1 << 12;
        break;
    case LED_COLOR_SUCCESS:
        GPIOB_REGISTERS->ODR &= ~(0b1 << 12);
        GPIOB_REGISTERS->ODR |= 0b1 << 13;
        break;
    default:
        DEBUG_PRINT("Invalid LED color");
        break;
    }
}