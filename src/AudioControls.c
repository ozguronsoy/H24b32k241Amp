#include "AudioControls.h"

#if !defined(RCC_H)
#include "rcc.h"
#define RCC_H
#endif

#if !defined(GPIO_H)
#include "gpio.h"
#define GPIO_H
#endif

#if !defined(NVIC_H)
#include "nvic.h"
#define NVIC_H
#endif

#if !defined(DMA_H)
#include "dma.h"
#define DMA_H
#endif

#if !defined(MEMORY_H)
#include <memory.h>
#define MEMORY_H
#endif

#if !defined(MATH_H)
#include <math.h>
#define MATH_H
#endif

#define ADC1_REGISTERS ((AdcRegisters *)0x40012000u)

typedef volatile struct
{
    uint32_t SR;
    uint32_t CR1;
    uint32_t CR2;
    uint32_t SMPR1;
    uint32_t SMPR2;
    uint32_t JOFR1;
    uint32_t JOFR2;
    uint32_t JOFR3;
    uint32_t JOFR4;
    uint32_t HTR;
    uint32_t LTR;
    uint32_t SQR1;
    uint32_t SQR2;
    uint32_t SQR3;
    uint32_t JSQR;
    uint32_t JDR1;
    uint32_t JDR2;
    uint32_t JDR3;
    uint32_t JDR4;
    uint32_t DR;
    uint32_t CCR;
} AdcRegisters;

AudioControlsStruct audioControls = {ADC_MAX, ADC_MAX, ADC_MAX, ADC_MAX, ADC_MAX, 0u, 0u, 0u, 0u};

void ADC_ConfigureAdcRegisters();
void ADC_ConfigureGPIORegisters();
void ADC_ConfigureDmaRegisters();

uint32_t InitializeAudioControls()
{
    DEBUG_PRINT("AUDIO CONTROLS: Initializing...\n");

    RCC_APB2ENR |= 1 << 8;  // enable the ADC1 clock
    RCC_AHB1ENR |= 1 << 22; // enable the DMA2 clock

    ADC_ConfigureGPIORegisters();
    ADC_ConfigureAdcRegisters();
    ADC_ConfigureDmaRegisters();

    NVIC_ISER0 |= 1 << 18;         // enable the ADC IRQ handling (for Overrun)
    NVIC_IPR4 |= 0b11110000 << 24; // set the interrupt priority

    ADC1_REGISTERS->CR2 |= 1;       // ADON
    ADC1_REGISTERS->CR2 |= 1 << 30; // start conversion

    DEBUG_PRINT("AUDIO CONTROLS: initialized successfully\n");

    return RESULT_SUCCESS;
}

uint32_t IsCleanMode()
{
    return (GPIOB_REGISTERS->IDR >> 14) & 1;
}

void ADC_ConfigureAdcRegisters()
{
    DEBUG_PRINT("AUDIO CONTROLS: Configuring the ADC1 registers\n");

    ADC1_REGISTERS->CR1 |= 1 << 26; // overrun interrupt
    ADC1_REGISTERS->CR1 |= 1 << 8;  // scan mode

    ADC1_REGISTERS->CR2 |= 0b11 << 8; // DMA & DDS
    ADC1_REGISTERS->CR2 |= 0b10;      // continuous mode

    ADC1_REGISTERS->SMPR2 |= 0b111111111111111000111111111111; // sampling time = 480 cycles

    ADC1_REGISTERS->CCR |= 0b11 << 16; // PCLCK2 / 8

    // 5 bits for each conversion
    ADC1_REGISTERS->SQR3 = 0b001100010100011000100000100000; // CH0-CH1-CH2-CH3-CH5-CH6
    ADC1_REGISTERS->SQR2 = 0b010010100000111;                // CH7-CH8-CH9
    ADC1_REGISTERS->SQR1 = 8 << 20;                          // 9 conversions will occur
}

void ADC_ConfigureGPIORegisters()
{
    DEBUG_PRINT("AUDIO CONTROLS: Configuring the GPIO registers\n");

    GPIOA_REGISTERS->MODER |= 0b1111110011111111; // set PA0, PA1, PA2, PA3, PA5, PA6, and PA7 as analogue mode
    GPIOB_REGISTERS->MODER |= 0b1111;             // set PB0 and PB1 as analogue mode
}

void ADC_ConfigureDmaRegisters()
{
    DEBUG_PRINT("AUDIO CONTROLS: Configuring the DMA registers\n");

    // Memory data size = half-word (16-bit), Peripheral data size = half-word
    // Memory increment mode enabled
    DMA2_REGISTERS->S0.CR |= 0b01011 << 10;
    DMA2_REGISTERS->S0.CR &= ~(0b11 << 6); // data transfer direction = P2M
    DMA2_REGISTERS->S0.CR |= 1 << 8;       // circular mode
    DMA2_REGISTERS->S0.NDTR = 9;
    DMA2_REGISTERS->S0.PAR = (uint32_t)&ADC1_REGISTERS->DR;
    DMA2_REGISTERS->S0.M0AR = (uint32_t)&audioControls;
    DMA2_REGISTERS->S0.CR |= 1; // enable the DMA
}

void ADC_IRQHandler()
{
    if ((ADC1_REGISTERS->SR >> 5) & 1)
    {
        DEBUG_PRINT("ADC OVERRUN OCCURRED!\n");

        DMA2_REGISTERS->S0.CR &= ~1;

        DMA2_REGISTERS->S0.M0AR = (uint32_t)&audioControls;
        DMA2_REGISTERS->S0.NDTR = 9;

        ADC1_REGISTERS->CR2 &= ~(1 << 8); // enable DMA
        ADC1_REGISTERS->CR2 |= 1 << 8;

        ADC1_REGISTERS->SR &= ~(1 << 5);

        DMA2_REGISTERS->S0.CR |= 1;     // enable the DMA
        ADC1_REGISTERS->CR2 |= 1 << 30; // start conversion
    }
}