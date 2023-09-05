#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

// Allocates memory for the equalizer
uint32_t InitializeSoundEffects();
float SFX_Distortion(float sample);
float SFX_Overdrive(float sample);
float SFX_Chorus(volatile AudioBuffer* pBuffer);
void SFX_Equalizer(volatile AudioBuffer* pInputBuffer, volatile AudioBuffer* pOutputBuffer);