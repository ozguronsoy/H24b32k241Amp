#if !defined(SOUND_EFFECTS_H)
#include "SoundEffects.h"
#define SOUND_EFFECTS_H
#endif

#if !defined (AUDIO_CONTROLS_H)
#include "AudioControls.h"
#define AUDIO_CONTROLS_H
#endif

#if !defined(FOURIER_H)
#include "Fourier.h"
#define FOURIER_H
#endif

#if !defined(LED_CONTROLS_H)
#include "LEDControls.h"
#define LED_CONTROLS_H
#endif

#if !defined(MATH_H)
#include <math.h>
#define MATH_H
#endif

#if !defined(STDLIB_H)
#include <stdlib.h>
#define STDLIB_H
#endif

#define INPUT_RANGE(sample) ((sample) * 2.0f - 1.0f) // [0, 1] -> [-1, 1]
#define OUTPUT_RANGE(sample) (((sample) + 1.0f) * 0.5f) // [-1, 1] -> [0, 1]

#define DISTORTION_LOWER_BOUNDARY(distortion) (0.3f * distortion)
#define DISTORTION_UPPER_BOUNDARY(distortion) (0.7f * distortion)


#define CHORUS_BASE_DELAY_MS 3.0f
#define CHORUS_BASE_DELAY_SAMPLE (CHORUS_BASE_DELAY_MS * 1e-3f * ((float)SAMPLE_RATE)) // the delay will be between [BASE_DELAY_MS, (BASE_DELAY_MS + DELAY_MS)]
#define CHORUS_DELAY_MS 30.0f
#define CHORUS_DELAY_SAMPLE (CHORUS_DELAY_MS * 1e-3f * ((float)SAMPLE_RATE))
#define CHORUS_RESAMPLE_DELTA 0 // fs * (pow(2, semitone / 12) - 1.0)


#define EQ_NYQUIST (FFT_SIZE * 0.5f)
#define EQ_HANN_WINDOW(index) (sinf(PI * index / (OUTPUT_BUFFER_FRAME_COUNT - 1)))
#define EQ_BIN_START_L_MID 7    // 160 Hz
#define EQ_BIN_START_H_MID 31   // 720 Hz
#define EQ_BIN_START_TREBLE 55  // 1280 Hz



volatile uint32_t chorus_LFO_index = 0;


float SFX_Distortion(float sample)
{
    volatile const float d = audioControls.distortion;
    
    const float u = DISTORTION_UPPER_BOUNDARY(d);
    if (sample > u)
    {
        return u;
    }

    const float l = DISTORTION_LOWER_BOUNDARY(d);
    if (sample < l)
    {
        return l;
    }

    return sample;
}

float SFX_Overdrive(float sample)
{
    volatile const float a = sinf(audioControls.overdrive * PI * 0.5f);
    const float k = 2.0f * a / (1.0f - a);

    float result = INPUT_RANGE(sample);
    result = (1.0f + k) * result / (1.0f + k * fabsf(result));
    return OUTPUT_RANGE(result);
}

float SFX_Chorus(float* pBuffer, uint16_t index)
{
    volatile const float rate = audioControls.chorus_rate;
    volatile const float wet = audioControls.chorus_depth * 0.5f;

    if (rate == 0 || wet == 0)
    {
        return pBuffer[index];
    }

    const float lfoSample = OUTPUT_RANGE(sinf(2.0f * PI * rate * ((float)chorus_LFO_index) / ((float)SAMPLE_RATE)));
    const uint16_t currentDelay_sample = roundf(lfoSample * CHORUS_DELAY_SAMPLE + CHORUS_BASE_DELAY_SAMPLE); // n samples of delay
    const float resampleIndex = (index - currentDelay_sample) + lfoSample * CHORUS_RESAMPLE_DELTA;
    const float resampleFactor = resampleIndex - floorf(resampleIndex);
    const uint16_t roundedResampleIndex = roundf(resampleIndex);
        
    chorus_LFO_index++;
    return pBuffer[index] * (1.0f - wet) + (pBuffer[roundedResampleIndex] * (1.0f - resampleFactor) + pBuffer[roundedResampleIndex + 1] * resampleFactor) * wet;
}

void SFX_Equalizer(float* pInputBuffer, float* pOutputBuffer, uint32_t inputBufferStartIndex)
{
    Complex* pComplexBuffer = (Complex*)malloc(FFT_SIZE * sizeof(Complex));
    if (pComplexBuffer == NULL)
    {
        DEBUG_PRINT("Insufficient memory! (EQ)");
        CHANGE_LED_COLOR(LED_COLOR_ERROR);
        return;
    }

    uint32_t i;
    for (i = 0; i < FFT_SIZE; i++)
    {
        pComplexBuffer[i].re = pInputBuffer[(inputBufferStartIndex + i) % INPUT_BUFFER_FRAME_COUNT];
        pComplexBuffer[i].im = 0.0f;
    }

    FFT(pComplexBuffer);

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

    IFFT(pComplexBuffer);
    
    for (i = 0; i < FFT_SIZE; i++)
    {
        pOutputBuffer[i] += pComplexBuffer[i].re * EQ_HANN_WINDOW(i) / FFT_SIZE;
    }
    
    free(pComplexBuffer);
}