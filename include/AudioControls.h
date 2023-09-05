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
    float chorus_depth;
    float chorus_rate; // LFO frequency
} AudioControlsStruct;

extern AudioControlsStruct audioControls;

/*
    PA0 - bass
    PA1 - low_mid
    PA2 - high_mid
    PA3 - treble
    PA5 - volume
    PA6 - distortion (hard-clip)
    PA7 - overdrive
    PB0 - chorus-depth
    PB1 - chorus-rate
    PB14 - clean mode (digital input)
*/
uint32_t InitializeAudioControls();
uint32_t IsCleanMode();