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

#define I2S2_REGISTERS ((SpiRegisters *)0x40003800u)

volatile uint8_t receiveOrder = HIGH;
volatile AudioBuffer captureBuffer;

uint32_t Capture_InitBuffer()
{
    DEBUG_PRINT("CAPTURE: Initializing the capture buffer\n");
    const uint32_t captureBufferSize = CAPTURE_BUFFER_FRAME_COUNT * sizeof(uint32_t);
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

void Capture_ConfigureRCC()
{
    DEBUG_PRINT("CAPTURE: Configuring the RCC for I2S2\n");
    RCC_APB1ENR |= 0b1 << 14; // enable the SPI2 clock
}

void Capture_ConfigureGPIORegisters()
{
    DEBUG_PRINT("CAPTURE: Configuring the I2S2 GPIO registers\n");

    // set to alternate function mode
    GPIOB_REGISTERS->MODER |= 0b10001010 << 24;
    GPIOC_REGISTERS->MODER |= 0b10 << 12;

    // alternate function mapping
    GPIOB_REGISTERS->AFRH |= 0b0101000001010101 << 16;
    GPIOC_REGISTERS->AFRL |= 0b0101 << 24;
}

void Capture_ConfigureI2SRegisters()
{
    DEBUG_PRINT("CAPTURE: Configuring the I2S2 registers\n");

    // ENTERS AN INFINITE LOOP FOR SOME REASON
    I2S2_REGISTERS->CR2 |= 1 << 6;          // enable the I2S2 receive IRQ handling
    I2S2_REGISTERS->I2SCFGR |= 0b1011 << 8; // select the I2S mode as master-receive
    I2S2_REGISTERS->I2SCFGR |= 0b010011;    // steady state low, 24-bit data, 32 bit container, MSB (left) justified
    I2S2_REGISTERS->I2SPR |= I2S_PR_VALUE;
}

uint32_t InitializeCapture()
{
    DEBUG_PRINT("CAPTURE: initializing...\n");

    receiveOrder = HIGH;

    if (Capture_InitBuffer() == RESULT_FAIL)
    {
        return RESULT_FAIL;
    }

    Capture_ConfigureRCC();
    Capture_ConfigureGPIORegisters();
    Capture_ConfigureI2SRegisters();

    NVIC_ISER1 |= 1 << 4;    // enable the SPI2 interrupt
    NVIC_IPR9 |= 0b00100000; // set the interrupt priority

    DEBUG_PRINT("CAPTURE: initialized successfully\n");

    return RESULT_SUCCESS;
}

void StartCapturing()
{
    I2S2_REGISTERS->I2SCFGR |= 1 << 10; // enable the I2S peripheral
}

void ShiftRenderBuffer()
{
    // shift the render buffer to the left by 1024 samples since the first FFT_STEP_SIZE samples are already transmitted
    uint32_t i;
    for (i = 0; i < RENDER_BUFFER_FRAME_COUNT; i++)
    {
        renderBuffer.pData[i] = (i < (RENDER_BUFFER_FRAME_COUNT - FFT_STEP_SIZE)) ? (renderBuffer.pData[i + FFT_STEP_SIZE]) : (0);
    }
}

void ApplyEffects()
{
    enableTransmit = 0;

    ShiftRenderBuffer();
    SFX_Equalizer(&captureBuffer, &renderBuffer);

    if (!IsCleanMode())
    {
        for (renderBuffer.index = RENDER_BUFFER_TRANSMIT_START_INDEX; renderBuffer.index < (RENDER_BUFFER_TRANSMIT_START_INDEX + FFT_STEP_SIZE); renderBuffer.index++)
        {
            renderBuffer.pData[renderBuffer.index] = SFX_Chorus(&renderBuffer);
            renderBuffer.pData[renderBuffer.index] = SFX_Overdrive(renderBuffer.pData[renderBuffer.index]);
            renderBuffer.pData[renderBuffer.index] = SFX_Distortion(renderBuffer.pData[renderBuffer.index]);
        }
    }

    renderBuffer.index = RENDER_BUFFER_TRANSMIT_START_INDEX;

    enableTransmit = 1;
}

void SPI2_IRQHandler()
{
    if (receiveOrder == HIGH)
    {
        captureBuffer.pData[captureBuffer.index] = (I2S2_REGISTERS->DR << 8) & 0x00FFFF00;
    }
    else
    {
        captureBuffer.pData[captureBuffer.index] |= (I2S2_REGISTERS->DR >> 8) & 0x000000FF;
        if ((++captureBuffer.index) == CAPTURE_BUFFER_FRAME_COUNT)
        {
            captureBuffer.index = 0;
        }

        if (enableTransmit)
        {
            if ((captureBuffer.index % FFT_STEP_SIZE) == 0)
            {
                ApplyEffects();
            }
        }
        else
        {
            if (captureBuffer.index == 0) // wait for the first FFT_SIZE of samples to be received, then start applying EQ every FFT_STEP_SIZE samples received
            {
                ApplyEffects();
            }
        }
    }
    receiveOrder ^= 1;
}