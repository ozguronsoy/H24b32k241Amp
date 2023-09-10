#include "Capture.h"
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

#if !defined(SOUND_EFFECTS_H)
#include "SoundEffects.h"
#define SOUND_EFFECTS_H
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

#define I2S3EXT_REGISTERS ((SpiRegisters *)0x40004000u)

volatile AudioBuffer captureBuffer;

uint32_t Capture_InitBuffer()
{
    DEBUG_PRINT("CAPTURE: Initializing the capture buffer\n");
    const uint32_t captureBufferSize = CAPTURE_BUFFER_FRAME_COUNT * sizeof(uint32_t) * 2;
    captureBuffer.pData = (uint32_t *)malloc(captureBufferSize);
    if (captureBuffer.pData == NULL)
    {
        DEBUG_PRINT("Insufficient memory (capture buffer)\n");
        return RESULT_FAIL;
    }
    memset(captureBuffer.pData, 0, captureBufferSize);
    captureBuffer.index = 0;

    return RESULT_SUCCESS;
}

void Capture_ConfigureGPIORegisters()
{
    DEBUG_PRINT("CAPTURE: Configuring the I2S3ext GPIO registers\n");

    GPIOB_REGISTERS->MODER |= 0b10 << 7;
    GPIOB_REGISTERS->AFRL |= 0b0111 << 16;
}

void Capture_ConfigureI2SRegisters()
{
    DEBUG_PRINT("CAPTURE: Configuring the I2S3ext registers\n");

    I2S3EXT_REGISTERS->I2SCFGR |= 0b1001 << 8; // select the I2S mode as slave-receive
    I2S3EXT_REGISTERS->I2SCFGR |= 0b010011;    // steady state low, 24-bit data, 32 bit container, MSB (left) justified
    I2S3EXT_REGISTERS->CR2 |= 1;               // Receive DMA enabled
}

void Capture_ConfigureDmaRegisters()
{
    DEBUG_PRINT("CAPTURE: Configuring the DMA registers\n");

    // Memory data size = half-word (16-bit), Peripheral data size = half-word
    // Memory increment mode enabled
    DMA1_REGISTERS->S0.CR |= 0b01011 << 10;
    DMA1_REGISTERS->S0.CR |= 3 << 25;                         // channel 3
    DMA1_REGISTERS->S0.CR |= 0b111 << 16;                     // double buffered, very high priority
    DMA1_REGISTERS->S5.CR |= 0b100 << 6;                      // circular mode, data direction = M2P
    DMA1_REGISTERS->S0.CR |= 1 << 4;                          // transfer complete interrupt
    DMA1_REGISTERS->S0.NDTR = 2 * CAPTURE_BUFFER_FRAME_COUNT; // since the I2S container size is 32 bits and one transfer is 16 bits, have to make (2 * buffer_size) transfers
    DMA1_REGISTERS->S0.PAR = (uint32_t)&I2S3EXT_REGISTERS->DR;
    DMA1_REGISTERS->S0.M0AR = (uint32_t)captureBuffer.pData;
    DMA1_REGISTERS->S0.M1AR = (uint32_t)(captureBuffer.pData + CAPTURE_BUFFER_FRAME_COUNT);
    DMA1_REGISTERS->S0.CR |= 1; // enable the DMA
}

uint32_t InitializeCapture()
{
    DEBUG_PRINT("CAPTURE: initializing...\n");

    if (Capture_InitBuffer() == RESULT_FAIL)
    {
        return RESULT_FAIL;
    }

    Capture_ConfigureGPIORegisters();
    Capture_ConfigureI2SRegisters();
    Capture_ConfigureDmaRegisters();

    NVIC_ISER0 |= 1 << 11; // enable DMA1_Stream0 IRQ handler

    DEBUG_PRINT("CAPTURE: initialized successfully\n");

    return RESULT_SUCCESS;
}

void DeinitializeCapture()
{
    DEBUG_PRINT("CAPTURE: Deinitializing\n");

    NVIC_ISER0 &= ~(1 << 11);
    DMA1_REGISTERS->S0.CR &= ~1;
    I2S3EXT_REGISTERS->I2SCFGR &= ~(1 << 10);
}

void StartCapturing()
{
    I2S3EXT_REGISTERS->I2SCFGR |= 1 << 10;
}

void ApplyEffects()
{
    volatile const float volume = AUDIO_CONTROLS_NORMALIZE(audioControls.volume);

    uint32_t i, j;
    if (IsCleanMode())
    {
        for (i = 0, j = renderBuffer.index; i < CAPTURE_BUFFER_FRAME_COUNT; i++, j++)
        {
            renderBuffer.pData[j] = (processBuffer.pData[i] >> 8) * volume;
            renderBuffer.pData[j] = ((renderBuffer.pData[j] << 24) & 0xFF000000) | ((renderBuffer.pData[j] >> 8) & 0xFFFF); // 0x00ABCDEF -> 0xEF00ABCD to render correctly
            processBuffer.index = ((++processBuffer.index) == PROCESS_BUFFER_FRAME_COUNT) ? (0) : (processBuffer.index);
        }
    }
    else
    {
        SFX_PrepareDistortion();
        SFX_PrepareOverdrive();
        SFX_PrepareChorus();
        for (i = 0, j = renderBuffer.index; i < CAPTURE_BUFFER_FRAME_COUNT; i++, j++)
        {
            renderBuffer.pData[j] = SFX_Distortion(SFX_Overdrive(SFX_Chorus(&processBuffer))) * volume;
            renderBuffer.pData[j] = ((renderBuffer.pData[j] << 24) & 0xFF000000) | ((renderBuffer.pData[j] >> 8) & 0xFFFF); // 0x00ABCDEF -> 0xEF00ABCD to render correctly
            processBuffer.index = ((++processBuffer.index) == PROCESS_BUFFER_FRAME_COUNT) ? (0) : (processBuffer.index);
        }
    }
}

void DMA1_Stream0_IRQHandler()
{
    memcpy(processBuffer.pData + processBuffer.index, captureBuffer.pData + captureBuffer.index, CAPTURE_BUFFER_FRAME_COUNT * sizeof(uint32_t));
    captureBuffer.index = (captureBuffer.index + CAPTURE_BUFFER_FRAME_COUNT) % (CAPTURE_BUFFER_FRAME_COUNT * 2);
    ApplyEffects();
    DMA1_REGISTERS->LIFCR |= 1 << 5;
}