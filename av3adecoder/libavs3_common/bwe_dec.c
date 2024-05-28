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
#include <stdint.h>
#include <math.h>

#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_rom_com.h"
#include "avs3_stat_com.h"
#include "avs3_prot_com.h"


/*
Init decoder side bwe data structure
I/O params:
    BweDecDataHandle bweDecData         (i/o) decoder side bwe data handle
*/
void InitBweDecData(
    BweDecDataHandle bweDecData
)
{
    SetFloat(bweDecData->sfbEnvelope, 0.0f, MAX_NUM_SFB_BWE);
    SetShort(bweDecData->sfbEnvQIdx, 0, MAX_NUM_SFB_BWE);
    SetShort(bweDecData->whiteningLevel, 0, MAX_NUM_TILE);

    SetFloat(bweDecData->bweSpectrum, 0.0f, BLOCK_LEN_LONG);

    return;
}


/* 
Copy spectrum from source tile to target tile
I/O params:
    BweConfigHandle bweConfig           (i)   bwe config handle
    BweDecDataHandle bweDecData         (i/o) decoder side bwe data handle
    float *mdctSpectrum                 (i)   decoded mdct spectrum
*/
static void BweCopySpectrum(
    BweConfigHandle bweConfig,
    BweDecDataHandle bweDecData,
    float *mdctSpectrum
)
{
    int16_t i;
    int16_t tileIdx;
    int16_t srcLineIdx;

    // clear buffer
    SetFloat(bweDecData->bweSpectrum, 0.0f, BLOCK_LEN_LONG);

    // copy spectrum below bwe start line
    Mvf2f(mdctSpectrum, bweDecData->bweSpectrum, bweConfig->bweStartLine);

    // copy from src tile to target tile
    for (tileIdx = 0; tileIdx < bweConfig->numTiles; tileIdx++) {

        srcLineIdx = bweConfig->srcTiles[tileIdx];

        for (i = bweConfig->targetTiles[tileIdx]; i < bweConfig->targetTiles[tileIdx + 1]; i++) {
            bweDecData->bweSpectrum[i] = mdctSpectrum[srcLineIdx];
            srcLineIdx++;
        }
    }

    return;
}


/*
Middle level whitening of bwe spectrum, by moving average
I/O params:
    float *inSpectrum               (i) input mdct spectrum
    float *outSpectrum              (o) output mdct spectrum
    int16_t start                   (i) start bin index of spectrum
    int16_t stop                    (i) stop bin index of spectrum
    int16_t averageSize             (i) average size in whitening
*/
static void BweSpecWhiteningMid(
    float *inSpectrum,
    float *outSpectrum,
    int16_t start, 
    int16_t stop,
    int16_t averageSize
)
{
    float squareSum;
    float averageSpec;

    for (int16_t i = start; i < stop; i++) {
        
        squareSum = 0.0f;
        averageSpec = 0.0f;

        for (int16_t j = (i - averageSize); j < (i + averageSize + 1); j++) {
            squareSum += inSpectrum[j] * inSpectrum[j];
        }

        averageSpec = squareSum / (float)(2 * averageSize + 1);
        averageSpec = (float)sqrt(averageSpec);

        if (averageSpec == 0.0f) {
            outSpectrum[i] = inSpectrum[i];
        }
        else {
            outSpectrum[i] = inSpectrum[i] / averageSpec;
        }
    }

    return;
}


/*
High level whitening of bwe spectrum, by noise substitution
I/O params:
    float *inSpectrum               (i) input mdct spectrum
    float *outSpectrum              (o) output mdct spectrum
    int16_t start                   (i) start bin index of spectrum
    int16_t stop                    (i) stop bin index of spectrum
*/
static void BweSpecWhiteningHigh(
    float *inSpectrum,
    float *outSpectrum,
    int16_t start,
    int16_t stop
)
{
    int16_t i;
    float absSum = 0.0f;

    // get abs spectrum sum in current range
    for (i = start; i < stop; i++) {
        absSum += (float)fabs(inSpectrum[i]);
    }

    if (absSum > 0.0f) {
        for (i = start; i < stop; i++) {
            // [-1, 1] range random noise
            outSpectrum[i] = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        }
    }
    else {
        // clear spectrum
        for (i = start; i < stop; i++) {
            outSpectrum[i] = 0.0f;
        }
    }

    return;
}


