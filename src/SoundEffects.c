#if !defined(SOUND_EFFECTS_H)
#include "SoundEffects.h"
#define SOUND_EFFECTS_H
#endif

#if !defined(AUDIO_CONTROLS_H)
#include "AudioControls.h"
#define AUDIO_CONTROLS_H
#endif

#if !defined(MATH_H)
#include <math.h>
#define MATH_H
#endif

#if !defined(STDLIB_H)
#include <stdlib.h>
#define STDLIB_H
#endif

#include "arm_math.h"
#include "arm_const_structs.h"

#define INPUT_RANGE(sample) (((sample)*2.0f) - 1.0f)    // [0, 1] -> [-1, 1]
#define OUTPUT_RANGE(sample) (((sample) + 1.0f) * 0.5f) // [-1, 1] -> [0, 1]

#define OVERDRIVE_ORIGIN (PROCESS_BUFFER_FRAME_COUNT / 2)

#define CHORUS_LFO_DT (1.0f / ((float)SAMPLE_RATE))
#define CHORUS_BASE_DELAY_MS 3u
#define CHORUS_BASE_DELAY_SAMPLE (CHORUS_BASE_DELAY_MS * SAMPLE_RATE / 1000u) // the delay will be between [BASE_DELAY_MS, (BASE_DELAY_MS + DELAY_MS)]
#define CHORUS_DELAY_MS 30u
#define CHORUS_DELAY_SAMPLE (CHORUS_DELAY_MS * SAMPLE_RATE / 1000u)
// fs * (pow(2, semitone / 12) - 1.0)
// 1 semitone
#define CHORUS_RESAMPLE_DELTA 1902u

#define EQ_NYQUIST (FFT_SIZE / 2)
#define EQ_BIN_START_L_MID 21u   // 160Hz, calculated with [(FFT_SIZE / SAMPLE_RATE) * 160Hz]
#define EQ_BIN_START_H_MID 93u   // 720Hz
#define EQ_BIN_START_TREBLE 164u // 1280Hz

#if FFT_SIZE == 4096
#define EQ_CFFT_STRUCT arm_cfft_sR_f32_len4096
#elif FFT_SIZE == 2048
#define EQ_CFFT_STRUCT arm_cfft_sR_f32_len2048
#elif FFT_SIZE == 1024
#define EQ_CFFT_STRUCT arm_cfft_sR_f32_len1024
#elif FFT_SIZE == 512
#define EQ_CFFT_STRUCT arm_cfft_sR_f32_len512
#elif FFT_SIZE == 256
#define EQ_CFFT_STRUCT arm_cfft_sR_f32_len256
#else
#error Invalid FFT size!
#endif

uint32_t distortion_upper_boundary = 0;
uint32_t distortion_lower_boundary = 0;

float overdrive_wet = 0.0f;
float overdrive_dry = 0.0f;

float chorus_LFO_t = 0.0f;
float chorus_rate = 0.0f;
float chorus_wet = 0.0f;
float chorus_dry = 0.0f;

volatile float *hannBuffer = NULL;
volatile Complex *pComplexBuffer = NULL;

uint32_t SFX_Initialize()
{
    DEBUG_PRINT("SOUND EFFECTS: initializing...\n");

    DEBUG_PRINT("SOUND EFFECTS: initializing the EQ buffer\n");
    pComplexBuffer = (Complex *)malloc(FFT_SIZE * sizeof(Complex));
    if (pComplexBuffer == NULL)
    {
        DEBUG_PRINT("Insufficient memory! (EQ buffer)");
        return RESULT_FAIL;
    }

    DEBUG_PRINT("SOUND EFFECTS: initializing the Hann  window\n");
    hannBuffer = (float *)malloc(FFT_SIZE * sizeof(float));
    if (hannBuffer == NULL)
    {
        DEBUG_PRINT("Insufficient memory! (Hann window)");
        return RESULT_FAIL;
    }

    uint32_t i;
    for (i = 0; i < FFT_SIZE; i++)
    {
        hannBuffer[i] = sinf(PI * ((float)i) / ((float)(FFT_SIZE - 1)));
    }

    DEBUG_PRINT("SOUND EFFECTS: initialized successfully\n");

    return RESULT_SUCCESS;
}

void SFX_PrepareDistortion()
{
    distortion_lower_boundary = ((float)audioControls.distortion / (ADC_MAX * 10 / 3)) * UINT24_MAX;
    distortion_upper_boundary = UINT24_MAX - distortion_lower_boundary;
}

uint32_t SFX_Distortion(uint32_t sample)
{
    if (sample >= distortion_upper_boundary)
    {
        return distortion_upper_boundary;
    }

    if (sample <= distortion_lower_boundary)
    {
        return distortion_lower_boundary;
    }

    return sample;
}

void SFX_PrepareOverdrive()
{
    overdrive_wet = AUDIO_CONTROLS_NORMALIZE(audioControls.overdrive);
    overdrive_dry = 1.0f - overdrive_wet;
}

