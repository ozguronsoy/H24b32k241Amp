#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

extern volatile AudioBuffer processBuffer;
extern volatile AudioBuffer renderBuffer;

/*
    PA4  - I2S3_WS
    PB3  - I2S3_CK
    PB5  - I2S3_SD
    PB10 - I2S3_MCK
*/
uint32_t InitializeRender();
void StartRendering();
void ShiftProcessBuffer();