#if !defined(FRAMEWORK_H)
#include "framework.h"
#define FRAMEWORK_H
#endif

/*
    PB12  - I2S2_WS
    PB13  - I2S2_CK
    PB15  - I2S2_SD
    PC6   - I2S2_MCK
*/
uint32_t InitializeCapture();
void StartCapturing();