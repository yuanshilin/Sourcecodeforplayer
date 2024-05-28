/* ==========================================================================
  Copyright 2023 HUAWEI TECHNOLOGIES CO., LTD.
  Licensed under the Code Sharing Policy of the UHD World Association (the
  "Policy");
  http://www.theuwa.com/UWA_Code_Sharing_Policy.pdf.
  you may not use this file except in compliance with the Policy.
  Unless agreed to in writing, software distributed under the Policy is
  distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OF ANY KIND, either express or implied.
  See the Policy for the specific language governing permissions and
  limitations under the Policy.
========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <assert.h>

#include "avs3_options.h"
#include "avs3_debug.h"
#include "avs3_cnst_com.h"
#include "avs3_cnst_enc.h"
#include "avs3_rom_com.h"
#include "avs3_prot_com.h"
#include "avs3_prot_enc.h"


/*
preemphasis the windowed input signal
I/O params:
    float *winSignal                                (i/o) original/weighted input signal
    const short len                                 (i)   frame length
    float *lastSignalPoint                          (i/o)  last point of the weighted input signal
*/
static void Preemph(
    float *winSignal, 
    const short len, 
    float *lastSignalPoint
)
{
    short i;
    float preemphFactorFb, tmp;

    tmp = winSignal[len - 1];
    preemphFactorFb = PREEMPH_FAC_FB;
    for (i = len - 1; i >= 1; i--)
    {
        winSignal[i] = winSignal[i] - preemphFactorFb * winSignal[i - 1];
    }

    *lastSignalPoint = tmp;

    return;
}


/*
compute autocorrelation coefficients
I/O params:
    float *winSignal                                (i)   weighted input signal
    double *autoCorr                                (o)   autocorrelation coefficients
    const short lpcOrder                            (i)   order of autocorrelation coefficients
    const short len                                 (i)   length of input signal
*/
static void GetAutoCorr(
    float *winSignal, 
    double *autoCorr, 
    const short lpcOrder, 
    const short len
)
{
    double t[BLOCK_LEN_LONG + BLOCK_LEN_LONG + LPC_ORDER];
    int i, j;

    for (i = 0; i < len; i += 4) {
        t[i] = winSignal[i];
        t[i + 1] = winSignal[i + 1];
        t[i + 2] = winSignal[i + 2];
        t[i + 3] = winSignal[i + 3];
    }

    memset(&t[BLOCK_LEN_LONG + BLOCK_LEN_LONG], 0, LPC_ORDER * sizeof(double));
    memset(autoCorr, 0, (LPC_ORDER + 1) * sizeof(double));

    for (j = 0; j < len; j++) {
        for (i = 0; i < lpcOrder + 1; i++) {
            autoCorr[i] += (t[j] * t[j + i]);
        }
    }

    if (autoCorr[0] < 1.0f) {
        autoCorr[0] = 1.0f;
    }

    return;
}


/*
compute LP parameters from the autocorrelations based on Wiener-Levinson-Durbin algorithm
I/O params:
    double *autoCorr                                (i)   input autocorrelation coefficients
    double *lpc                                     (o)   LPC coefficients
    const short lpcOrder                            (i)   order of  LPC coefficients
*/
static short GetLpcFromAutocorr(
    double *autoCorr, 
    float *lpc, 
    const short lpcOrder
)
{
    short i, j, k;
    double lpcCoeffs[LPC_ORDER + 1];
    double buffer[LPC_ORDER + 1];
    double *reflectCoef;                /* reflection coefficients  0,...,order-1 */
    double s, at, err;
    short flag = 0;

    reflectCoef = &buffer[0];
    reflectCoef[0] = (-autoCorr[1]) / autoCorr[0];
    lpcCoeffs[0] = 1.0f;
    lpcCoeffs[1] = reflectCoef[0];
    err = autoCorr[0] + autoCorr[1] * reflectCoef[0];

    for (i = 2; i <= lpcOrder; i++) {
        s = 0.0f;
        for (j = 0; j < i; j++) {
            s += autoCorr[i - j] * lpcCoeffs[j];
        }

        reflectCoef[i - 1] = (-s) / err;

        if (fabs(reflectCoef[i - 1]) > 0.99945f) {
            flag = 1;                               /* Test for unstable filter. If unstable keep old A(z) */
        }

        for (j = 1; j <= i / 2; j++) {
            k = i - j;
            at = lpcCoeffs[j] + reflectCoef[i - 1] * lpcCoeffs[k];
            lpcCoeffs[k] += reflectCoef[i - 1] * lpcCoeffs[j];
            lpcCoeffs[j] = at;
        }

        lpcCoeffs[i] = reflectCoef[i - 1];

        err += reflectCoef[i - 1] * s;
        if (err <= 0.0f) {
            err = 0.01f;
        }
    }

    for (i = 0; i <= lpcOrder; i++) {
        lpc[i] = (float)lpcCoeffs[i];
    }

    return flag;
}


