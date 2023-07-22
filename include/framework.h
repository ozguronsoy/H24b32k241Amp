#include <inttypes.h>
#define INTTYPES_H 

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
#define PI 3.14159265358979323846f
#define INPUT_BUFFER_FRAME_COUNT 7168u
#define OUTPUT_BUFFER_FRAME_COUNT 4096u
#define FFT_SIZE 4096u




typedef struct
{
    float re;
    float im;
} Complex;