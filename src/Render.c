#include "Render.h"

#if !defined(SPI_H)
#include "spi.h"
#define SPI_H
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

#if !defined(STDLIB_H)
#include <stdlib.h>
#define STDLIB_H
#endif

#if !defined(MEMORY_H)
#include <memory.h>
#define MEMORY_H
#endif

#define I2S3_REGISTERS ((SpiRegisters *)0x40003C00u)

volatile uint8_t enableTransmit = 0;
volatile uint8_t transmitOrder = HIGH;
/*
     [0-4096) samples are the old (transmitted) samples, they are necessary for the chorus effect.
     [4096, 8192) samples are received and yet to be transmitted.
*/
volatile AudioBuffer renderBuffer;
volatile uint32_t transmitSample;

uint32_t Render_InitBuffer()
{
    DEBUG_PRINT("RENDER: Initializing the render buffer\n");
    const uint32_t renderBufferSize = RENDER_BUFFER_FRAME_COUNT * sizeof(float);
    renderBuffer.pData = (float *)malloc(renderBufferSize);
    if (renderBuffer.pData == NULL)
    {
        DEBUG_PRINT("Insufficient memory (render buffer)\n");
        return RESULT_FAIL;
    }
    memset(renderBuffer.pData, 0, renderBufferSize);
    renderBuffer.index = 4096;

    return RESULT_SUCCESS;
}

void Render_ConfigureRCC()
{
    DEBUG_PRINT("RENDER: Configuring the RCC for I2S3\n");
    RCC_APB1ENR |= 0b1 << 15; // enable the SPI3 clock
}

void Render_ConfigureGPIORegisters()
{
    DEBUG_PRINT("RENDER: Configuring the I2S3 GPIO registers\n");

    // set to alternate function mode
    GPIOA_REGISTERS->MODER |= 0b10 << 8;
    GPIOB_REGISTERS->MODER |= 0b100010 << 6;
    GPIOB_REGISTERS->MODER |= 0b10 << 20;

    // alternate function mapping
    GPIOA_REGISTERS->AFRL |= 0b0110 << 16;
    GPIOB_REGISTERS->AFRL |= 0b011000000110 << 12;
    GPIOB_REGISTERS->AFRH |= 0b0110 << 8;
}

void Render_ConfigureI2SRegisters()
{
    DEBUG_PRINT("RENDER: Configuring the I2S3 registers\n");

    I2S3_REGISTERS->CR2 |= 1 << 7;          // enable the I2S3 transmit IRQ handling
    I2S3_REGISTERS->I2SCFGR |= 0b1010 << 8; // select the I2S mode as master-transmit
    I2S3_REGISTERS->I2SCFGR |= 0b010011;    // steady state low, 24-bit data, 32 bit container, MSB (left) justified
    I2S3_REGISTERS->I2SPR |= I2S_PR_VALUE;
}

uint32_t InitializeRender()
{
    DEBUG_PRINT("RENDER: initializing...\n");

    enableTransmit = 0;
    transmitOrder = HIGH;
    transmitSample = 0;

    if (Render_InitBuffer() == RESULT_FAIL)
    {
        return RESULT_FAIL;
    }

    Render_ConfigureRCC();
    Render_ConfigureGPIORegisters();
    Render_ConfigureI2SRegisters();

    NVIC_ISER1 |= 1 << 19;          // enable the SPI3 interrupt
    NVIC_IPR12 |= 0b00010000 << 24; // set the interrupt priority

    DEBUG_PRINT("RENDER: initialized successfully\n");

    return RESULT_SUCCESS;
}

void StartRendering()
{
    I2S3_REGISTERS->I2SCFGR |= 1 << 10; // enable the I2S peripheral
}

void SPI3_IRQHandler()
{
    // every 1024 sample (after 5119) the effects are applied and buffer is shifted 1024 samples to the left,
    // hence render the last sample after 5120 while waiting for the next 1024 samples
    if (enableTransmit && renderBuffer.index < 5120)
    {
        if (transmitOrder == HIGH)
        {
            transmitSample = FLOAT_TO_UINT24(renderBuffer.pData[renderBuffer.index]);
            I2S3_REGISTERS->DR = SAMPLE_HO(transmitSample);
        }
        else
        {
            I2S3_REGISTERS->DR = SAMPLE_LO(transmitSample);
            renderBuffer.index++;
        }
    }
    else
    {
        I2S3_REGISTERS->DR = (transmitOrder == HIGH) ? SAMPLE_HO(transmitSample) : SAMPLE_LO(transmitSample);
    }
    transmitOrder ^= 1;
}