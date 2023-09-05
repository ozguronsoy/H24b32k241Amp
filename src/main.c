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

#if !defined(DAC_CONTROLS_H)
#include "DacControls.h"
#define DAC_CONTROLS_H
#endif

#if !defined(AUDIO_CONTROLS_H)
#include "AudioControls.h"
#define AUDIO_CONTROLS_H
#endif

#if !defined(SOUND_EFFECTS_H)
#include "SoundEffects.h"
#define SOUND_EFFECTS_H
#endif

#define CHECK_RESULT(x) if (x == RESULT_FAIL) { goto LOOP; }

int main()
{
    INIT_MONITOR_HANDLES; // debug only

    RCC_AHB1ENR |= 0b111; // enable the GPIO A, B & C clock

    CHECK_RESULT(ConfigureDAC());
    CHECK_RESULT(InitializeSoundEffects());
    CHECK_RESULT(InitializeAudioControls());
    ConfigurePLLI2S();
    CHECK_RESULT(InitializeCapture());
    CHECK_RESULT(InitializeRender());
    StartCapturing();
    StartRendering();

LOOP:
    while(1);
    
    return 0;
}