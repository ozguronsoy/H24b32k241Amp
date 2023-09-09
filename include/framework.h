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

#define RESULT_FAIL 0
#define RESULT_SUCCESS 1
#define UINT24_MAX 0xFFFFFFu
#define SAMPLE_RATE 96000u
#define PIN_STATE_HIGH 1u
#define PIN_STATE_LOW 0u
#define PI 3.14159265358979323846f
#define FFT_STEP_SIZE (FFT_SIZE / 4)
#define FFT_SIZE 4096u
#define CAPTURE_BUFFER_FRAME_COUNT (FFT_SIZE + FFT_STEP_SIZE)
#define PROCESS_BUFFER_FRAME_COUNT 9600u
#define PROCESS_BUFFER_TRANSMIT_START_INDEX (PROCESS_BUFFER_FRAME_COUNT / 2) // the first half of the buffer is for the old (rendered) samples
#define RENDER_BUFFER_FRAME_COUNT (2 * FFT_STEP_SIZE)
// CaptureBuffer + ProcessBuffer + RenderBuffer + HannWindow + ComplexBuffer (FFT) in KB
#define TOTAL_BUFFER_MEM_USAGE_KB (((CAPTURE_BUFFER_FRAME_COUNT * sizeof(uint32_t)) + (PROCESS_BUFFER_FRAME_COUNT * sizeof(uint32_t)) + (RENDER_BUFFER_FRAME_COUNT * sizeof(uint32_t)) + (FFT_SIZE * sizeof(float)) + (FFT_SIZE * sizeof(Complex))) * 1e-3f)
#define LOW 0
#define HIGH 1
#define SAMPLE_LO(sample) ((sample & 0x000000FF) << 8)
#define SAMPLE_HO(sample) ((sample & 0x00FFFF00) >> 8) // for 24-bit data, 32-bit container
#define FLOAT_TO_UINT24(sample) ((uint32_t)(sample * UINT24_MAX))
#define UINT24_TO_FLOAT(sample) (((float)sample) / ((float)UINT24_MAX))
#define I2S_PR_VALUE 0b1100000110 // MCK enabled, odd and I2SDIV = 6 for 96kHz when PLLN = 328 and PLLR = 1

typedef struct
{
    float re;
    float im;
} Complex;

typedef struct
{
    uint32_t *pData;
    uint16_t index;
} AudioBuffer;