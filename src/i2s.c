#if !defined(I2S_H)
#include "i2s.h"
#define I2S_H
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

#if !defined(SOUND_EFFECTS_H)
#include "SoundEffects.h"
#define SOUND_EFFECTS_H
#endif

#if !defined(AUDIO_CONTROLS_H)
#include "AudioControls.h"
#define AUDIO_CONTROLS_H
#endif

#if !defined(LED_CONTROLS_H)
#include "LEDControls.h"
#define LED_CONTROLS_H
#endif

#if !defined(STDLIB_H)
#include <stdlib.h>
#define STDLIB_H
#endif

#if !defined(MEMORY_H)
#include <memory.h>
#define MEMORY_H
#endif


#define I2S_SAMPLE_HO(sample) ((sample & 0x00FFFF00) >> 8) // for 24-bit data, 32-bit container
#define I2S_SAMPLE_LO(sample) ((sample & 0x000000FF) << 8)

#define I2S_FLOAT_TO_UINT24(sample) ((uint32_t)(sample * UINT24_MAX))
#define I2S_UINT24_TO_FLOAT(sample) (((float)sample) / ((float)UINT24_MAX))

#define I2S_FLAGS_RECIEVE_HIGH_ORDER (flags & 1)
#define I2S_FLAGS_TRANSMIT_HIGH_ORDER ((flags >> 1) & 1)
#define I2S_FLAGS_TOGGLE_RECIEVE_ORDER() flags ^= 0b01
#define I2S_FLAGS_TOGGLE_TRANSMIT_ORDER() flags ^= 0b10
#define I2S_FLAGS_ENABLE_TRANSMIT() flags |= 0b100
#define I2S_FLAGS_TRANSMIT_ENABLED ((flags >> 2) & 1)

#define I2S3_REGISTERS ((SpiRegisters*)0x40003C00u)
#define I2S3EXT_REGISTERS ((SpiRegisters*)0x40004000u)




typedef volatile struct
{
    uint32_t CR1;
    uint32_t CR2;
    uint32_t SR;
    uint32_t DR;
    uint32_t CRCPR;
    uint32_t RXCRCR;
    uint32_t TXCRCR;
    uint32_t I2SCFGR;
    uint32_t I2SPR;
} SpiRegisters;



volatile uint8_t flags;
volatile float transmitSample;
volatile uint32_t recieveSample;
volatile AudioBuffer inputBuffer;
/*
     [0-4096) samples are the old (transmitted) samples, they are necessary for the chorus effect.
     [4096, 8192) samples are recieved and yet to be transmitted.
*/
volatile AudioBuffer outputBuffer; 
 

void I2S_InitBuffers();
void I2S_ConfigureRCC();
void I2S_ConfigureI2S3Registers();
void I2S_ConfigureGPIORegisters();
void I2S_ConfigurePLLI2S();
void I2S_ShiftOutputBuffer();
void I2S_HandleTransmit();
void I2S_HandleRecieve();

void InitializeI2S3()
{
    flags = 0b11;
    
    I2S_InitBuffers();
    I2S_ConfigureRCC();
    I2S_ConfigureGPIORegisters();
    I2S_ConfigureI2S3Registers();
    I2S_ConfigurePLLI2S();
    NVIC_ISER1 |= 1 << 19; // enable the SPI3 interrupt
    NVIC_IPR12 |= 0b00010000 << 24; // set the interrupt priority

    // enable the I2S peripheral
    I2S3EXT_REGISTERS->I2SCFGR |= 1 << 10;
    I2S3_REGISTERS->I2SCFGR |= 1 << 10;
}

