#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

typedef volatile struct
{
    float bass;
    float low_mid;
    float high_mid;
    float treble;
    float volume;
    float distortion;
    float overdrive;
    float reverb;
    float chorus;
} AudioControlsStruct;

extern AudioControlsStruct audioControls;

/*
    PA0 - bass
    PA1 - low-mid
    PA2 - high-mid
    PA3 - treble
    PA5 - volume
    PA6 - distortion (hard-clip)
    PA7 - overdrive
    PB0 - reverb
    PB1 - chorus
    PB14 - clean mode (digital input)
*/
void InitializeAudioControls();
uint32_t IsCleanMode();