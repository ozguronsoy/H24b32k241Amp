#if !defined(SOUND_EFFECTS_H)
#include "SoundEffects.h"
#define SOUND_EFFECTS_H
#endif

#if !defined (AUDIO_CONTROLS_H)
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
#include "arm_common_tables.h"
#include "arm_const_structs.h"
#include "Fourier.h"


#define INPUT_RANGE(sample) (((sample) * 2.0f) - 1.0f) // [0, 1] -> [-1, 1]
#define OUTPUT_RANGE(sample) (((sample) + 1.0f) * 0.5f) // [-1, 1] -> [0, 1]

#define DISTORTION_LOWER_BOUNDARY(distortion) (0.3f * (distortion) * UINT24_MAX)
#define DISTORTION_UPPER_BOUNDARY(distortion) (0.7f * (distortion) * UINT24_MAX)


#define CHORUS_LFO_DT (1.0f / ((float)SAMPLE_RATE))
#define CHORUS_BASE_DELAY_MS 3u
#define CHORUS_BASE_DELAY_SAMPLE (CHORUS_BASE_DELAY_MS * SAMPLE_RATE / 1000) // the delay will be between [BASE_DELAY_MS, (BASE_DELAY_MS + DELAY_MS)]
#define CHORUS_DELAY_MS 30u
#define CHORUS_DELAY_SAMPLE (CHORUS_DELAY_MS * SAMPLE_RATE / 1000)
// fs * (pow(2, semitone / 12) - 1.0)
// 0.75 semitone
#define CHORUS_RESAMPLE_DELTA 4250u


#define EQ_NYQUIST (FFT_SIZE >> 1)
#define EQ_BIN_START_L_MID 7u    // 160 Hz
#define EQ_BIN_START_H_MID 31u   // 720 Hz
#define EQ_BIN_START_TREBLE 55u  // 1280 Hz

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



volatile float chorus_t_LFO = 0.0f;
volatile float* hannBuffer = NULL;
volatile Complex* pComplexBuffer = NULL;


uint32_t InitializeSoundEffects()
{
    DEBUG_PRINT("SOUND EFFECTS: initializing...\n");

    DEBUG_PRINT("SOUND EFFECTS: initializing the EQ buffer\n");
    pComplexBuffer = (Complex*)malloc(FFT_SIZE * sizeof(Complex));
    if (pComplexBuffer == NULL)
    {
        DEBUG_PRINT("Insufficient memory! (EQ buffer)");
        return RESULT_FAIL;
    }

    DEBUG_PRINT("SOUND EFFECTS: initializing the Hann  window\n");
    hannBuffer = (float*)malloc(FFT_SIZE * sizeof(float));
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

uint32_t SFX_Distortion(uint32_t sample)
{
    volatile const float d = AUDIO_CONTROLS_NORMALIZE(audioControls.distortion);
    
    const uint32_t u = DISTORTION_UPPER_BOUNDARY(d);
    if (sample > u)
    {
        return u;
    }

    const uint32_t l = DISTORTION_LOWER_BOUNDARY(d);
    if (sample < l)
    {
        return l;
    }

    return sample;
}

uint32_t SFX_Overdrive(uint32_t sample)
{
    volatile const float a = sinf(AUDIO_CONTROLS_NORMALIZE(audioControls.overdrive) * PI * 0.5f);
    const float k = 2.0f * a / (1.0f - a);
    const float fsample = INPUT_RANGE(UINT24_TO_FLOAT(sample));
    return OUTPUT_RANGE(((1.0f + k) * sample / (1.0f + k * fabsf(fsample)))) * UINT24_MAX;
}

uint32_t SFX_Chorus(volatile AudioBuffer* pBuffer)
{
    volatile const float rate = AUDIO_CONTROLS_NORMALIZE(audioControls.chorus_rate);
    volatile const float wet = AUDIO_CONTROLS_NORMALIZE(audioControls.chorus_depth) * 0.5f;

    if (rate == 0 || wet == 0)
    {
        return pBuffer->pData[pBuffer->index];
    }

    const float lfoSample = (sinf(2.0f * PI * rate * chorus_t_LFO) + 1.0f) * 0.5f;
    const uint16_t currentDelay_sample = roundf(lfoSample * CHORUS_DELAY_SAMPLE + CHORUS_BASE_DELAY_SAMPLE); // n samples of delay
    const float resampleIndex = (pBuffer->index - currentDelay_sample) + lfoSample * CHORUS_RESAMPLE_DELTA;
    const float resampleFactor = resampleIndex - floorf(resampleIndex);
    const uint16_t roundedResampleIndex = roundf(resampleIndex);
        
    chorus_t_LFO += CHORUS_LFO_DT;

    return pBuffer->pData[pBuffer->index] * (1.0f - wet) + (pBuffer->pData[roundedResampleIndex] * (1.0f - resampleFactor) + pBuffer->pData[roundedResampleIndex + 1] * resampleFactor) * wet;
}

void SFX_Equalizer(volatile AudioBuffer* pInputBuffer, volatile AudioBuffer* pOutputBuffer)
{
    uint32_t i;
    for (i = 0; i < FFT_SIZE; i++)
    {
        pComplexBuffer[i].re = UINT24_TO_FLOAT(pInputBuffer->pData[(pInputBuffer->index + RENDER_BUFFER_TRANSMIT_START_INDEX + i) % CAPTURE_BUFFER_FRAME_COUNT]);
        pComplexBuffer[i].im = 0.0f;
    }

    arm_cfft_f32(&EQ_CFFT_STRUCT, (float*)&pComplexBuffer->re, 0, 1);

    volatile const float bass = AUDIO_CONTROLS_NORMALIZE(audioControls.bass);
    volatile const float low_mid = AUDIO_CONTROLS_NORMALIZE(audioControls.low_mid);
    volatile const float high_mid = AUDIO_CONTROLS_NORMALIZE(audioControls.high_mid);
    volatile const float treble = AUDIO_CONTROLS_NORMALIZE(audioControls.treble);

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
    
    arm_cfft_f32(&EQ_CFFT_STRUCT, (float*)&pComplexBuffer->re, 1, 1);

    for (i = 0; i < FFT_SIZE; i++)
    {
        pOutputBuffer->pData[i + RENDER_BUFFER_TRANSMIT_START_INDEX] += FLOAT_TO_UINT24(pComplexBuffer[i].re * hannBuffer[i] / FFT_SIZE);
    }
}