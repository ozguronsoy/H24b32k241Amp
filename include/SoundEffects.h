#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

void SFX_PrepareDistortion();
uint32_t SFX_Distortion(uint32_t sample);
void SFX_PrepareOverdrive();
uint32_t SFX_Overdrive(uint32_t sample);
void SFX_PrepareChorus();
uint32_t SFX_Chorus(volatile AudioBuffer* pBuffer);