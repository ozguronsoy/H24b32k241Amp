#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

#if !defined(RCC_H)
#include "rcc.h"
#define RCC_H
#endif

#if !defined(I2S_H)
#include "i2s.h"
#define I2S_H
#endif

#if !defined(RENDER_H)
#include "Render.h"
#define RENDER_H
#endif

#if !defined(CAPTURE_H)
#include "Capture.h"
#define CAPTURE_H
#endif

#if !defined(AUDIO_CONTROLS_H)
#include "AudioControls.h"
#define AUDIO_CONTROLS_H
#endif

#if !defined(SOUND_EFFECTS_H)
#include "SoundEffects.h"
#define SOUND_EFFECTS_H
#endif

#include "gpio.h"

#define CHECK_RESULT(x)   \
    if (x == RESULT_FAIL) \
    {                     \
        goto ERROR;       \
    }

#define SCB_CPACR (*(uint32_t *)0xE000ED88u)

int main()
{
    INIT_MONITOR_HANDLES; // debug only

    SCB_CPACR |= 0b1111 << 20; // set CP10 & CP11 Full Access for FPU

    RCC_ConfigureSystemClock();

    DEBUG_PRINTF("Total Buffer Memory Usage: %lf KB\n", TOTAL_BUFFER_MEM_USAGE_KB);

    RCC_AHB1ENR |= 0b11; // enable the GPIO A & B clock

    SFX_Initialize();
    CHECK_RESULT(InitializeAudioControls());

    ConfigurePLLI2S();
    CHECK_RESULT(InitializeRender());
    CHECK_RESULT(InitializeCapture());
    StartRendering();
    StartCapturing();

    MEASURING_INIT();

    while (1);

ERROR:
    DeinitializeCapture();
    DeinitializeRender();
    DeinitializeAudioControls();
    RCC_AHB1ENR &= ~0b11;
    SCB_CPACR &= ~(0b1111 << 20);
    while (1);

    return 0;
}