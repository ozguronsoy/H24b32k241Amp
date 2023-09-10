#include "i2s.h"
#include "rcc.h"


uint32_t ConfigurePLLI2S()
{
    DEBUG_PRINT("I2S: Configuring the PLLI2S\n");

    RCC_PLLI2SCFGR |= 16;                  // PLLI2SM, PLLM VCO = 1MHz
    RCC_PLLI2SCFGR &= ~(0b111111111 << 6); // clear the PLLI2SN
    RCC_PLLI2SCFGR &= ~(0b111 << 28);      // clear the PLLI2SR
    RCC_PLLI2SCFGR |= 233 << 6;            // PLLI2SN
    RCC_PLLI2SCFGR |= 2 << 28;             // PLLI2SR
    RCC_CR |= 1 << 26;                     // Enable the PLLI2S

    return RESULT_SUCCESS;
}