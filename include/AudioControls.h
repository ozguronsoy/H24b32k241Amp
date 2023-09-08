#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

#if !defined(MATH_H)
#include <math.h>
#define MATH_H
#endif

#define ADC_MAX 4095u // 12-bit ADC
#define AUDIO_CONTROLS_NORMALIZE(x) (ceilf((((float)x) / ((float)ADC_MAX)) * 1e2f) * 1e-2f) // two digits

typedef volatile struct
{
    uint16_t bass;
    uint16_t low_mid;
    uint16_t high_mid;
    uint16_t treble;
    uint16_t volume;
    uint16_t distortion;
    uint16_t overdrive;
    uint16_t chorus_depth;
    uint16_t chorus_rate; // LFO frequency
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
    conversion order: PA0-PA1-PA2-PA3-PA5-PA6-PA7-PB0-PB1 (CH0-CH1-CH2-CH3-CH5-CH6-CH7-CH8-CH9)
*/
uint32_t InitializeAudioControls();
// PB14 HIGH = Clean Mode
uint32_t IsCleanMode();