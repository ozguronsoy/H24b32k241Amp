#include "Fourier.h"

#if !defined(MATH_H)
#include <math.h>
#define MATH_H
#endif

#define LOG2_FFT_SIZE 12 // log2(FFT_SIZE)

void ReverseBits(Complex* pComplexBuffer)
{
    uint32_t i, j = 0;
    Complex temp;
    for (i = 0; i < FFT_SIZE; i++)
    {
        if (i < j)
        {
            temp = pComplexBuffer[j];
            pComplexBuffer[j] = pComplexBuffer[i];
			pComplexBuffer[i] = temp;
        }
        j ^= FFT_SIZE - FFT_SIZE / ((i ^ (i + 1)) + 1);
    }
}

Complex ComplexMultipication(Complex a, Complex b)
{
    Complex result;
    result.re = a.re * b.re - a.im * b.im;
    result.im = a.im * b.re + a.re * b.im;
    return result;
}

void FFT(Complex* pComplexBuffer)
{
    ReverseBits(pComplexBuffer);
    
    Complex a = {-1.0f, 0.0f}, b, temp;
    uint32_t s, s2, i, j, k;

    for (i = 0; i < LOG2_FFT_SIZE; i++)
    {
        s = 1 << i;
        s2 = s << 1;
        b.re = 1.0f;
        b.im = 0.0f;

        for (j = 0; j < s; j++)
        {
            for (k = j; k < FFT_SIZE; k += s2)
            {
                temp = ComplexMultipication(b, pComplexBuffer[k + s]);
                pComplexBuffer[k + s].re = pComplexBuffer[k].re - temp.re;
                pComplexBuffer[k + s].im = pComplexBuffer[k].im - temp.im;
                pComplexBuffer[k].re += temp.re;
                pComplexBuffer[k].im += temp.im;
            }

            b = ComplexMultipication(a, b);
        }

        a.im = -sqrtf((1.0f - a.re) * 0.5f);
        a.re = sqrt((1.0f + a.re) * 0.5f);
    }
}

void IFFT(Complex* pComplexBuffer)
{
    ReverseBits(pComplexBuffer);
    
    Complex a = {-1.0f, 0.0f}, b, temp;
    uint32_t s, s2, i, j, k;

    for (i = 0; i < LOG2_FFT_SIZE; i++)
    {
        s = 1 << i;
        s2 = s << 1;
        b.re = 1.0f;
        b.im = 0.0f;

        for (j = 0; j < s; j++)
        {
            for (k = j; k < FFT_SIZE; k += s2)
            {
                temp = ComplexMultipication(b, pComplexBuffer[k + s]);
                pComplexBuffer[k + s].re = pComplexBuffer[k].re - temp.re;
                pComplexBuffer[k + s].im = pComplexBuffer[k].im - temp.im;
                pComplexBuffer[k].re += temp.re;
                pComplexBuffer[k].im += temp.im;
            }

            b = ComplexMultipication(a, b);
        }

        a.im = sqrtf((1.0f - a.re) * 0.5f);
        a.re = sqrt((1.0f + a.re) * 0.5f);
    }
}