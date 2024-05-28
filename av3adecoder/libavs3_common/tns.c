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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_stat_com.h"
#include "avs3_rom_com.h"
#include "avs3_prot_com.h"


/*
Init tns parameters
I/O params:
    TnsData * pTnsData      (i) tns data structure
*/
void TnsParaInit(
    TnsData * pTnsData
) 
{
    int filterIdx;
    int orderIdx;

    TnsFilter *tnsFilter = NULL;
    TnsDetector *tnsDetector = NULL;
    TnsBsParam *tnsBsParam = NULL;

    /* cycle each filter */
    for (filterIdx = 0; filterIdx < TNS_MAX_FILTER_NUM; filterIdx++) {

        tnsFilter = &pTnsData->filter[filterIdx];
        tnsDetector = &pTnsData->tnsDetector[filterIdx];
        tnsBsParam = &pTnsData->bsParam[filterIdx];

        // Init filter param
        tnsFilter->order = 0;
        for (orderIdx = 0; orderIdx < TNS_MAX_FILTER_ORDER; orderIdx++) {
            tnsFilter->coefIndex[orderIdx] = 0;
        }

        // Init tns detector
        tnsDetector->avgSqrCoef = 0.0f;
        tnsDetector->predictionGain = 0.0f;

        // Init tns bitstream params
        tnsBsParam->enable = 0;
        tnsBsParam->order = 0;
        for (int16_t i = 0; i < TNS_MAX_FILTER_ORDER; i++) {
            tnsBsParam->parcorNbits[i] = 0;
            tnsBsParam->parcorHuffCode[i] = 0;
        }
    }
}


/*
clear tns filter data
I/O params:
    TnsFilter * pTnsFilter:    (i) tns filter structure
*/
void ClearTnsFilterCoefficients(
    TnsFilter * pTnsFilter
)
{
    pTnsFilter->order = 0;
    SetShort(pTnsFilter->coefIndex, 0, TNS_MAX_FILTER_ORDER);

    return;
}


/*
Huffman encoding of tns parcor
I/O params:
    TnsData * pTnsData      (i/o) tns data structure
*/
void TnsEncodeParam(
    TnsData * pTnsData
) 
{
    short indexVal, huffmanBits;
    unsigned short huffmanCode;
    int filterIdx, orderIdx;

    TnsFilter *tnsFilter = NULL;
    TnsBsParam *tnsBsParam = NULL;

    /* cycle each filter */
    for (filterIdx = 0; filterIdx < TNS_MAX_FILTER_NUM; filterIdx++) {

        tnsFilter = &pTnsData->filter[filterIdx];
        tnsBsParam = &pTnsData->bsParam[filterIdx];

        if (tnsBsParam->enable == 1) {

            // save order to bs param
            tnsBsParam->order = tnsFilter->order;

            /* cycle each order */
            for (orderIdx = 0; orderIdx < tnsBsParam->order; orderIdx++) {

                // huffman coding
                indexVal = tnsFilter->coefIndex[orderIdx];
                huffmanBits = tnsCodingTable[orderIdx][indexVal + INDEX_SHIFT].nBits;
                huffmanCode = tnsCodingTable[orderIdx][indexVal + INDEX_SHIFT].code;

                // save bsParam parameters
                // including number bits and huffman code
                tnsBsParam->parcorNbits[orderIdx] = huffmanBits;
                tnsBsParam->parcorHuffCode[orderIdx] = huffmanCode;
            }
        }
        else {

            // set order to 0
            tnsBsParam->order = 0;

            // clear bs param
            for (int16_t i = 0; i < TNS_MAX_FILTER_ORDER; i++) {
                tnsBsParam->parcorNbits[i] = 0;
                tnsBsParam->parcorHuffCode[i] = 0;
            }
        }
    }
}


