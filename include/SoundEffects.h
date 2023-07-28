#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

void InitializeSoundEffects();
float SFX_Distortion(float sample);
float SFX_Overdrive(float sample);
float SFX_Chorus(AudioBuffer* pBuffer);
void SFX_Equalizer(AudioBuffer* pInputBuffer, AudioBuffer* pOutputBuffer);