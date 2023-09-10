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

#define INPUT_RANGE(sample) (((sample)*2.0f) - 1.0f)    // [0, 1] -> [-1, 1]
#define OUTPUT_RANGE(sample) (((sample) + 1.0f) * 0.5f) // [-1, 1] -> [0, 1]

#define DISTORTION_LOWER_BOUNDARY(distortion) ((distortion) * (0.3f * UINT24_MAX))
#define DISTORTION_UPPER_BOUNDARY(distortion) ((distortion) * (0.7f * UINT24_MAX))

#define OVERDRIVE_ORIGIN (PROCESS_BUFFER_FRAME_COUNT / 2)

#define CHORUS_BASE_DELAY_MS 3u
#define CHORUS_BASE_DELAY_SAMPLE (CHORUS_BASE_DELAY_MS * SAMPLE_RATE / 1000u) // the delay will be between [BASE_DELAY_MS, (BASE_DELAY_MS + DELAY_MS)]
#define CHORUS_DELAY_MS 30u
#define CHORUS_DELAY_SAMPLE (CHORUS_DELAY_MS * SAMPLE_RATE / 1000u)
// fs * (pow(2, semitone / 12) - 1.0)
// 0.75 semitone
#define CHORUS_RESAMPLE_DELTA 1416u

float distortion_upper_boundary = 0.0f;
float distortion_lower_boundary = 0.0f;

float overdrive_wet = 0.0f;
float overdrive_dry = 0.0f;

uint64_t chorus_n = 0;
float chorus_rate = 0.0f;
float chorus_wet = 0.0f;
float chorus_dry = 0.0f;

void SFX_PrepareDistortion()
{
    const float distortion = AUDIO_CONTROLS_NORMALIZE(audioControls.distortion);
    distortion_lower_boundary = DISTORTION_LOWER_BOUNDARY(distortion);
    distortion_upper_boundary = DISTORTION_UPPER_BOUNDARY(distortion);
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
    const float lfoSample = (arm_sin_f32(2.0f * PI * chorus_rate * chorus_n / SAMPLE_RATE) + 1.0f) * 0.5f;
    const uint16_t currentDelay_sample = roundf(lfoSample * (CHORUS_DELAY_SAMPLE) + (CHORUS_BASE_DELAY_SAMPLE)); // n samples of delay

    float resampleIndex = (pBuffer->index - currentDelay_sample) + lfoSample * CHORUS_RESAMPLE_DELTA;
    if (resampleIndex < 0)
    {
        resampleIndex = PROCESS_BUFFER_FRAME_COUNT - resampleIndex;
    }

    const uint16_t flooredResampleIndex = floorf(resampleIndex);
    const float resampleFactor = resampleIndex - flooredResampleIndex;

    return pBuffer->pData[pBuffer->index] * chorus_dry + (pBuffer->pData[flooredResampleIndex] * (1.0f - resampleFactor) + pBuffer->pData[flooredResampleIndex + 1] * resampleFactor) * chorus_wet;
}