/*
Huffman decoding of tns parcor
I/O params:
    TnsData * pTnsData      (i/o) tns data structure
*/
void TnsDecodeParam(
    TnsData * pTnsData
) 
{
    short i;
    short filterIdx, orderIdx;
    short codeIdx;

    TnsFilter *tnsFilter = NULL;
    TnsBsParam *tnsBsParam = NULL;

    for (filterIdx = 0; filterIdx < TNS_MAX_FILTER_NUM; filterIdx++) {

        tnsFilter = &pTnsData->filter[filterIdx];
        tnsBsParam = &pTnsData->bsParam[filterIdx];

        // clear filter param before decode
        ClearTnsFilterCoefficients(tnsFilter);

        // decode param to get filter coefficients when filter enabled
        if (tnsBsParam->enable == 1) {

            // set order
            tnsFilter->order = tnsBsParam->order;

            // loop over order
            for (orderIdx = 0; orderIdx < tnsFilter->order; orderIdx++) {

                // huffman decoding to get coefficient index
                codeIdx = -1;
                for (i = 0; i < N_TNS_COEFF_CODES; i++) {

                    if ((tnsBsParam->parcorHuffCode[orderIdx] == tnsCodingTable[orderIdx][i].code) &&
                        (tnsBsParam->parcorNbits[orderIdx] == tnsCodingTable[orderIdx][i].nBits)) {

                        codeIdx = i;
                        break;
                    }
                }
                tnsFilter->coefIndex[orderIdx] = tnsCodingTable[orderIdx][codeIdx].value - INDEX_SHIFT;
            }
        }
    }

    return;
}


/*
Get parcor from auto corr
I/O params:
    const float input[]     (i) auto correlation
    float parCoeff[]        (o) parcor coefficients
    const short order       (i) order of parcor
*/
static float AutoToParcor(
    const float input[],
    float parCoeff[],
    const short order
)
{
    short i, j;
    float tmp, tmp2;
    float workBuffer[2 * TNS_MAX_FILTER_ORDER];
    float * const pWorkBuffer = &workBuffer[order]; /* temp pointer */

    for (i = 0; i<order; i++) {
        workBuffer[i] = input[i];
        pWorkBuffer[i] = input[i + 1];
    }

    for (i = 0; i<order; i++) {

        if (workBuffer[0] < 1.0f / 65536.0f) {
            tmp = 0;
        }
        else {
            tmp = -pWorkBuffer[i] / workBuffer[0];
        }

        /* compensate for calculation inaccuracies limit reflection coefs to ]-1,1[ */
        tmp = min(0.999f, max(-0.999f, tmp));

        parCoeff[i] = tmp;
        for (j = i; j< order; j++) {
            tmp2 = pWorkBuffer[j] + tmp * workBuffer[j - i];
            workBuffer[j - i] += tmp * pWorkBuffer[j];
            pWorkBuffer[j] = tmp2;
        }
    }

    return ((input[0] + 1e-30f) / (workBuffer[0] + 1e-30f));
}


/*
Quantization of parcor
I/O params:
    const float parCoeff[]      (i) parcor coefficients
    short index[]               (o) quantization index
    const short order           (i) parcor order
*/
static void Parcor2Index(
    const float parCoeff[],
    short index[],
    const short order
)
{
    int const nValues = 1 << TNS_COEFF_RES;
    float const *parcorCb = tnsCoeff4;
    short i;
    short iIndex;
    float x;

    for (i = 0; i < order; i++)
    {
        iIndex = 1;
        x = parCoeff[i];

        assert((x >= -1.0f) && (x <= 1.0f));
        while ((iIndex < nValues) && (x > 0.5f*(parcorCb[iIndex - 1] + parcorCb[iIndex])))
        {
            ++iIndex;
        }
        index[i] = (iIndex - 1) - INDEX_SHIFT;
    }

    return;
}


/*
Map quantization index to parcor coefficient
I/O params:
    const short index[]         (i) parcor quantization index
    float parCoeff[]            (o) dequantized parcor
    const short order           (i) filter order
*/
static void Index2Parcor(
    const short index[],
    float parCoeff[],
    const short order
)
{
    float const * parcorCb = tnsCoeff4;
    short i;

    for (i = 0; i < order; i++) {
        parCoeff[i] = parcorCb[index[i] + INDEX_SHIFT];
    }

    return;
}