void I2S_InitBuffers()
{
    const uint32_t inputBufferSize = INPUT_BUFFER_FRAME_COUNT * sizeof(float);
    inputBuffer.pData = (float*)malloc(inputBufferSize);
    if (inputBuffer.pData == NULL)
    {
        DEBUG_PRINT("Insufficient memory (input buffer)");
        CHANGE_LED_COLOR(LED_COLOR_ERROR);
        return;
    }
    memset(inputBuffer.pData, 0, inputBufferSize);
    inputBuffer.readIndex = 0;
    inputBuffer.writeIndex = 0;

    const uint32_t outputBufferSize = OUTPUT_BUFFER_FRAME_COUNT * sizeof(float);
    outputBuffer.pData = (float*)malloc(outputBufferSize);
    if (outputBuffer.pData == NULL)
    {
        DEBUG_PRINT("Insufficient memory (output buffer)");
        CHANGE_LED_COLOR(LED_COLOR_ERROR);
        return;
    }
    memset(outputBuffer.pData, 0, outputBufferSize);
    outputBuffer.readIndex = 4096;
    outputBuffer.writeIndex = 0;
}

void I2S_ConfigureRCC()
{
    RCC_AHB1ENR |= 0b11; // enable the GPIO A & B clock
    RCC_APB1ENR |= 0b1 << 15; // enable the SPI3 clock
}

void I2S_ConfigureI2S3Registers()
{
    I2S3_REGISTERS->CR2 |= 1 << 7; // enable the I2S3 transmit IRQ handling
    I2S3EXT_REGISTERS->CR2 |= 1 << 6; // enable the I2S3ext recieve IRQ handling

    I2S3_REGISTERS->I2SCFGR |= 0b1010 << 8; // select the I2S mode as master-transmit
    I2S3EXT_REGISTERS->I2SCFGR |= 0b1001 << 8; // select the I2S mode as slave-receive

    // steady state low, 24-bit data, 32 bit container, MSB (left) justified
    I2S3_REGISTERS->I2SCFGR |= 0b010011;
    I2S3EXT_REGISTERS->I2SCFGR |= 0b010011;

    // MCK enabled, odd and I2SDIV = 3 for 96kHz
    I2S3_REGISTERS->I2SPR |= 0b1100000011;
}

void I2S_ConfigureGPIORegisters()
{
    // set to alternate function mode
    GPIOA_REGISTERS->MODER |= 0b10 << 8;
    GPIOB_REGISTERS->MODER |= 0b101010 << 6;
    GPIOB_REGISTERS->MODER |= 0b10 << 20;

    // alternate function mapping
    GPIOA_REGISTERS->AFRL |= 0b0110 << 16;
    GPIOB_REGISTERS->AFRL |= 0b011001110110 << 12;
    GPIOB_REGISTERS->AFRH |= 0b0110 << 8;
}

void I2S_ConfigurePLLI2S()
{
    RCC_PLLI2SCFGR |= 16; // PLLI2SM, PLLM VCO = 1MHz
    RCC_PLLI2SCFGR &= ~(0b111111111 << 6); // clear the PLLI2SN
    RCC_PLLI2SCFGR |= 344 << 6; // PLLI2SN
    RCC_PLLI2SCFGR |= 2 << 28; // PLLI2SR
    RCC_CR |= 1 << 26; // Enable the PLLI2S
}

void SPI3_IRQHandler()
{
    if (((I2S3_REGISTERS->SR >> 1) & 1))
    {
        I2S_HandleTransmit();
    }
    if (I2S3EXT_REGISTERS->SR & 1) // recieve
    {
        I2S_HandleRecieve();
    }
}

void I2S_ShiftOutputBuffer()
{
    // shift the output buffer to the left by 1024 samples since the first 1024 samples are already transmitted and no longer needed
    memcpy(outputBuffer.pData, outputBuffer.pData + 1024, 7168);
    memset(outputBuffer.pData + 7168, 0, 1024);
}

