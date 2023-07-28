#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

#if !defined(I2S_H)
#include "i2s.h"
#define I2S_H
#endif

#if !defined(AUDIO_CONTROLS_H)
#include "AudioControls.h"
#define AUDIO_CONTROLS_H
#endif

#if !defined(SOUND_EFFECTS_H)
#include "SoundEffects.h"
#define SOUND_EFFECTS_H
#endif

#if !defined(LED_CONTROLS_H)
#include "LEDControls.h"
#define LED_CONTROLS_H
#endif

int main()
{
    INIT_MONITOR_HANDLES;
    INIT_LED;
    InitializeSoundEffects();
    InitializeAudioControls();
    InitializeI2S3();

    while(1);
    return 0;
}