/*
Calculate TNS parcor coefficients and some dicision params from spectrum data
I/O params:
    float const pSpectrum[]         (i)   mdct spectrum data
    TnsData * pTnsData              (i/o) tns data structure
*/
void GetTnsData(
    float const pSpectrum[],
    TnsData * pTnsData
)
{
    short i, k, filterIdx, divisionIdx;
    short idx0, idx1;
    short iStartLine, iEndLine, lag;
    float fac;                                              // norm factor
    short maxOrder;                                         // max order of parcor

    float norms[TNS_MAX_FILTER_NUM][TNS_DIVISION_NUM];      // norm for each division
    float rxx[TNS_MAX_FILTER_ORDER + 1];                    // autocorr for each spectrum part
    float parCoeff[TNS_MAX_FILTER_ORDER];                   // parcor coefficient

    float const *parcorCb = tnsCoeff4;                      // parcor SQ codebook
    float value;

    TnsFilter *tnsFilter = NULL;
    TnsDetector *tnsDetector = NULL;

    for (i = 0; i<TNS_MAX_FILTER_NUM; i++)
    {
        SetFloat(norms[i], 0.0f, TNS_DIVISION_NUM);
        ClearTnsFilterCoefficients(&(pTnsData->filter[i]));
    }

    /* Calculate norms for each spectrum part */
    for (filterIdx = 0; filterIdx < TNS_MAX_FILTER_NUM; filterIdx++) {

        // start and stop line idx for each spectrum part
        idx0 = 2 * FRAME_LEN * FilterBorders[filterIdx][0] / AVS3_SAMPLING_48KHZ;
        idx1 = 2 * FRAME_LEN * FilterBorders[filterIdx][1] / AVS3_SAMPLING_48KHZ;

        // for each division
        for (divisionIdx = 0; divisionIdx < TNS_DIVISION_NUM; divisionIdx++) {

            // start and end line for each division
            iStartLine = idx0 + (idx1 - idx0) * divisionIdx / TNS_DIVISION_NUM;
            iEndLine = idx0 + (idx1 - idx0) * (divisionIdx + 1) / TNS_DIVISION_NUM;

            // calculate norm of each division
            norms[filterIdx][divisionIdx] = Dotp(pSpectrum + iStartLine, pSpectrum + iStartLine, iEndLine - iStartLine);
        }
    }

    /* Calculate normalized autocorrelation for spectrum subdivision and get TNS filter parameters based on it */
    for (filterIdx = 0; filterIdx < TNS_MAX_FILTER_NUM; filterIdx++)
    {
        tnsDetector = &pTnsData->tnsDetector[filterIdx];
        tnsFilter = &pTnsData->filter[filterIdx];

        // start and stop line idx for each spectrum part
        idx0 = 2 * FRAME_LEN * FilterBorders[filterIdx][0] / AVS3_SAMPLING_48KHZ;
        idx1 = 2 * FRAME_LEN * FilterBorders[filterIdx][1] / AVS3_SAMPLING_48KHZ;

        // clear auto corr
        SetFloat(rxx, 0.0f, TNS_MAX_FILTER_ORDER + 1);

        for (divisionIdx = 0; (divisionIdx < TNS_DIVISION_NUM) && (norms[filterIdx][divisionIdx] > HLM_MIN_NRG); divisionIdx++) {

            // norm factor
            fac = 1.0f / norms[filterIdx][divisionIdx];

            // start/end line of division
            iStartLine = idx0 + (idx1 - idx0) * divisionIdx / TNS_DIVISION_NUM;
            iEndLine = idx0 + (idx1 - idx0) * (divisionIdx + 1) / TNS_DIVISION_NUM;

            // normalized auto correlation
            for (lag = 0; lag <= TNS_MAX_FILTER_ORDER; lag++) {
                rxx[lag] += fac * Dotp(pSpectrum + iStartLine, pSpectrum + iStartLine + lag, iEndLine - iStartLine - lag);
            }
        }

        /* meaning there is no subdivision with low energy */
        if (divisionIdx == TNS_DIVISION_NUM) {

            /* Limit the maximum order to spectrum length/4 */
            maxOrder = min(TNS_MAX_FILTER_ORDER, (idx1 - idx0) / 4);

            /* compute TNS filter in lattice (ParCor) form with LeRoux-Gueguen algorithm */
            tnsDetector->predictionGain = AutoToParcor(rxx, parCoeff, maxOrder);

            /* non-linear quantization of TNS lattice coefficients with given resolution */
            Parcor2Index(parCoeff, tnsFilter->coefIndex, maxOrder);

            /* reduce filter order by truncating trailing zeros */
            k = maxOrder - 1;
            while ((k >= 0) && (tnsFilter->coefIndex[k] == 0)) {
                --k;
            }
            tnsFilter->order = k + 1;
            
            /* compute avg(coef*coef) */
            tnsDetector->avgSqrCoef = 0;
            for (k = tnsFilter->order - 1; k >= 0; k--) {

                value = parcorCb[tnsFilter->coefIndex[k] + INDEX_SHIFT];
                tnsDetector->avgSqrCoef += value * value;

            }
            // divide by filter order
            if (tnsFilter->order > 0) {
                tnsDetector->avgSqrCoef /= tnsFilter->order;
            }
            else {
                tnsDetector->avgSqrCoef = 0.0f;
            }
        }
    }

    return;
}


