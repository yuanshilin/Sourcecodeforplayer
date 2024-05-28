
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <float.h>

#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_rom_com.h"
#include "avs3_stat_com.h"
#include "avs3_prot_com.h"


/*
Init MCR stereo configuration structure
I/O params:
    MCR_CONFIG_HANDLE mcrConfig         (i/o) mcr config handle
*/
void InitMcrConfig(
    MCR_CONFIG_HANDLE mcrConfig
)
{
    // for long window, including transition window
    mcrConfig->numSfb[0] = MCR_NUM_SFB_FB;
    mcrConfig->vqVecNum[0] = MCR_NUM_SUBVEC;
    mcrConfig->vqVecDim[0] = MCR_DIM_SUBVEC;
    mcrConfig->vqNumBits[0] = MCR_VQ_NBITS_LONG;
    mcrConfig->vqCbSize[0] = MCR_VQ_CBSIZE_LONG;
    mcrConfig->vqCodebook[0] = mcr_codebook_9bit;

    // for short window
    mcrConfig->numSfb[1] = MCR_NUM_SFB_FB;
    mcrConfig->vqVecNum[1] = MCR_NUM_SUBVEC;
    mcrConfig->vqVecDim[1] = MCR_DIM_SUBVEC;
    mcrConfig->vqNumBits[1] = MCR_VQ_NBITS_SHORT;
    mcrConfig->vqCbSize[1] = MCR_VQ_CBSIZE_SHORT;
    mcrConfig->vqCodebook[1] = mcr_codebook_8bit;

    return;
}


/*
Init MCR stereo data structure
I/O params:
    MCR_DATA_HANDLE mcrData         (i/o) mcr data handle
*/
void InitMcrData(
    MCR_DATA_HANDLE mcrData
)
{
    // init mcr angle, theta
    SetFloat(mcrData->theta[0], 0.0f, MCR_NUM_SFB_FB);
    SetFloat(mcrData->theta[1], 0.0f, MCR_NUM_SFB_FB);

    // init vq indices
    SetShort(mcrData->vqIdx[0], 0, MCR_NUM_SUBVEC);
    SetShort(mcrData->vqIdx[1], 0, MCR_NUM_SUBVEC);

    return;
}


/*
Get MCR rotation angle with the highest correlation
I/O params:
    float *theta                (o) mcr angle for current subband
    const float *specLeft       (i) left channel mdct spectrum
    const float *specRight      (i) right channel mdct spectrum
    int16_t len                 (i) spectrum length
*/
static void GetMcrAngle(
    float *theta,
    const float *specLeft,
    const float *specRight,
    int16_t len
)
{
    float enerLeft = 0.0f;
    float enerRight = 0.0f;
    float xcorr = 0.0f;
    float enerd = 0.0f;
    float enerx = 0.0f;

    float tan2;

    // get energy and cross-corralation
    for (int16_t i = 0; i < len; i++) {
        enerLeft += specLeft[i] * specLeft[i];
        enerRight += specRight[i] * specRight[i];
        xcorr += specLeft[i] * specRight[i];
    }

    enerd = 0.5f * (enerRight - enerLeft);
    enerx = (float)sqrt(enerd * enerd + xcorr * xcorr);

    if (xcorr == 0.0f) {
        if (enerd > 0.0f) {
            tan2 = FLT_MAX;
        }
        else {
            tan2 = -FLT_MAX;
        }
    }
    else {
        tan2 = enerd / xcorr;
    }

    // get mcr angle
    *theta = 0.5f * (float)atan(tan2);

    // post proc of mcr angle
    if (*theta > 0.0f && enerd < 0.0f) {
        *theta -= AVS3_PI / 2.0f;
    }
    else if (*theta < 0.0f && enerd > 0.0f) {
        *theta += AVS3_PI / 2.0f;
    }

    if (enerx < 1.0e-6) {
        *theta = 0.0f;
    }

    return;
}


/*
Rotate left/right channel mdct spectrum according to mcr angle
I/O params:
    float *specLeft             (i/o) left channel mdct spectrum
    float *specRight            (i/o) right channel mdct spectrum
    int16_t len                 (i)   spectrum length
    float theta                 (i)   mcr angle for current subband
*/
static void RotateTransform(
    float *specLeft,
    float *specRight,
    int16_t len,
    float theta
)
{
    float c, s;
    float tmpL, tmpR;

    c = (float)cos(theta);
    s = (float)sin(theta);

    for (int16_t i = 0; i < len; i++) {

        tmpL = specLeft[i];
        tmpR = specRight[i];

        specLeft[i] = c * tmpL + s * tmpR;
        specRight[i] = -s * tmpL + c * tmpR;
    }

    return;
}