uint32_t SFX_Overdrive(uint32_t sample)
{
    const uint32_t drySample = sample;
    sample -= OVERDRIVE_ORIGIN;

    if (sample < 0)
    {
        const uint32_t absSample = -sample;
        if (absSample < (OVERDRIVE_ORIGIN / 3))
        {
            return (2 * sample + OVERDRIVE_ORIGIN) * overdrive_wet + drySample * overdrive_dry;
        }
        else if (absSample < (2 * OVERDRIVE_ORIGIN / 3))
        {
            const uint32_t z = 2 - 3 * sample;
            return (((3 - z * z) / 3) + OVERDRIVE_ORIGIN) * overdrive_wet + drySample * overdrive_dry;
        }
        return drySample * overdrive_dry;
    }
    else
    {
        if (sample < (OVERDRIVE_ORIGIN / 3))
        {
            return (2 * sample + OVERDRIVE_ORIGIN) * overdrive_wet + drySample * overdrive_dry;
        }
        else if (sample < (2 * OVERDRIVE_ORIGIN / 3))
        {
            const uint32_t z = 2 - 3 * sample;
            return (((3 - z * z) / 3) + OVERDRIVE_ORIGIN) * overdrive_wet + drySample * overdrive_dry;
        }
        return UINT24_MAX * overdrive_wet + drySample * overdrive_dry;
    }
}

void SFX_PrepareChorus()
{
    chorus_rate = AUDIO_CONTROLS_NORMALIZE(audioControls.chorus_rate);
    chorus_wet = AUDIO_CONTROLS_NORMALIZE(audioControls.chorus_depth) * 0.5f;
    chorus_dry = 1.0f - chorus_wet;
}

uint32_t SFX_Chorus(volatile AudioBuffer *pBuffer)
{
    const float lfoSample = (arm_sin_f32((2.0f * PI) * chorus_rate * chorus_LFO_t) + 1.0f) * 0.5f;
    const uint32_t currentDelay_sample = roundf(lfoSample * (CHORUS_DELAY_SAMPLE) + (CHORUS_BASE_DELAY_SAMPLE)); // n samples of delay
    const float resampleIndex = (pBuffer->index - currentDelay_sample) + lfoSample * CHORUS_RESAMPLE_DELTA;
    const float resampleFactor = resampleIndex - floorf(resampleIndex);
    const uint32_t roundedResampleIndex = roundf(resampleIndex);

    chorus_LFO_t += CHORUS_LFO_DT;

    return (pBuffer->pData[pBuffer->index] >> 8) * chorus_dry + ((pBuffer->pData[roundedResampleIndex] >> 8) * (1.0f - resampleFactor) + (pBuffer->pData[roundedResampleIndex + 1] >> 8) * resampleFactor) * chorus_wet;
}

void SFX_Equalizer(volatile AudioBuffer *pInputBuffer, volatile AudioBuffer *pOutputBuffer)
{
    uint32_t i;
    for (i = 0; i < FFT_SIZE; i++)
    {
        pComplexBuffer[i].re = UINT24_TO_FLOAT(pInputBuffer->pData[(pInputBuffer->index + PROCESS_BUFFER_RENDER_START_INDEX + i) % CAPTURE_BUFFER_FRAME_COUNT] >> 8);
        pComplexBuffer[i].im = 0.0f;
    }

    arm_cfft_f32(&EQ_CFFT_STRUCT, (float *)&pComplexBuffer->re, 0, 1);

    volatile const float bass = audioControls.bass;
    volatile const float low_mid = audioControls.low_mid;
    volatile const float high_mid = audioControls.high_mid;
    volatile const float treble = audioControls.treble;

    for (i = 0; i < EQ_BIN_START_L_MID; i++)
    {
        pComplexBuffer[i].re *= bass;
        pComplexBuffer[i].im *= bass;
        pComplexBuffer[FFT_SIZE - i] = pComplexBuffer[i];
    }
    for (; i < EQ_BIN_START_H_MID; i++)
    {
        pComplexBuffer[i].re *= low_mid;
        pComplexBuffer[i].im *= low_mid;
        pComplexBuffer[FFT_SIZE - i] = pComplexBuffer[i];
    }
    for (; i < EQ_BIN_START_TREBLE; i++)
    {
        pComplexBuffer[i].re *= high_mid;
        pComplexBuffer[i].im *= high_mid;
        pComplexBuffer[FFT_SIZE - i] = pComplexBuffer[i];
    }
    for (; i < EQ_NYQUIST; i++)
    {
        pComplexBuffer[i].re *= treble;
        pComplexBuffer[i].im *= treble;
        pComplexBuffer[FFT_SIZE - i] = pComplexBuffer[i];
    }

    arm_cfft_f32(&EQ_CFFT_STRUCT, (float *)&pComplexBuffer->re, 1, 1);

    for (i = 0; i < FFT_SIZE; i++)
    {
        pOutputBuffer->pData[i + PROCESS_BUFFER_RENDER_START_INDEX] += FLOAT_TO_UINT24(pComplexBuffer[i].re * hannBuffer[i] / FFT_SIZE);
    }
}