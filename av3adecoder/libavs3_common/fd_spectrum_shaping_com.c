
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "avs3_options.h"
#include "avs3_debug.h"
#include "avs3_cnst_com.h"
#include "avs3_rom_com.h"
#include "avs3_prot_com.h"


/*
compute the LSP coefficients from the quanted LSF coefficients
I/O params:
    const float *lsf                          (i)   quanted LSF coefficients
    float *lsp                                (o)   LSP coefficients
    const short lpcOrder                      (i)   LSP order
    const int samplingRate                    (i)   the sampling rate of the input signal
*/
void LsfToLsp(
    const float *lsf, 
    float *lsp, 
    const short lpcOrder, 
    const int samplingRate
)
{
    short i;

    /* convert LSFs to LSPs */
    for (i = 0; i < lpcOrder; i++) {
        lsp[i] = (float)cos(lsf[i] * AVS3_PI / (samplingRate / 2));
    }

    return;
}


/*
find the polynomial F1(z) or F2(z) from the LSPs
I/O params:
    const float lsp[]                               (i)   LSP coefficients
    float polynomialCoffs[]                         (o)   polynomial coefficients
    const short numCoffs                            (i)   the order of polynomial coefficients
    short flag                                      (i)   the polynomial number (F1(z) or F2(z))
*/
static void GetLsppol(
    const float lsp[], 
    float polynomialCoffs[], 
    const short numCoffs, 
    short flag
)                          
{
    float tmpf;
    const float *posLsp;
    short i, j;

    posLsp = lsp + flag - 1;

    polynomialCoffs[0] = 1.0f;
    tmpf = -2.0f * *posLsp;
    polynomialCoffs[1] = tmpf;

    for (i = 2; i <= numCoffs; i++) {
        posLsp += 2;
        tmpf = -2.0f * *posLsp;
        polynomialCoffs[i] = tmpf * polynomialCoffs[i - 1] + 2.0f * polynomialCoffs[i - 2];

        for (j = i - 1; j > 1; j--) {
            polynomialCoffs[j] += tmpf * polynomialCoffs[j - 1] + polynomialCoffs[j - 2];
        }

        polynomialCoffs[1] += tmpf;
    }

    return;
}


/*
compute LPC parameters from the quantized LSP parameters
I/O params:
    const float *lsp                          (i)   LSP coefficients
    float *lpc                                (o)   LPC coefficients
    const short lpcOrder                      (i)   LSP order
*/
void LspToLpc(
    const float *lsp, 
    float *lpc, 
    const short lpcOrder
)
{
    float poly1[LSP_ROOT_NUM + 1], poly2[LSP_ROOT_NUM + 1];
    short i, k, nc;
    float *pPoly1, *pPoly2, *pPoly1Tmp, *pPoly2Tmp, *pLpc1, *pLpc2;

    nc = lpcOrder / 2;

    /* -----------------------------------------------------*
    * Find the polynomials F1(z) and F2(z)               *
    * ----------------------------------------------------- */

    GetLsppol(lsp, poly1, nc, 1);
    GetLsppol(lsp, poly2, nc, 2);

    /* -----------------------------------------------------*
    * Multiply F1(z) by (1+z^-1) and F2(z) by (1-z^-1)   *
    * ----------------------------------------------------- */

    pPoly1 = poly1 + nc;
    pPoly1Tmp = pPoly1 - 1;
    pPoly2 = poly2 + nc; /* Version using indices            */
    pPoly2Tmp = pPoly2 - 1;
    k = nc - 1;
    for (i = 0; i <= k; i++) {
        *pPoly1-- += *pPoly1Tmp--;
        *pPoly2-- -= *pPoly2Tmp--;
    }

    /* -----------------------------------------------------*
    * A(z) = (F1(z)+F2(z))/2                             *
    * F1(z) is symmetric and F2(z) is antisymmetric      *
    * ----------------------------------------------------- */

    pLpc1 = lpc;
    *pLpc1++ = 1.0;
    pLpc2 = lpc + lpcOrder;
    pPoly1 = poly1 + 1;
    pPoly2 = poly2 + 1;
    for (i = 0; i <= k; i++) {
        *pLpc1++ = 0.5f * (*pPoly1 + *pPoly2);
        *pLpc2-- = 0.5f * (*pPoly1++ - *pPoly2++);
    }

    return;
}