/*
VQ of MCR angle vector
I/O params:
    float *vector                   (i/o) mcr angle vector, before/after quantization
    int16_t vecSize                 (i)   vector size
    const float *vqCodebook         (i)   VQ codebook pointer
    int16_t cbSize                  (i)   codebook size
    int16_t *vqIdx                  (o)   VQ codebook index
*/
static void McrVectorQuantize(
    float *vector,
    int16_t vecSize,
    const float *vqCodebook,
    int16_t cbSize,
    int16_t *vqIdx
)
{
    float minValue = FLT_MAX;
    float tmp;
    int16_t index = -1;

    for (int16_t i = 0; i < cbSize; i++) {
        tmp = 0.0f;
        for (int16_t j = 0; j < vecSize; j++) {
            tmp += (vector[j] - vqCodebook[i * vecSize + j]) * (vector[j] - vqCodebook[i * vecSize + j]);
        }
        if (tmp < minValue) {
            minValue = tmp;
            index = i;
        }
    }

    *vqIdx = index;
    for (int16_t i = 0; i < vecSize; i++) {
        vector[i] = vqCodebook[(*vqIdx) * vecSize + i];
    }

    return;
}


/*
VQ dequantize of MCR angle vector
I/O params:
    float *vector                   (o) dequantized MCR angle vector
    int16_t vecSize                 (i) vector size
    const float *vqCodebook         (i) VQ codebook pointer
    int16_t vqIdx                   (i) VQ index
*/
static void McrVectorDequantize(
    float *vector,
    int16_t vecSize,
    const float *vqCodebook,
    int16_t vqIdx
)
{
    for (int16_t i = 0; i < vecSize; i++) {
        vector[i] = vqCodebook[vqIdx * vecSize + i];
    }

    return;
}


/*
MCR stereo top level interface, perform MCR downmix at encoder
I/O params:
    MCR_DATA_HANDLE mcrData             (i/o) mcr data handle
    MCR_CONFIG_HANDLE mcrConfig         (i)   mcr config handle
    float *mdctSpectrumL                (i/o) left channel mdct spectrum
    float *mdctSpectrumR                (i/o) right channel mdct spectrum
    int16_t isShortWin                  (i)   short window indicator, use different VQ settings
*/
int16_t McrEncode(
    MCR_DATA_HANDLE mcrData,
    MCR_CONFIG_HANDLE mcrConfig,
    float *mdctSpectrumL,
    float *mdctSpectrumR,
    int16_t isShortWin
)
{
    // spectrum buffer
    float specLeftOdd[BLOCK_LEN_LONG / 2] = { 0.0f };
    float specLeftEven[BLOCK_LEN_LONG / 2] = { 0.0f };
    float specRightOdd[BLOCK_LEN_LONG / 2] = { 0.0f };
    float specRightEven[BLOCK_LEN_LONG / 2] = { 0.0f };

    // vq dim and size settings
    // according to transform type
    int16_t numSfb = mcrConfig->numSfb[isShortWin];
    int16_t vqVecNum = mcrConfig->vqVecNum[isShortWin];
    int16_t vqVecDim = mcrConfig->vqVecDim[isShortWin];
    int16_t vqCbSize = mcrConfig->vqCbSize[isShortWin];
    const float *vqCodebook = mcrConfig->vqCodebook[isShortWin];

    // split spectrum to get odd and even sub-spec
    for (int16_t i = 0; i < BLOCK_LEN_LONG / 2; i++) {
        // even part
        specLeftEven[i] = mdctSpectrumL[2 * i];
        specRightEven[i] = mdctSpectrumR[2 * i];

        // odd part
        specLeftOdd[i] = mdctSpectrumL[2 * i + 1];
        specRightOdd[i] = mdctSpectrumR[2 * i + 1];
    }

    // get mcr angle, theta
    for (int16_t i = 0; i < numSfb; i++) {
        // spectrum idx
        int16_t startIdx = mcr_sfb_table_fb[i];
        int16_t endIdx = mcr_sfb_table_fb[i + 1];
        int16_t sfbLen = endIdx - startIdx;

        // even part
        GetMcrAngle(&mcrData->theta[0][i], specLeftEven + startIdx, specRightEven + startIdx, sfbLen);

        // odd part
        GetMcrAngle(&mcrData->theta[1][i], specLeftOdd + startIdx, specRightOdd + startIdx, sfbLen);
    }

    // vq of mcr angle
    for (int16_t i = 0; i < vqVecNum; i++) {
        // even part
        McrVectorQuantize(&mcrData->theta[0][i * vqVecDim], vqVecDim, vqCodebook, vqCbSize, &mcrData->vqIdx[0][i]);

        // odd part
        McrVectorQuantize(&mcrData->theta[1][i * vqVecDim], vqVecDim, vqCodebook, vqCbSize, &mcrData->vqIdx[1][i]);
    }

    // dequantize of mcr angle
    for (int16_t i = 0; i < vqVecNum; i++) {
        // even part
        McrVectorDequantize(&mcrData->theta[0][i * vqVecDim], vqVecDim, vqCodebook, mcrData->vqIdx[0][i]);

        // odd part
        McrVectorDequantize(&mcrData->theta[1][i * vqVecDim], vqVecDim, vqCodebook, mcrData->vqIdx[1][i]);
    }

    // perform rotate transform
    for (int16_t i = 0; i < numSfb; i++) {
        // spectrum idx
        int16_t startIdx = mcr_sfb_table_fb[i];
        int16_t endIdx = mcr_sfb_table_fb[i + 1];
        int16_t sfbLen = endIdx - startIdx;

        // even part
        RotateTransform(specLeftEven + startIdx, specRightEven + startIdx, sfbLen, mcrData->theta[0][i]);

        // odd part
        RotateTransform(specLeftOdd + startIdx, specRightOdd + startIdx, sfbLen, mcrData->theta[1][i]);
    }

    // transform odd/even spectrum back to left and right
    for (int16_t i = 0; i < BLOCK_LEN_LONG / 2; i++) {
        // left channel
        mdctSpectrumL[2 * i] = specLeftEven[i];
        mdctSpectrumL[2 * i + 1] = specLeftOdd[i];

        // right channel
        mdctSpectrumR[2 * i] = specRightEven[i];
        mdctSpectrumR[2 * i + 1] = specRightOdd[i];
    }

    // clear right channel spectrum
    // only left channel spectrum will be encoded
    SetFloat(mdctSpectrumR, 0.0f, BLOCK_LEN_LONG);

    return 0;
}