/*
Apply whitening to mdct spectrum
I/O params:
    BweConfigHandle bweConfig           (i) bwe config handle
    BweDecDataHandle bweDecData         (o) decoder side bwe data handle
*/
static void BweApplyWhitening(
    BweConfigHandle bweConfig,
    BweDecDataHandle bweDecData
)
{
    int16_t tileIdx;
    float whitenedSpectrum[BLOCK_LEN_LONG] = { 0.0f };

    // loop over tiles
    for (tileIdx = 0; tileIdx < bweConfig->numTiles; tileIdx++) {

        // get tile related info
        int16_t tileStartLine = bweConfig->targetTiles[tileIdx];
        int16_t tileStopLine = bweConfig->targetTiles[tileIdx + 1];
        int16_t tileWidth = tileStopLine - tileStartLine;

        if (bweDecData->whiteningLevel[tileIdx] == BWE_WHITENING_OFF) {
            // whitening off, copy spectrum
            Mvf2f(bweDecData->bweSpectrum + tileStartLine, whitenedSpectrum + tileStartLine, tileWidth);
        }
        else if (bweDecData->whiteningLevel[tileIdx] == BWE_WHITENING_MID) {
            // whitening middle, divide spectrum by the moving averaged spectrum
            BweSpecWhiteningMid(bweDecData->bweSpectrum, whitenedSpectrum, tileStartLine, tileStopLine, LEN_WHITEN_AVERAGE);
        }
        else if (bweDecData->whiteningLevel[tileIdx] == BWE_WHITENING_HIGH) {
            // whitening high, using random noise
            BweSpecWhiteningHigh(bweDecData->bweSpectrum, whitenedSpectrum, tileStartLine, tileStopLine);
        }
    }

    // copy whitened spectrum back to buffer
    Mvf2f(whitenedSpectrum, bweDecData->bweSpectrum, BLOCK_LEN_LONG);

    return;
}


/*
Apply SFB envelope to whitened spectrum
I/O params:
    BweConfigHandle bweConfig               (i) bwe config handle
    BweDecDataHandle bweDecData             (i) decoder side bwe data handle
    float *mdctSpectrum                     (o) bwe processed mdct spectrum
*/
static void BweApplyEnvelope(
    BweConfigHandle bweConfig,
    BweDecDataHandle bweDecData, 
    float *mdctSpectrum
)
{
    int16_t i;
    int16_t sfbIdx;
    int16_t sfbWidth;
    float targetEner, currEner;
    float gainSfb;

    for (sfbIdx = 0; sfbIdx < bweConfig->numSfb; sfbIdx++) {

        // sfb width
        sfbWidth = bweConfig->sfbTable[sfbIdx + 1] - bweConfig->sfbTable[sfbIdx];

        // curr energy of whitened spectrum
        currEner = 0.0f;
        for (i = bweConfig->sfbTable[sfbIdx]; i < bweConfig->sfbTable[sfbIdx + 1]; i++) {
            currEner += bweDecData->bweSpectrum[i] * bweDecData->bweSpectrum[i];
        }
        currEner /= sfbWidth;

        // target energy of current sfb
        targetEner = (float)pow(2.0f, bweDecData->sfbEnvQIdx[sfbIdx] / 4.24966f - 4.0f);

        // get sfb gain
        if (currEner != 0.0f) {
            gainSfb = (float)sqrt(targetEner / currEner);
        }
        else {
            gainSfb = 1.0f;
        }

        // apply gain to whitened spectrum
        for (i = bweConfig->sfbTable[sfbIdx]; i < bweConfig->sfbTable[sfbIdx + 1]; i++) {
            bweDecData->bweSpectrum[i] *= gainSfb;
        }

        // copy bwe spectrum to mdct spectrum buffer
        for (i = bweConfig->sfbTable[sfbIdx]; i < bweConfig->sfbTable[sfbIdx + 1]; i++) {
            mdctSpectrum[i] = bweDecData->bweSpectrum[i];
        }
    }

    // clear spectrum beyond BWE stop line
    for (i = bweConfig->bweStopLine; i < BLOCK_LEN_LONG; i++) {
        mdctSpectrum[i] = 0.0f;
    }

    return;
}


/*
Decoder side bwe process
I/O params:
    BweConfigHandle bweConfig           (i)   bwe config handle
    BweDecDataHandle bweDecData         (i)   decoder side bwe data handle
    float *mdctSpectrum                 (i/o) mdct spectrum before and after bwe
*/
void BweApplyDec(
    BweConfigHandle bweConfig,
    BweDecDataHandle bweDecData,
    float *mdctSpectrum
)
{
    BweCopySpectrum(bweConfig, bweDecData, mdctSpectrum);

    BweApplyWhitening(bweConfig, bweDecData);

    BweApplyEnvelope(bweConfig, bweDecData, mdctSpectrum);

    return;
}