void I2S_HandleTransmit()
{
    if (I2S_FLAGS_TRANSMIT_ENABLED)
    {
        if (outputBuffer.readIndex < 5120)
        {
            if (I2S_FLAGS_TRANSMIT_HIGH_ORDER)
            {
                if (!IsCleanMode())
                {
                    transmitSample = SFX_Chorus(&outputBuffer);
                    transmitSample = SFX_Overdrive(transmitSample);
                    transmitSample = SFX_Distortion(transmitSample);
                }
                transmitSample *= audioControls.volume;
                I2S3_REGISTERS->DR = I2S_SAMPLE_HO(I2S_FLOAT_TO_UINT24(transmitSample));
            }
            else
            {
                I2S3_REGISTERS->DR = I2S_SAMPLE_LO(I2S_FLOAT_TO_UINT24(transmitSample));
                outputBuffer.readIndex++;
            }
        }
        else
        {
            I2S3_REGISTERS->DR = 0;
            DEBUG_PRINT("Output buffer read index is greater than or equal to 5120!");
            CHANGE_LED_COLOR(LED_COLOR_ERROR);
        }
    }
    else
    {
        I2S3_REGISTERS->DR = 0;
    }
    I2S_FLAGS_TOGGLE_TRANSMIT_ORDER();
}

void I2S_HandleRecieve()
{
    if (I2S_FLAGS_RECIEVE_HIGH_ORDER)
    {
        recieveSample = (I2S3EXT_REGISTERS->DR << 8) & 0x00FFFF00;
    }
    else
    {
        recieveSample |= (I2S3EXT_REGISTERS->DR >> 8) & 0x000000FF;
        inputBuffer.pData[inputBuffer.writeIndex] = I2S_UINT24_TO_FLOAT(recieveSample);
        inputBuffer.writeIndex = (inputBuffer.writeIndex + 1 == INPUT_BUFFER_FRAME_COUNT) ? (0) : (inputBuffer.writeIndex + 1); 
        if (I2S_FLAGS_TRANSMIT_ENABLED)
        {
            switch (inputBuffer.writeIndex)
            {
                case 0:         // 3072-7168
                    I2S_ShiftOutputBuffer();
                    SFX_Equalizer(&inputBuffer, &outputBuffer);
                    inputBuffer.readIndex = 4096;
                    outputBuffer.readIndex = 4096;
                    break;
                case 1024:      // 4096-1024
                    I2S_ShiftOutputBuffer();
                    SFX_Equalizer(&inputBuffer, &outputBuffer);
                    inputBuffer.readIndex = 5120;
                    outputBuffer.readIndex = 4096;
                    break;
                case 2048:      // 5120-2048
                    I2S_ShiftOutputBuffer();
                    SFX_Equalizer(&inputBuffer, &outputBuffer);
                    inputBuffer.readIndex = 6144;
                    outputBuffer.readIndex = 4096;
                    break;
                case 3072:      // 6144-3072
                    I2S_ShiftOutputBuffer();
                    SFX_Equalizer(&inputBuffer, &outputBuffer);
                    inputBuffer.readIndex = 0;
                    outputBuffer.readIndex = 4096;
                    break;
                case 4096:      // 0-4096
                    I2S_ShiftOutputBuffer();
                    SFX_Equalizer(&inputBuffer, &outputBuffer);
                    inputBuffer.readIndex = 1024;
                    outputBuffer.readIndex = 4096;
                    break;
                case 5120:      // 1024-5120
                    I2S_ShiftOutputBuffer();
                    SFX_Equalizer(&inputBuffer, &outputBuffer);
                    inputBuffer.readIndex = 2048;
                    outputBuffer.readIndex = 4096;
                    break;
                case 6144:      // 2048-6144
                    I2S_ShiftOutputBuffer();
                    SFX_Equalizer(&inputBuffer, &outputBuffer);
                    inputBuffer.readIndex = 3072;
                    outputBuffer.readIndex = 4096;
                    break;
                default:
                    break;
            }
        }
        else
        {
            if (inputBuffer.writeIndex == 4096) // wait for the first FFT_SIZE of samples to be recieved, then start applying EQ every 1024 samples recieved
            {
               I2S_FLAGS_ENABLE_TRANSMIT(); 
               SFX_Equalizer(&inputBuffer, &outputBuffer);
               inputBuffer.readIndex = 1024;
            }
        }
    }   
    I2S_FLAGS_TOGGLE_RECIEVE_ORDER();
}