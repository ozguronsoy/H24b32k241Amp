#include "rcc.h"
#include "framework.h"

#define RCC_PLLCFGR (*(uint32_t *)(0x40023800u + 0x04u))
#define RCC_CFGR (*(uint32_t *)(0x40023800u + 0x08u))
#define FLASH_ACR (*(uint32_t *)0x40023C00u)

void RCC_ConfigureSystemClock()
{
    DEBUG_PRINT("RCC: configuring the system clock\n");

    RCC_PLLCFGR |= 16;            // PLLM = 16
    RCC_PLLCFGR |= 200 << 6;      // PLLN = 200
    RCC_PLLCFGR &= ~(0b11 << 16); // PLLP = 2

    RCC_CR |= 1 << 24; // activate the PLL
    volatile uint32_t rcc_cr = RCC_CR;

    FLASH_ACR |= 3; // set the wait state to 3
    volatile uint32_t acr = FLASH_ACR; // "Check that the new number of wait states is taken into account to access the Flash memory by reading the FLASH_ACR register"

    RCC_CFGR |= 0b10;        // use PLL as system clock
    RCC_CFGR |= 0b100 << 10; // APB1 prescaler = 2 (50MHz max)
    volatile uint32_t rcc_cfgr = RCC_CFGR;

    DEBUG_PRINT("RCC: system clock configured successfully\n");
}