/*
MCR stereo top level interface, perform MCR upmix at decoder
I/O params:
    MCR_DATA_HANDLE mcrData             (i/o) mcr data handle
    MCR_CONFIG_HANDLE mcrConfig         (i)   mcr config handle
    float *mdctSpectrumL                (i/o) left channel mdct spectrum
    float *mdctSpectrumR                (i/o) right channel mdct spectrum
    int16_t isShortWin                  (i)   short window indicator, use different VQ settings
*/
int16_t McrDecode(
    MCR_DATA_HANDLE mcrData,
    MCR_CONFIG_HANDLE mcrConfig,
    float *mdctSpectrumL,
    float *mdctSpectrumR,
    int16_t isShortWin
)
{
    // spectrum buffer
    float specLeftOdd[BLOCK_LEN_LONG / 2] = { 0.0f };
    float specLeftEven[BLOCK_LEN_LONG / 2] = { 0.0f };
    float specRightOdd[BLOCK_LEN_LONG / 2] = { 0.0f };
    float specRightEven[BLOCK_LEN_LONG / 2] = { 0.0f };

    // vq dim and size settings
    // according to transform type
    int16_t numSfb = mcrConfig->numSfb[isShortWin];
    int16_t vqVecNum = mcrConfig->vqVecNum[isShortWin];
    int16_t vqVecDim = mcrConfig->vqVecDim[isShortWin];
    int16_t vqCbSize = mcrConfig->vqCbSize[isShortWin];
    const float *vqCodebook = mcrConfig->vqCodebook[isShortWin];

    // copy left channel spectrum to right channel
    Mvf2f(mdctSpectrumL, mdctSpectrumR, BLOCK_LEN_LONG);

    // split spectrum to get odd and even sub-spec
    for (int16_t i = 0; i < BLOCK_LEN_LONG / 2; i++) {
        // even part
        specLeftEven[i] = mdctSpectrumL[2 * i];
        specRightEven[i] = mdctSpectrumR[2 * i];

        // odd part
        specLeftOdd[i] = mdctSpectrumL[2 * i + 1];
        specRightOdd[i] = mdctSpectrumR[2 * i + 1];
    }

    // dequantize of mcr angle
    for (int16_t i = 0; i < vqVecNum; i++) {
        // even part
        McrVectorDequantize(&mcrData->theta[0][i * vqVecDim], vqVecDim, vqCodebook, mcrData->vqIdx[0][i]);

        // odd part
        McrVectorDequantize(&mcrData->theta[1][i * vqVecDim], vqVecDim, vqCodebook, mcrData->vqIdx[1][i]);
    }

    // perform rotate transform
    for (int16_t i = 0; i < numSfb; i++) {
        // spectrum idx
        int16_t startIdx = mcr_sfb_table_fb[i];
        int16_t endIdx = mcr_sfb_table_fb[i + 1];
        int16_t sfbLen = endIdx - startIdx;

        // even part
        RotateTransform(specLeftEven + startIdx, specRightEven + startIdx, sfbLen, -mcrData->theta[0][i]);

        // odd part
        RotateTransform(specLeftOdd + startIdx, specRightOdd + startIdx, sfbLen, -mcrData->theta[1][i]);
    }

    // transform odd/even spectrum back to left and right
    for (int16_t i = 0; i < BLOCK_LEN_LONG / 2; i++) {
        // left channel
        mdctSpectrumL[2 * i] = specLeftEven[i];
        mdctSpectrumL[2 * i + 1] = specLeftOdd[i];

        // right channel
        mdctSpectrumR[2 * i] = specRightEven[i];
        mdctSpectrumR[2 * i + 1] = specRightOdd[i];
    }

    return 0;
}