/*
compute spectrum shaping gain in frequency domain
I/O params:
    const float *weightedLpcCoeffs                  (i)   LPC coefficients
    float *lpcGain                                  (o)   spectrum shaping gain in frequency domain
    const short lpcOrder                            (i)   LSP order
*/
static void GetLpcGain(
    const float *weightedLpcCoeffs, 
    float *lpcGain, 
    const short lpcOrder
)
{
    short i, j, n, fftLen;
    float tmp;
    float realPart[FFT_TABLE_SIZE512];
    float imagPart[FFT_TABLE_SIZE512];
    float rawLpcGain[FFT_TABLE_SIZE512];
    float interpLpcGain[BLOCK_LEN_LONG];

    // ratio of interpolation, curr 4
    n = BLOCK_LEN_LONG / (FFT_TABLE_SIZE512 / 2);
    // fft length, 512
    fftLen = FFT_TABLE_SIZE512;

    // rotation of LPC coefficients
    for (i = 0; i < lpcOrder + 1; i++) {
        tmp = (float)(((float)i) * AVS3_PI / (float)(fftLen));
        realPart[i] = (float)(weightedLpcCoeffs[i] * cos(tmp));
        imagPart[i] = (float)(-weightedLpcCoeffs[i] * sin(tmp));
    }
    for (; i < fftLen; i++) {
        realPart[i] = 0.f;
        imagPart[i] = 0.f;
    }

    // perform fft on zero padded lpc with rotation
    FFT(realPart, imagPart, FFT_TABLE_SIZE512);

    // raw lpc gain
    for (i = 0; i < fftLen; i++) {
        rawLpcGain[i] = (float)(1.0f / sqrt(realPart[i] * realPart[i] + imagPart[i] * imagPart[i]));
    }

    // interploation of lpc gain, from 256 points to 1024 points
    // linear interpolation
    for (i = 0; i < fftLen / 2; i++) {
        interpLpcGain[n * i] = rawLpcGain[i];
        interpLpcGain[n * i + 1] = rawLpcGain[i] + (rawLpcGain[i + 1] - rawLpcGain[i]) / n;
        interpLpcGain[n * i + 2] = rawLpcGain[i] + 2.0f * ((rawLpcGain[i + 1] - rawLpcGain[i]) / n);
        interpLpcGain[n * i + 3] = rawLpcGain[i] + 3.0f * ((rawLpcGain[i + 1] - rawLpcGain[i]) / n);
    }

    // get lpc gain for each sfb
    for (i = 0; i < N_SFB_FB_LONG; i++) {

        // sum interpolated gain in each sfb
        tmp = 0.0f;
        for (j = sfb_table_fb_long[i]; j < sfb_table_fb_long[i + 1]; j++) {
            tmp += interpLpcGain[j];
        }

        // get averaged gain in sfb
        tmp /= (float)sfb_len_fb_long[i];

        // set averaged gain to each bin of current sfb
        for (j = sfb_table_fb_long[i]; j < sfb_table_fb_long[i + 1]; j++) {
            lpcGain[j] = tmp;
        }
    }

    return;
}


/*
shaping the input signal in frequency domain
I/O params:
    float *mdctSpectrum                             (i/o) the orignal/shaped MDCT coefficients of the input siganl
    float *lpcQuantCoeffs                           (i)   recovered LPC coefficients
    float *lpcGain                                  (i/o) spectrum shaping gain in frequency domain
    const short lpcOrder                            (i)   LSP order
    const short len                                 (i)   frame length
    const short noInverse                           (i)   spectrum shaping gain in encoder/decoder state
*/
void SpectrumShaping(
    float *mdctSpectrum, 
    float *lpcQuantCoeffs, 
    float *lpcGain, 
    const short lpcOrder, 
    const short len, 
    const short noInverse
)
{
    short i, mdctGainLen;
    float gammaLpc, weightedLpcQuantCoeffs[LPC_ORDER + 1];
    float weightedFactorBuf[LPC_ORDER + 1] = {0.0f};

    /* weighting the quantized LPC coefficients */
    gammaLpc = GAMMA_LPC;
    weightedFactorBuf[0] = 1.0f;
    for (i = 1; i <= lpcOrder; i++)
    {
        weightedFactorBuf[i] = weightedFactorBuf[i - 1] * gammaLpc;
    }
    for (i = 0; i <= lpcOrder; i++)
    {
        weightedLpcQuantCoeffs[i] = weightedFactorBuf[i] * lpcQuantCoeffs[i];
    }

    /* calculate lpc gain */
    mdctGainLen = FFT_TABLE_SIZE512 / 2;
    SetZero(lpcGain, BLOCK_LEN_LONG);
    GetLpcGain(weightedLpcQuantCoeffs, lpcGain, LPC_ORDER);

    if (noInverse) {
        /* shaping the original mdct spectrum */
        for (i = 0; i < BLOCK_LEN_LONG; i++) {
            mdctSpectrum[i] = mdctSpectrum[i] / lpcGain[i];
        }
    }
    else {
        /* shaping the quantized mdct spectrum */
        for (i = 0; i < BLOCK_LEN_LONG; i++) {
            mdctSpectrum[i] = mdctSpectrum[i] * lpcGain[i];
        }
    }

    return;
}