/* filter function prototype
I/O params:
    const short order                      (i)   the order of the filter
    float const parCoeff[]                 (i)   parcor coefficients
    float * state                          (i/o) the filter state
    float x                                (i/o) input value of filter
    ret                                    (o)   the filter output
*/
typedef float(*TLinearPredictionFilter)(
    const short order,
    float const parCoeff[],
    float * state,
    float x);


/* 
TNS filter.
I/O params:
    float const spectrum[]                      (i)   mdct spectrum
    const short numOfLines                      (i)   the mdct spectrum line number
    float const parCoeff[]                      (i)   parcor coefficients
    const short order                           (i)   filter order
    TLinearPredictionFilter filter              (i)   the filter implement
    float * state                               (i/o) the filter state
    float output[]                              (o)   the output of filtering
*/
static void Filter(
    float const spectrum[],
    const short numOfLines,
    float const parCoeff[],
    const short order,
    TLinearPredictionFilter filter,
    float * state,
    float output[]
)
{
    short j;

    assert((order >= 0) && (order <= TNS_MAX_FILTER_ORDER));
    assert((numOfLines > 0) || ((numOfLines == 0) && (order == 0)));

    if (order == 0) {
        if ((spectrum != output) && (numOfLines > 0)) {
            Mvf2f(spectrum, output, numOfLines);
        }
    }
    else {
        for (j = 0; j < numOfLines; j++) {
            output[j] = filter(order, parCoeff, state, spectrum[j]);
        }
    }

    return;
}


/*
Linear prediction analysis filter
I/O params
    const short order           (i)   order of LPC
    const float *parCoeff       (i)   parcor coeffs
    float *state                (i/o) filter state
    float x                     (i)   input data
*/
static float FirFilter(
    const short order,
    const float *parCoeff,
    float *state,
    float x
)
{
    short i;
    float tmpSave;

    tmpSave = x;
    for (i = 0; i < order - 1; i++) {
        float const tmp = parCoeff[i] * x + state[i];
        x += parCoeff[i] * state[i];
        state[i] = tmpSave;
        tmpSave = tmp;
    }

    /* last stage: only need half operations */
    x += parCoeff[order - 1] * state[order - 1];
    state[order - 1] = tmpSave;

    return x;
}


/* 
Linear prediction synthesis filter
I/O params
    const short order           (i)   order of LPC
    const float *parCoeff       (i)   parcor coeffs
    float *state                (i/o) filter state
    float x                     (i)   input data
*/
static float IirFilter(
    const short order,
    const float *parCoeff,
    float *state,
    float x
)
{
    short i;

    /* first stage: no need to calculate state[order-1] */
    x -= parCoeff[order - 1] * state[order - 1];
    for (i = order - 2; i >= 0; i--) {
        x -= parCoeff[i] * state[i];
        state[i + 1] = parCoeff[i] * x + state[i];
    }

    state[0] = x;

    return x;
}


