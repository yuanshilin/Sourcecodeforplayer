#include <math.h>
#include "avs3_prot_com.h"

void IMDCT(float *signal, const short N)
{
    short i;

    float xi[MAX_FFT_TABLE_SIZE];
    float xr[MAX_FFT_TABLE_SIZE];

    float tempr, tempi, c, s, cold; /* temps for pre and post twiddle */
    const float freq = 2.0f * AVS3_PI / N;
    const float cfreq = (float)cos(freq);
    const float sfreq = (float)sin(freq);
    const float cosfreq8 = (float)cos(freq * FACTOR_TWIDDLE_SHORT);
    const float sinfreq8 = (float)sin(freq * FACTOR_TWIDDLE_SHORT);

    c = cosfreq8;
    s = sinfreq8;

    for (i = 0; i < (N >> 2); i++) {
        tempr = -signal[2 * i];
        tempi = signal[(N >> 1) - 1 - 2 * i];

        xr[i] = tempr * c - tempi * s;
        xi[i] = tempi * c + tempr * s;

        cold = c;
        c = c * cfreq - s * sfreq;
        s = s * cfreq + cold * sfreq;
    }

    /* Perform in-place complex IFFT of length N/4 */
    switch (N) {
    case BLOCK_LEN_SHORT * 2:
        IFFT(xr, xi, FFT_TABLE_SIZE64);
        break;
    case BLOCK_LEN_HALF_LONG * 2:
        IFFT(xr, xi, FFT_TABLE_SIZE256);
        break;
    case BLOCK_LEN_LONG * 2:
        IFFT(xr, xi, FFT_TABLE_SIZE512);
        break;
    default:
        break;
    }

    /* prepare for recurrence relations in post-twiddle */
    c = cosfreq8;
    s = sinfreq8;

    /* post-twiddle FFT output and then get output data */
    for (i = 0; i < (N >> 2); i++) {

        /* get post-twiddled FFT output  */
        tempr = NORM_MDCT_FACTOR * (xr[i] * c - xi[i] * s);
        tempi = NORM_MDCT_FACTOR * (xi[i] * c + xr[i] * s);

        /* fill in output values */
        signal[(N >> 1) + (N >> 2) - 1 - 2 * i] = tempr;
        if (i < (N >> 3))
        {
            signal[(N >> 1) + (N >> 2) + 2 * i] = tempr;
        }
        else
        {
            signal[2 * i - (N >> 2)] = -tempr;
        }

        signal[(N >> 2) + 2 * i] = tempi;
        if (i < (N >> 3))
        {
            signal[(N >> 2) - 1 - 2 * i] = -tempi;
        }
        else
        {
            signal[(N >> 2) + N - 1 - 2 * i] = tempi;
        }

        /* use recurrence to prepare cosine and sine for next value of i */
        cold = c;
        c = c * cfreq - s * sfreq;
        s = s * cfreq + cold * sfreq;
    }

    for (i = 0; i < N; i++) {
        signal[i] = (float)(signal[i] * sqrt(N));
    }
}

void MDCT(float *signal, float* output, const short N)
{
    short i, n;

    float xi[MAX_FFT_TABLE_SIZE];
    float xr[MAX_FFT_TABLE_SIZE];
    float tmpReal, tmpImg, c, s, cold;
    const float freq = 2.0f * AVS3_PI / N;
    const float cfreq = (float)cos(freq);
    const float sfreq = (float)sin(freq);
    const float cosfreq8 = (float)cos(freq * FACTOR_TWIDDLE_SHORT);
    const float sinfreq8 = (float)sin(freq * FACTOR_TWIDDLE_SHORT);

    c = cosfreq8;
    s = sinfreq8;

    for (i = 0; i < (N >> 2); i++)
    {
        n = (N >> 1) - 1 - 2 * i;

        if (i < (N >> 3))
        {
            tmpReal = signal[(N >> 2) + n] + signal[N + (N >> 2) - 1 - n];
        }
        else
        {
            tmpReal = signal[(N >> 2) + n] - signal[(N >> 2) - 1 - n];
        }

        n = 2 * i;
        if (i < (N >> 3))
        {
            tmpImg = signal[(N >> 2) + n] - signal[(N >> 2) - 1 - n];
        }
        else
        {
            tmpImg = signal[(N >> 2) + n] + signal[N + (N >> 2) - 1 - n];
        }

        /* calculate pre-twiddled FFT input */
        xr[i] = tmpReal * c + tmpImg * s;
        xi[i] = tmpImg * c - tmpReal * s;

        /* use recurrence to prepare cosine and sine for next value of i */
        cold = c;
        c = c * cfreq - s * sfreq;
        s = s * cfreq + cold * sfreq;
    }

    switch (N) {
    case BLOCK_LEN_SHORT * 2:
        FFT(xr, xi, FFT_TABLE_SIZE64);
        break;
    case BLOCK_LEN_HALF_LONG * 2:
        FFT(xr, xi, FFT_TABLE_SIZE256);
        break;
    case BLOCK_LEN_LONG * 2:
        FFT(xr, xi, FFT_TABLE_SIZE512);
        break;
    default:
        break;
    }

    c = cosfreq8;
    s = sinfreq8;

    for (i = 0; i < (N >> 2); i++)
    {
        tmpReal = (float)(1.f / NORM_MDCT_FACTOR) * (xr[i] * c + xi[i] * s);
        tmpImg = (float)(1.f / NORM_MDCT_FACTOR) * (xi[i] * c - xr[i] * s);

        signal[2 * i] = -tmpReal;
        signal[(N >> 1) - 1 - 2 * i] = tmpImg; 
        signal[(N >> 1) + 2 * i] = -tmpImg; 
        signal[N - 1 - 2 * i] = tmpReal;

        cold = c;
        c = c * cfreq - s * sfreq;
        s = s * cfreq + cold * sfreq;
    }

    /* Output data */
    for (i = 0; i < N >> 1; i++)
    {
        output[i] = (float)(signal[i] / sqrt(N));
    }
}