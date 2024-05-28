#include <stdio.h>
#include <assert.h>

#include "avs3_prot_com.h"
#include "avs3_cnst_com.h"
#include "avs3_stat_com.h"
#include "avs3_rom_com.h"


static void reorder(float *x, const short size)
{
    short i;
    const unsigned short *r = NULL;

    switch (size)
    {
    case FFT_TABLE_SIZE64:
        r = fft_reordertbl64;
        break;
    case FFT_TABLE_SIZE256:
        r = fft_reordertbl256;
        break;
    case FFT_TABLE_SIZE512:
        r = fft_reordertbl512;
        break;
    default:
        break;
    }

    for (i = 0; i < size; i++)
    {
        int j = r[i];
        float tmp;

        if (j <= i) continue;

        tmp = x[i];
        x[i] = x[j];
        x[j] = tmp;
    }
}

static void fft_proc(float *xr, float *xi, const float *refac, const float *imfac, const short size)
{
    int step, shift, pos;
    int exp, estep;

    estep = size;
    for (step = 1; step < size; step *= 2)
    {
        int x1;
        int x2 = 0;
        estep >>= 1;
        for (pos = 0; pos < size; pos += (2 * step))
        {
            x1 = x2;
            x2 += step;
            exp = 0;
            for (shift = 0; shift < step; shift++)
            {
                float v2r, v2i;

                v2r = xr[x2] * refac[exp] - xi[x2] * imfac[exp];
                v2i = xr[x2] * imfac[exp] + xi[x2] * refac[exp];

                xr[x2] = xr[x1] - v2r;
                xr[x1] += v2r;

                xi[x2] = xi[x1] - v2i;

                xi[x1] += v2i;

                exp += estep;

                x1++;
                x2++;
            }
        }
    }
}

//todo.. optmize by common fft,replace origin tables ..(it's ok with some accuracy error)
void FFT(float *xr, float *xi, const short size)
{
    assert(size == FFT_TABLE_SIZE512 || size == FFT_TABLE_SIZE64 || size == FFT_TABLE_SIZE256);

    const float* cosTable = NULL;
    const float* sinTable = NULL;

    reorder(xr, size);
    reorder(xi, size);

    switch (size)
    {
    case FFT_TABLE_SIZE64:
        cosTable = fft_cos_table32;
        sinTable = fft_sin_table32;
        break;
    case FFT_TABLE_SIZE256:
        cosTable = fft_cos_table128;
        sinTable = fft_sin_table128;
        break;
    case FFT_TABLE_SIZE512:
        cosTable = fft_cos_table256;
        sinTable = fft_sin_table256;
        break;
    default:
        break;
    }

    fft_proc(xr, xi, cosTable, sinTable, size);
}

void IFFT(float *xr, float *xi, const short size)
{
    short i;
    float fac;
    float *xrp, *xip;

    FFT(xi, xr, size);

    fac = 1.0f / size;
    xrp = xr;
    xip = xi;

    for (i = 0; i < size; i++)
    {
        *xrp++ *= fac;
        *xip++ *= fac;
    }
}