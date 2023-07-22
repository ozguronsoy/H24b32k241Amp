#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

float SFX_Distortion(float sample);
float SFX_Overdrive(float sample);
float SFX_Chorus(float* pBuffer, uint16_t index);
void SFX_Equalizer(float* pInputBuffer, float* pOutputBuffer, uint32_t inputBufferStartIndex);