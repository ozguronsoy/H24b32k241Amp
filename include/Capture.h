#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

extern volatile AudioBuffer captureBuffer;

/*
    PB4  - I2S3ext_SD
*/
uint32_t InitializeCapture();
void DeinitializeCapture();
void StartCapturing();