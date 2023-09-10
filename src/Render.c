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

#if !defined(DMA_H)
#include "dma.h"
#define DMA_H
#endif

#if !defined(AUDIO_CONTROLS_H)
#include "AudioControls.h"
#define AUDIO_CONTROLS_H
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

volatile AudioBuffer processBuffer;

/*
    -DMA transfer buffer (double-bufferd).

    -[0, FFT_STEP_SIZE) is the first buffer.

    -[FFT_STEP_SIZE, 2 * FFT_STEP_SIZE) is the second buffer.

    -The index parameter is used to indicate which buffer is the current target for the DMA.
*/
volatile AudioBuffer renderBuffer;

uint32_t Render_InitBuffer()
{
    DEBUG_PRINT("RENDER: Initializing the process buffer\n");
    uint32_t bufferSize = PROCESS_BUFFER_FRAME_COUNT * sizeof(uint32_t);
    processBuffer.pData = (uint32_t *)malloc(bufferSize);
    if (processBuffer.pData == NULL)
    {
        DEBUG_PRINT("Insufficient memory (process buffer)\n");
        return RESULT_FAIL;
    }
    memset(processBuffer.pData, 0, bufferSize);
    processBuffer.index = 0;

    DEBUG_PRINT("RENDER: Initializing the render buffer\n");
    bufferSize = RENDER_BUFFER_FRAME_COUNT * sizeof(uint32_t) * 2;
    renderBuffer.pData = (uint32_t *)malloc(bufferSize);
    if (renderBuffer.pData == NULL)
    {
        DEBUG_PRINT("Insufficient memory (render buffer)\n");
        return RESULT_FAIL;
    }
    memset(renderBuffer.pData, 0, bufferSize);
    renderBuffer.index = RENDER_BUFFER_FRAME_COUNT; // start index of the available buffer (currently not targeted by the DMA)

    return RESULT_SUCCESS;
}

void Render_ConfigureRCC()
{
    DEBUG_PRINT("RENDER: Configuring the RCC for I2S3\n");
    RCC_APB1ENR |= 1 << 15; // enable the SPI3 clock
    RCC_AHB1ENR |= 1 << 21; // enable the DMA1 clock
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

    I2S3_REGISTERS->I2SCFGR |= 0b1010 << 8; // select the I2S mode as master-transmit
    I2S3_REGISTERS->I2SCFGR |= 0b010011;    // steady state low, 24-bit data, 32 bit container, MSB (left) justified
    I2S3_REGISTERS->I2SPR = 0b1000001110;   // MCK enabled, even and I2SDIV = 13
    I2S3_REGISTERS->CR2 |= 0b10;            // Transmit DMA enabled
}

void Render_ConfigureDmaRegisters()
{
    DEBUG_PRINT("RENDER: Configuring the DMA registers\n");

    // Memory data size = half-word (16-bit), Peripheral data size = half-word
    // Memory increment mode enabled
    DMA1_REGISTERS->S5.CR |= 0b01011 << 10;
    DMA1_REGISTERS->S5.CR |= 0b111 << 16;                    // Double buffer mode, very high priority
    DMA1_REGISTERS->S5.CR |= 0b101 << 6;                     // circular mode, data direction = M2P
    DMA1_REGISTERS->S5.CR |= 1 << 4;                         // transfer complete interrupt
    DMA1_REGISTERS->S5.NDTR = 2 * RENDER_BUFFER_FRAME_COUNT; // since the I2S container size is 32 bits and one transfer is 16 bits, have to make (2 * buffer_size) transfers
    DMA1_REGISTERS->S5.PAR = (uint32_t)&I2S3_REGISTERS->DR;
    DMA1_REGISTERS->S5.M0AR = (uint32_t)renderBuffer.pData;
    DMA1_REGISTERS->S5.M1AR = (uint32_t)(renderBuffer.pData + RENDER_BUFFER_FRAME_COUNT);
    DMA1_REGISTERS->S5.CR |= 1; // enable the DMA
}

uint32_t InitializeRender()
{
    DEBUG_PRINT("RENDER: initializing...\n");

    if (Render_InitBuffer() == RESULT_FAIL)
    {
        return RESULT_FAIL;
    }

    Render_ConfigureRCC();
    Render_ConfigureGPIORegisters();
    Render_ConfigureI2SRegisters();
    Render_ConfigureDmaRegisters();

    NVIC_ISER0 |= 1 << 16; // enable DMA1_Stream5 IRQ handler

    DEBUG_PRINT("RENDER: initialized successfully\n");

    return RESULT_SUCCESS;
}

void DeinitializeRender()
{
    DEBUG_PRINT("RENDER: Deinitializing\n");

    NVIC_ISER0 &= ~(1 << 16);
    DMA1_REGISTERS->S5.CR &= ~1;
    I2S3_REGISTERS->I2SCFGR &= ~(1 << 10);
    RCC_APB1ENR &= ~(1 << 15);
    RCC_AHB1ENR &= ~(1 << 21);
}

void StartRendering()
{
    I2S3_REGISTERS->I2SCFGR |= 1 << 10;
}

void DMA1_Stream5_IRQHandler()
{
    renderBuffer.index ^= RENDER_BUFFER_FRAME_COUNT;
    DMA1_REGISTERS->HIFCR |= 1 << 11;
}