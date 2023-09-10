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
#define SAMPLE_RATE 32000u
#define PIN_STATE_HIGH 1u
#define PIN_STATE_LOW 0u
#define PI 3.14159265358979323846f
#define CAPTURE_BUFFER_FRAME_COUNT (SAMPLE_RATE / 100) // 10ms
#define PROCESS_BUFFER_FRAME_COUNT (SAMPLE_RATE / 5)   // 200ms
#define RENDER_BUFFER_FRAME_COUNT CAPTURE_BUFFER_FRAME_COUNT
// CaptureBuffer + ProcessBuffer + RenderBuffer
#define TOTAL_BUFFER_MEM_USAGE_KB (((CAPTURE_BUFFER_FRAME_COUNT * sizeof(uint32_t) * 2) + (PROCESS_BUFFER_FRAME_COUNT * sizeof(uint32_t)) + (RENDER_BUFFER_FRAME_COUNT * sizeof(uint32_t) * 2)) * 1e-3f)
#define LOW 0
#define HIGH 1
#define FLOAT_TO_UINT24(sample) ((uint32_t)((sample)*UINT24_MAX))
#define UINT24_TO_FLOAT(sample) (((float)(sample)) / ((float)UINT24_MAX))

typedef struct
{
    uint32_t *pData;
    uint16_t index;
} AudioBuffer;