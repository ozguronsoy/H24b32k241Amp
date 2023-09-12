#include <inttypes.h>
#define INTTYPES_H

#if defined(DEBUG)

#include <stdio.h>
#define STDIO_H

extern int initialise_monitor_handles();

#define INIT_MONITOR_HANDLES initialise_monitor_handles()
#define DEBUG_PRINT(str) printf(str)
#define DEBUG_PRINTF(str, ...) printf(str, __VA_ARGS__)

// measuring via GPIO PB12 pin
#define MEASURING_INIT()               \
    GPIOB_REGISTERS->MODER |= 1 << 24; \
    GPIOB_REGISTERS->ODR &= ~(1 << 12)
#define MEASURING_TOGGLE() GPIOB_REGISTERS->ODR = ((GPIOB_REGISTERS->ODR >> 12) & 1) ? (GPIOB_REGISTERS->ODR & ~(1 << 12)) : (GPIOB_REGISTERS->ODR | (1 << 12))

#else

#define INIT_MONITOR_HANDLES
#define DEBUG_PRINT(str)
#define DEBUG_PRINTF(str, ...)

#define MEASURING_INIT()
#define MEASURING_TOGGLE()

#endif

#define RESULT_FAIL 0
#define RESULT_SUCCESS 1
#define UINT24_MAX 0xFFFFFFu
#define SAMPLE_RATE 32000u
#define PI 3.14159265358979323846f
#define STEP_SIZE (FFT_SIZE / 4u)
#define FFT_SIZE 2048u
#define CAPTURE_BUFFER_FRAME_COUNT (FFT_SIZE + STEP_SIZE)
#define PROCESS_BUFFER_FRAME_COUNT 9600u
#define PROCESS_BUFFER_RENDER_START_INDEX (PROCESS_BUFFER_FRAME_COUNT / 2)
#define RENDER_BUFFER_FRAME_COUNT STEP_SIZE
// CaptureBuffer + ProcessBuffer + RenderBuffer + EQ_Buffer + HannBuffer
#define TOTAL_BUFFER_MEM_USAGE_KB (((CAPTURE_BUFFER_FRAME_COUNT * sizeof(uint32_t)) + (PROCESS_BUFFER_FRAME_COUNT * sizeof(uint32_t)) + (2 * RENDER_BUFFER_FRAME_COUNT * sizeof(uint32_t)) + (FFT_SIZE * sizeof(Complex)) + (FFT_SIZE * sizeof(float))) * 1e-3f)
#define FLOAT_TO_UINT24(sample) ((uint32_t)((sample)*UINT24_MAX))
#define UINT24_TO_FLOAT(sample) (((float)(sample)) / ((float)UINT24_MAX))

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