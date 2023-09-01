#include <inttypes.h>
#define INTTYPES_H 

#if defined(DEBUG)

#include <stdio.h>
#define STDIO_H

extern int initialise_monitor_handles();

#define INIT_MONITOR_HANDLES initialise_monitor_handles()
#define DEBUG_PRINT(str) printf(str)
#define DEBUG_PRINTF(str, ...) printf(str, __VA_ARGS__)
#define TEST_OUTPUT TEST_DISABLED                            // set this to TEST_ENABLED to test the output with a sine wave
#define TEST_OUTPUT_USE_CONSTANT_VALUE TEST_DISABLED         // set both TEST_OUTPUT and this to TEST_ENABLED to test the output with a constant value
#define TEST_OTUPUT_CONSTANT_VALUE 0xABCDEFu                // the value that will be used in testing.

#else

#define INIT_MONITOR_HANDLES
#define DEBUG_PRINT(str)
#define DEBUG_PRINTF(str, ...)
#define TEST_OUTPUT
#define TEST_OUTPUT_USE_CONSTANT_VALUE
#define TEST_OTUPUT_CONSTANT_VALUE

#endif

#define TEST_DISABLED 0
#define TEST_ENABLED 1




#define UINT24_MAX 0xFFFFFFu
#define SAMPLE_RATE 96000u
#define PIN_STATE_HIGH 1u
#define PIN_STATE_LOW 0u
#define PI 3.14159265358979323846f
#define INPUT_BUFFER_FRAME_COUNT 7168u
#define OUTPUT_BUFFER_FRAME_COUNT 8192u
#define FFT_SIZE 4096u




typedef struct
{
    float re;
    float im;
} Complex;

typedef struct
{
    float* pData;
    uint16_t readIndex;
    uint16_t writeIndex;
} AudioBuffer;