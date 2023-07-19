#include <inttypes.h>
#define INTTYPES_H 

#pragma region DEFINES

#if defined(DEBUG)

#include <stdio.h>
#define STDIO_H

extern int initialise_monitor_handles();

#define INIT_MONITOR_HANDLES initialise_monitor_handles()
#define DEBUG_PRINT(str) printf(str)
#define DEBUG_PRINTF(str, ...) printf(str, __VA_ARGS__)

#else

#define INIT_MONITOR_HANDLES
#define DEBUG_PRINT(str)
#define DEBUG_PRINTF(str, ...) 

#endif

#define UINT24_MAX 0xFFFFFFu
#define SAMPLE_RATE 96000u
#define PIN_STATE_HIGH 1u
#define PIN_STATE_LOW 0u

#pragma endregion


#pragma region ENUMS

typedef enum
{
    R_Success = 0,
    R_Fail = 1, // nothing major, continue to run the app
    R_HardFail = 2, // needs reset
    R_Timeout = 3
} Result;

#pragma endregion


#pragma region STRUCTS

typedef struct
{
    float re;
    float im;
} Complex;

#pragma endregion