/* Evaluates the Chebyshev polynomial series */
static float Chebps2(
    const float x, 
    const float *f, 
    const short n
)
{
    float b1, b2, b0, x2;
    short i;

    x2 = (float)(2.0f * x);
    b2 = f[0];

    b1 = x2 * b2 + f[1];

    for (i = 2; i < n; i++) {
        b0 = x2 * b1 - b2 + f[i];
        b2 = b1;
        b1 = b0;
    }

    return (float)(x * b1 - b2 + 0.5f * f[n]);
}


/*
compute LSP parameters from the LPC coefficients
I/O params:
    float *lpc                                      (i)   input LPC coefficients
    float *oldLsp                                   (i)   input LSP coefficients of the last frame
    float *lsp                                      (o)   LSP coefficients of the current frame
    const short lpcOrder                            (i)   order of  LPC coefficients
*/
static void LpcToLsp(
    float *lpc, 
    float *oldLsp, 
    float *lsp, 
    const short lpcOrder
)
{
    short j, i, numFoundFreqs, firstPolyFlag;
    float xLow, yLow, xHigh, yHigh, xMid, yMid, xInt;
    float *pPoly1, *pPoly2;
    const float *pa1, *pa2;
    float poly1[LSP_ROOT_NUM + 1], poly2[LSP_ROOT_NUM + 1];

    pPoly1 = poly1;                 /* Equivalent code using indices   */
    pPoly2 = poly2;
    *pPoly1++ = 1.0f;               /* f1[0] = 1.0;                    */
    *pPoly2++ = 1.0f;               /* f2[0] = 1.0;                    */
    pa1 = lpc + 1;
    pa2 = lpc + lpcOrder;

    for (i = 0; i <= LSP_ROOT_NUM - 1; i++) {               /* for (i=1, j=M; i<=NC; i++, j--) */
        *pPoly1 = *pa1 + *pa2 - *(pPoly1 - 1);              /* f1[i] = a[i]+a[j]-f1[i-1];   */
        *pPoly2 = *pa1++ - *pa2-- + *(pPoly2 - 1);          /* f2[i] = a[i]-a[j]+f2[i-1];   */
        pPoly1++;
        pPoly2++;
    }

    numFoundFreqs = 0;                          /* number of found frequencies */
    firstPolyFlag = 0;                          /* flag to first polynomial   */
    pPoly1 = poly1;                             /* start with F1(z) */
    xLow = grid100[0];
    yLow = Chebps2(xLow, pPoly1, LSP_ROOT_NUM);
    j = 0;

    while ((numFoundFreqs < lpcOrder) && (j < GRID_100_POINTS)) {

        j++;
        xHigh = xLow;
        yHigh = yLow;
        xLow = grid100[j];
        yLow = Chebps2(xLow, pPoly1, LSP_ROOT_NUM);

        /* if sign change new root exists */
        if (yLow * yHigh <= 0.0) {
            j--;

            /* divide the interval of sign change by NUM_ITER */
            for (i = 0; i < NUM_ITER; i++) {
                xMid = 0.5f * (xLow + xHigh);
                yMid = Chebps2(xMid, pPoly1, LSP_ROOT_NUM);
                if (yLow * yMid > 0.0) {
                    yLow = yMid;
                    xLow = xMid;
                }
                else {
                    yHigh = yMid;
                    xHigh = xMid;
                }
            }

            /* linear Interpolation for evaluating the root */
            xInt = xLow - yLow * (xHigh - xLow) / (yHigh - yLow);
            lsp[numFoundFreqs] = xInt;                        /* new root */
            numFoundFreqs++;
            firstPolyFlag = 1 - firstPolyFlag;                      /* flag to other polynomial    */
            pPoly1 = firstPolyFlag ? poly2 : poly1;                 /* pointer to other polynomial */
            xLow = xInt;
            yLow = Chebps2(xLow, pPoly1, LSP_ROOT_NUM);
        }
    }

    /* Check if M roots found */
    /* if not use the LSPs from previous frame */
    if (numFoundFreqs < lpcOrder) {
        Mvf2f(oldLsp, lsp, lpcOrder);
    }

    return;
}