/*
Run tns filtering
I/O params:
    TnsData *pTnsData           (i/o) TNS data structure
    float spectrum[]            (i/o) mdct spectrum
    const short filterType      (i)   Filter type, FIR or IIR
*/
void RunTnsFilter(
    TnsData *pTnsData,
    float spectrum[],
    const short filterType
)
{
    TLinearPredictionFilter filterFunc;

    TnsFilter *tnsFilter = NULL;
    TnsBsParam *tnsBsParam = NULL;

    float parCoeff[TNS_MAX_FILTER_ORDER] = {0.0f};
    float state[TNS_MAX_FILTER_ORDER];
    short iFilter;
    short stopLine, startLine;

    // get filter function, FIR at encoder, IIR at decoder
    filterFunc = filterType ? FirFilter : IirFilter;
    // clear filter state
    SetFloat(state, 0.0f, TNS_MAX_FILTER_ORDER);

    // loop over filters, from high to low frequency
    for (iFilter = TNS_MAX_FILTER_NUM - 1; iFilter >= 0; iFilter--) {

        tnsFilter = &pTnsData->filter[iFilter];
        tnsBsParam = &pTnsData->bsParam[iFilter];

        if (tnsBsParam->enable == 0) {
            continue;
        }

        // start and stop line of spectrum
        startLine = 2 * FRAME_LEN * FilterBorders[iFilter][0] / AVS3_SAMPLING_48KHZ;
        stopLine = 2 * FRAME_LEN * FilterBorders[iFilter][1] / AVS3_SAMPLING_48KHZ;

        // get quantized parcor
        Index2Parcor(tnsFilter->coefIndex, parCoeff, tnsFilter->order);

        // perform filtering
        Filter(&spectrum[startLine], stopLine - startLine, parCoeff, tnsFilter->order, filterFunc, state, &spectrum[startLine]);
    }

    return;
}


/*
TNS decision logic
I/O params:
    TnsData * pTnsData          (i/o) TNS data structure
    const short isShortFrame    (i)   flag for short frame
*/
void TnsJudge(
    TnsData * pTnsData, 
    const short isShortFrame
) 
{
    short i;

    TnsDetector *tnsDetector = NULL;
    TnsFilter *tnsFilter = NULL;
    TnsBsParam *tnsBsParam = NULL;

    // loop over filters
    for (i = 0; i < TNS_MAX_FILTER_NUM; i++) {

        tnsDetector = &pTnsData->tnsDetector[i];
        tnsFilter = &pTnsData->filter[i];
        tnsBsParam = &pTnsData->bsParam[i];

        tnsBsParam->enable = 0;

        // filter order > 0
        if (tnsFilter->order > 0) {

            // Three conditions: 
            // averaged square coeff > TH
            // prediction gain > TH
            // is short frame
            if (((tnsDetector->avgSqrCoef > MIN_AVG_SQR_COEFF_THREHOLD) &&
                (tnsDetector->predictionGain > MIN_PREDICTION_GAIN)) ||
                isShortFrame) {

                // set tns enable flag
                tnsBsParam->enable = 1;
            }
        }
    }
}


/*
Tns encoder side interface
I/O params:
    TnsData * pTnsData          (i/o) TNS data structure
    float *origSpectrum         (i/o) mdct spectrum
    const short isShortFrame    (i)   flag for short frame
*/
void TnsEnc(
    TnsData * pTnsData, 
    float *origSpectrum, 
    const short isShortFrame
)
{
    // For short window, deinterleave before tns filtering
    if (isShortFrame == 1) {
        MdctSpectrumDeinterleave(origSpectrum, BLOCK_LEN_LONG, N_BLOCK_SHORT);
    }

    // calculate tns cofficients
    GetTnsData(origSpectrum, pTnsData);

    // judge whether to enable TNS filter
    TnsJudge(pTnsData, isShortFrame);

    // apply tns FIR filters
    RunTnsFilter(pTnsData, origSpectrum, TNS_FILTER_ANALYSIS);

    // After tns filtering, interleave again
    if (isShortFrame == 1) {
        MdctSpectrumInterleave(origSpectrum, BLOCK_LEN_LONG, N_BLOCK_SHORT);
    }

    // encode tns params
    TnsEncodeParam(pTnsData);
}


/*
Tns decoder side interface
I/O params:
    TnsData * pTnsData          (i/o) TNS data structure
    float *origSpectrum         (i/o) MDCT spectrum
    const short isShortFrame    (i)   flag for short frame
*/
void TnsDec(
    TnsData * pTnsData, 
    float *origSpectrum, 
    const short isShortFrame
)
{
    // decode tns params
    TnsDecodeParam(pTnsData);

    // For short window, deinterleave before tns filtering
    if (isShortFrame == 1) {
        MdctSpectrumDeinterleave(origSpectrum, BLOCK_LEN_LONG, N_BLOCK_SHORT);
    }

    // apply tns IIR filters
    RunTnsFilter(pTnsData, origSpectrum, TNS_FILTER_SYNTHESIS);

    // After tns filtering, interleave again
    if (isShortFrame == 1) {
        MdctSpectrumInterleave(origSpectrum, BLOCK_LEN_LONG, N_BLOCK_SHORT);
    }
}
