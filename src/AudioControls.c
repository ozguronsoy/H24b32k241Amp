#if !defined(AUDIO_CONTROLS_H)
#include "AudioControls.h"
#define AUDIO_CONTROLS_H
#endif

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

#if !defined(MEMORY_H)
#include <memory.h>
#define MEMORY_H
#endif


#define ADC_MAX 1023.0f // 10-bit ADC
#define ADC_NORMALIZED_VALUE (((float)ADC1_REGISTERS->DR) / ADC_MAX)
#define ADC1_REGISTERS ((AdcRegisters*)0x40012000u)

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

AudioControlsStruct audioControls;

void ADC_IRQHandler();
inline void ConfigureAdcRegisters();
inline void ConfigureGPIORegisters();

void InitializeAudioControls()
{
    audioControls.bass = 1.0f;
    audioControls.low_mid = 1.0f;
    audioControls.high_mid = 1.0f;
    audioControls.treble = 1.0f;
    audioControls.volume = 1.0f;
    audioControls.distortion = 0.0f;
    audioControls.overdrive = 0.0f;
    audioControls.reverb = 0.0f;
    audioControls.chorus = 0.0f;

    RCC_APB2ENR |= 1 << 8; // enable the ADC1 clock

    ConfigureAdcRegisters();
    ConfigureGPIORegisters();
    NVIC_ISER0 |= 1 << 18; // enable the ADC IRQ handling

    ADC1_REGISTERS->CR2 |= 1; // ADON
    
    int i;
    for(i = 0; i < 2500000; i++); // wait for ADC to stabilize
    
    ADC1_REGISTERS->CR2 |= 1 << 30; // start conversion
}

uint32_t IsCleanMode()
{
    return (GPIOB_REGISTERS->IDR >> 14) & 1;
}

inline void ConfigureAdcRegisters()
{
    ADC1_REGISTERS->CR1 |= 0b01 << 24; // 10-bit res
    ADC1_REGISTERS->CR1 |= 0b0001 << 5; // scan mode enabled, EOC interrupt enabled
    ADC1_REGISTERS->CR2 &= ~(0b11 << 10); // right alignment, EOC bit is set at the end of each regular conversion
    
    ADC1_REGISTERS->SMPR2 |= 0b111111111111000111111111000; // sampling time = 480 cycles 

    ADC1_REGISTERS->SQR3 = 0;
    
    ADC1_REGISTERS->CCR |= 0b11 << 16; // PCLCK2 / 8
}

inline void ConfigureGPIORegisters()
{
    RCC_AHB1ENR |= 0b11; // enable the GPIO A & B clock
    GPIOA_REGISTERS->MODER |= 0b1111110011111111; // set PA0, PA1, PA2, PA3, PA5, PA6, and PA7 as analogue mode
    GPIOB_REGISTERS->MODER |= 0b1111; // set PB0 and PB1 as analogue mode
}

void ADC_IRQHandler()
{    
    // conversion order: PA0-PA1-PA2-PA3-PA5-PA6-PA7-PB0-PB1 (CH0-CH1-CH2-CH3-CH5-CH6-CH7-CH8-CH9)
    switch (ADC1_REGISTERS->SQR3)
    {
    case 0:
        audioControls.bass = ADC_NORMALIZED_VALUE;
        ADC1_REGISTERS->SQR3 = 1;
        break;
    case 1:
        audioControls.low_mid = ADC_NORMALIZED_VALUE;
        ADC1_REGISTERS->SQR3 = 2;
        break;
    case 2:
        audioControls.high_mid = ADC_NORMALIZED_VALUE;
        ADC1_REGISTERS->SQR3 = 3;
        break;
    case 3:
        audioControls.treble = ADC_NORMALIZED_VALUE;
        ADC1_REGISTERS->SQR3 = 5;
        break;
    case 5:
        audioControls.volume = ADC_NORMALIZED_VALUE;
        ADC1_REGISTERS->SQR3 = 6;
        break;
    case 6:
        audioControls.distortion = ADC_NORMALIZED_VALUE;
        ADC1_REGISTERS->SQR3 = 7;
        break;
    case 7:
        audioControls.overdrive = ADC_NORMALIZED_VALUE;
        ADC1_REGISTERS->SQR3 = 8;
        break;
    case 8:
        audioControls.reverb = ADC_NORMALIZED_VALUE;
        ADC1_REGISTERS->SQR3 = 9;
        break;
    case 9:
        audioControls.chorus = ADC_NORMALIZED_VALUE;
        ADC1_REGISTERS->SQR3 = 0;
        break;
    default:
        break;
    }

    ADC1_REGISTERS->SR &= ~(0b10); // clear EOC
    ADC1_REGISTERS->CR2 |= 1 << 30; // start the next conversion
}