/*
compute LSF parameters using the LSP coefficients
I/O params:
    const float *lsp                          (i)   input LSP coefficients
    float *lsf                                (o)   output LSF parameters
    const short lpcOrder                      (i)   order of  LSP coefficients
    const int samplingRate                    (i)   the sampling rate of the input signals
*/
static void LspToLsf(
    const float *lsp, 
    float *lsf, 
    const short lpcOrder, 
    const int samplingRate
)
{
    short i;

    /* convert LSPs to LSFs */
    for (i = 0; i < lpcOrder; i++) {
        lsf[i] = (float)(acos(lsp[i]) * ((samplingRate / 2) / AVS3_PI));
    }

    return;
}


/*
LPC analysis and spectrum shaping
I/O params:
    AVS3_ENC_CORE_HANDLE hEncCore               (i/o) core coder handle, contains spectrum and lpc/lsf info
    int16_t chIdx                               (i)   channel index, for debug purpose
*/
void Avs3FdSpectrumShaping(
    AVS3_ENC_CORE_HANDLE hEncCore,
    int16_t chIdx
)
{
    short len, flag;

    float preemphWinSignal[BLOCK_LEN_LONG + BLOCK_LEN_LONG] = { 0.0f };
    double autoCorr[LPC_ORDER + 1] = { 0.0 };
    float lpc[LPC_ORDER + 1] = { 0.0f };
    float lpcQ[LPC_ORDER + 1] = { 0.0f };;
    float lsp[LPC_ORDER] = { 0.0f }; 
    float lspQ[LPC_ORDER] = { 0.0f };
    float lsf[LPC_ORDER] = { 0.0f };
    float lsfQ[LPC_ORDER] = { 0.0f };
    float lpcGain[BLOCK_LEN_LONG] = { 0.0f };

    len = BLOCK_LEN_LONG + BLOCK_LEN_LONG;

    /* preemphasis the input signal */
    Mvf2f(hEncCore->winSignal48kHz, preemphWinSignal, BLOCK_LEN_LONG + BLOCK_LEN_LONG);
    Preemph(preemphWinSignal, len, &hEncCore->memPreemph);

    /* calculate the auto-correlation of the weighted input signal */
    GetAutoCorr(preemphWinSignal, autoCorr, LPC_ORDER, len);

    /* compute LP parameters from the autocorrelations based on Wiener-Levinson-Durbin algorithm */
    flag = GetLpcFromAutocorr(autoCorr, lpc, LPC_ORDER);

    /* compute LSP parameters using the LP parameters */
    LpcToLsp(lpc, hEncCore->oldLsp, lsp, LPC_ORDER);

    /* compute LSF parameters using the LSP coefficients */
    LspToLsf(lsp, lsf, LPC_ORDER, AVS3_SAMPLING_48KHZ); 

    /* quantize LSF parameters*/
    LsfQuantEnc(lsf, lsfQ, hEncCore->lsfVqIndex, LPC_ORDER, hEncCore->lsfLbrFlag);

    /* compute quantized LSP parameters */
    LsfToLsp(lsfQ, lspQ, LPC_ORDER, AVS3_SAMPLING_48KHZ);

    /* compute quantized LP parameters using quantized LSP parameters */
    LspToLpc(lspQ, lpcQ, LPC_ORDER);

    /* perform spectrum shaping */
    SpectrumShaping(hEncCore->origSpectrum, lpcQ, lpcGain, LPC_ORDER, len, 1);

    /* update history */
    Mvf2f(lsp, hEncCore->oldLsp, LPC_ORDER);
    Mvf2f(lsfQ, hEncCore->lsfQ, LPC_ORDER);

    return;
}
