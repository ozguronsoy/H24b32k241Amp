#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

// Allocates memory for the equalizer
uint32_t InitializeSoundEffects();
uint32_t SFX_Distortion(uint32_t sample);
uint32_t SFX_Overdrive(uint32_t sample);
uint32_t SFX_Chorus(volatile AudioBuffer* pBuffer);
void SFX_Equalizer(volatile AudioBuffer* pInputBuffer, volatile AudioBuffer* pOutputBuffer);