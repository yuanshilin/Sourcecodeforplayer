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
#include <float.h>

#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_cnst_enc.h"
#include "avs3_stat_com.h"
#include "avs3_stat_enc.h"
#include "avs3_prot_com.h"
#include "avs3_prot_enc.h"


/*
Init encoder side bwe data structure
I/O params:
    BweEncDataHandle bweEncData         (i/o) encoder side bwe data handle
*/
void InitBweEncData(
    BweEncDataHandle bweEncData
)
{
    SetFloat(bweEncData->sfbEnvelope, 0.0f, MAX_NUM_SFB_BWE);
    SetShort(bweEncData->sfbEnvQIdx, 0, MAX_NUM_SFB_BWE);
    SetShort(bweEncData->whiteningLevel, 0, MAX_NUM_TILE);

    for (int16_t i = 0; i < LEN_HIST_WHITENING; i++) {
        SetShort(bweEncData->prevWhiteningLevel[i], 0, MAX_NUM_TILE);
    }

    return;
}


/*
Get SFB spectrum envelope in bwe range
I/O params:
    BweConfigHandle bweConfig           (i)   bwe config handle
    BweEncDataHandle bweEncData         (i/o) encoder side bwe data handle
    float *mdctSpectrum                 (i)   mdct spectrum
    float *powerSpectrum                (i)   power spectrum, if available
*/
static void BweGetEnvelope(
    BweConfigHandle bweConfig,
    BweEncDataHandle bweEncData,
    float *mdctSpectrum,
    float *powerSpectrum
)
{
    int16_t tileIdx, sfbIdx;
    int16_t *sfbTable;
    int16_t *sfbTileWrap;

    int16_t srcSfbLineIdx = 0;
    int16_t sfbWidth = 0;

    float sfbEnvelope;
    float sfbEnerSrcC, sfbEnerSrcR;         // powerspectrum and MDCT spectrum energy of src band
    float sfbEnerTarC, sfbEnerTarR;         // powerspectrum and MDCT spectrum energy of target band

    // init sfb table pointer
    sfbTable = bweConfig->sfbTable;
    sfbTileWrap = bweConfig->sfbTileWrap;

    // tile loop
    for (tileIdx = 0; tileIdx < bweConfig->numTiles; tileIdx++) {

        // src sfb start line
        srcSfbLineIdx = bweConfig->srcTiles[tileIdx];

        // sfb loop
        for (sfbIdx = sfbTileWrap[tileIdx]; sfbIdx < sfbTileWrap[tileIdx + 1]; sfbIdx++) {

            // sfb width
            sfbWidth = sfbTable[sfbIdx + 1] - sfbTable[sfbIdx];

            // power spectrum present
            if (powerSpectrum) {
                // init energy
                sfbEnerSrcC = (float)1.0e-7;           // src sfb powerspec
                sfbEnerSrcR = (float)1.0e-7;           // src sfb mdct energy
                sfbEnerTarC = (float)1.0e-7;           // tar sfb powerspec
                sfbEnerTarR = (float)1.0e-7;           // tar sfb mdct energy

                // get sfb energy
                for (int16_t i = sfbTable[sfbIdx]; i < sfbTable[sfbIdx + 1]; i++) {
                    sfbEnerTarC += powerSpectrum[i];
                    sfbEnerSrcC += powerSpectrum[srcSfbLineIdx];
                    sfbEnerSrcR += mdctSpectrum[srcSfbLineIdx] * mdctSpectrum[srcSfbLineIdx];

                    srcSfbLineIdx++;
                }

                // get expected target sbf mdct energy
                sfbEnerTarR = sfbEnerSrcR * (sfbEnerTarC / sfbEnerSrcC);

                // get target sfb envelope
                sfbEnvelope = sfbEnerTarR / (float)sfbWidth;
            }
            // only mdct spectrum
            else {
                // init target sfb mdct energy
                sfbEnerTarR = (float)1.0e-7;           // tar sfb mdct energy

                // get energy of mdct spectrum in target sfb
                for (int16_t i = sfbTable[sfbIdx]; i < sfbTable[sfbIdx + 1]; i++) {
                    sfbEnerTarR += mdctSpectrum[i] * mdctSpectrum[i];
                }

                // get target sfb envelope
                sfbEnvelope = sfbEnerTarR / (float)sfbWidth;
            }

            // scalar quantization of sfb envelope
            bweEncData->sfbEnvQIdx[sfbIdx] = (int16_t)floor(0.5f + 4.24966f * ((float)log2(sfbEnvelope) + 4.0f));
            // limit range to [0, 127]
            if (bweEncData->sfbEnvQIdx[sfbIdx] < 0) {
                bweEncData->sfbEnvQIdx[sfbIdx] = 0;
            }
            if (bweEncData->sfbEnvQIdx[sfbIdx] > 127) {
                bweEncData->sfbEnvQIdx[sfbIdx] = 127;
            }
            
            // get quantized sfb envelope
            bweEncData->sfbEnvelope[sfbIdx] = (float)pow(2.0f, bweEncData->sfbEnvQIdx[sfbIdx] / 4.24966f - 4.0f);
        }
    }

    return;
}


/*
Get whitening level of bwe tiles
I/O params:
    BweConfigHandle bweConfig               (i)   bwe config handle
    BweEncDataHandle bweEncData             (i/o) encoder side bwe data handle
    float *mdctSpectrum                     (i)   mdct spectrum
    int16_t isLongWin                       (i)   long window frame indicator
*/
static void BweGetWhiteningLevel(
    BweConfigHandle bweConfig,
    BweEncDataHandle bweEncData,
    float *mdctSpectrum,
    int16_t isLongWin
)
{
    int16_t tileIdx, sfbIdx;
    int16_t *sfbTable;
    int16_t *sfbTileWrap;
    int16_t srcSfbStartLine, srcSfbStopLine;
    int16_t sfbWidth;

    float mdctEnerSpec[BLOCK_LEN_LONG] = {0.0f};
    float logMdctEnerSpec[BLOCK_LEN_LONG] = {0.0f};

    float sfmSrcSfb[MAX_NUM_SFB_BWE] = { 0.0f };
    float sfmTarSfb[MAX_NUM_SFB_BWE] = { 0.0f };
    float sfmSrcTile = 0.0f;
    float sfmTarTile = 0.0f;

    float parSrcSfb[MAX_NUM_SFB_BWE] = { 0.0f };            // peak average ratio of src sfb
    float parTarSfb[MAX_NUM_SFB_BWE] = { 0.0f };            // peak average ratio of target sfb
    float parSrcTile = 0.0f;                                // peak average ratio of src tile
    float parTarTile = 0.0f;                                // peak average ratio of target tile

    int16_t numSfbPerTile = 0;

    // init sfb table pointer
    sfbTable = bweConfig->sfbTable;
    sfbTileWrap = bweConfig->sfbTileWrap;

    // By default, set whitening level to OFF
    for (tileIdx = 0; tileIdx < bweConfig->numTiles; tileIdx++) {
        bweEncData->whiteningLevel[tileIdx] = BWE_WHITENING_OFF;
    }

    // For long window frame
    if (isLongWin == 1) {

        // calculate mdct energy spectrum and log mdct energy spectrum
        for (int16_t i = 0; i < BLOCK_LEN_LONG; i++) {
            mdctEnerSpec[i] = mdctSpectrum[i] * mdctSpectrum[i];
            logMdctEnerSpec[i] = max(0.0f, (float)(log10(max(FLT_MIN, mdctEnerSpec[i])) / log10(2.0)));
        }

        // calculate SFM of target and source sfb
        for (tileIdx = 0; tileIdx < bweConfig->numTiles; tileIdx++) {

            // first src sfb in tile, start line
            srcSfbStartLine = bweConfig->srcTiles[tileIdx];

            for (sfbIdx = sfbTileWrap[tileIdx]; sfbIdx < sfbTileWrap[tileIdx + 1]; sfbIdx++) {

                // get sfb width
                sfbWidth = sfbTable[sfbIdx + 1] - sfbTable[sfbIdx];
                // get stop line of curr sfb
                srcSfbStopLine = srcSfbStartLine + sfbWidth;

                // calculate sfm param
                sfmTarSfb[sfbIdx] = BweGetSfm(mdctEnerSpec, logMdctEnerSpec, sfbTable[sfbIdx], sfbTable[sfbIdx + 1]);
                sfmSrcSfb[sfbIdx] = BweGetSfm(mdctEnerSpec, logMdctEnerSpec, srcSfbStartLine, srcSfbStopLine);

                // calculate par param
                parTarSfb[sfbIdx] = BweGetPeakAvgRatio(logMdctEnerSpec, sfbTable[sfbIdx], sfbTable[sfbIdx + 1]);
                parSrcSfb[sfbIdx] = BweGetPeakAvgRatio(logMdctEnerSpec, srcSfbStartLine, srcSfbStopLine);

                // update source sfb start line
                srcSfbStartLine += sfbWidth;
            }
        }

        // whitening level decision
        for (tileIdx = 0; tileIdx < bweConfig->numTiles; tileIdx++) {

            // get tile averaged sfm and par for src and target
            sfmSrcTile = 0.0f;
            sfmTarTile = 0.0f;
            parSrcTile = 0.0f;
            parTarTile = 0.0f;

            // number of sfbs in each tile
            numSfbPerTile = sfbTileWrap[tileIdx + 1] - sfbTileWrap[tileIdx];

            // loop over sfb in each tile
            for (sfbIdx = sfbTileWrap[tileIdx]; sfbIdx < sfbTileWrap[tileIdx + 1]; sfbIdx++) {
                sfmSrcTile += sfmSrcSfb[sfbIdx];
                sfmTarTile += sfmTarSfb[sfbIdx];

                parSrcTile += parSrcSfb[sfbIdx];
                parTarTile += parTarSfb[sfbIdx];
            }

            // average of sfm
            sfmSrcTile /= (float)numSfbPerTile;
            sfmTarTile /= (float)numSfbPerTile;
            // average of par
            parSrcTile /= (float)numSfbPerTile;
            parTarTile /= (float)numSfbPerTile;

            // divide sfm by par to get final sfm
            sfmSrcTile /= parSrcTile;
            sfmTarTile /= parTarTile;

            // decision logic, ToDo: fine tune
            if (sfmTarTile < sfmSrcTile || sfmTarTile < 0.19f) {
                bweEncData->whiteningLevel[tileIdx] = BWE_WHITENING_OFF;
            }
            else if (sfmTarTile >= sfmSrcTile && sfmTarTile < (sfmSrcTile + 0.15f) || (sfmTarTile >= 0.19f && sfmTarTile < 0.3f)) {
                bweEncData->whiteningLevel[tileIdx] = BWE_WHITENING_MID;
            }
            else if (sfmTarTile >= (sfmSrcTile + 0.15f) || sfmTarTile > 0.4f) {
                bweEncData->whiteningLevel[tileIdx] = BWE_WHITENING_HIGH;
            }
        }
    }

    // Update whitening level history
    for (int16_t i = 0; i < LEN_HIST_WHITENING - 1; i++) {
        for (tileIdx = 0; tileIdx < bweConfig->numTiles; tileIdx++) {
            bweEncData->prevWhiteningLevel[i][tileIdx] = bweEncData->prevWhiteningLevel[i + 1][tileIdx];
        }
    }
    for (tileIdx = 0; tileIdx < bweConfig->numTiles; tileIdx++) {
        bweEncData->prevWhiteningLevel[LEN_HIST_WHITENING - 1][tileIdx] = bweEncData->whiteningLevel[tileIdx];
    }

    return;
}


/*
Clear high band spectrum at the end of bwe process
I/O params:
    BweConfigHandle bweConfig                   (i)   bwe config handle
    float *mdctSpectrum                         (i/o) mdct spectrum
*/
static void BweClearHighBand(
    BweConfigHandle bweConfig,
    float *mdctSpectrum
)
{
    int16_t i;
    int16_t bweStartLine;

    // get bwe start line
    bweStartLine = bweConfig->bweStartLine;

    // clear high frequency components after BWE parameter extraction
    for (i = bweStartLine; i < BLOCK_LEN_LONG; i++) {
        mdctSpectrum[i] = 0.0f;
    }

    return;
}


/*
Encoder side bwe process
I/O params:
    BweConfigHandle bweConfig           (i)   bwe config handle
    BweEncDataHandle bweEncData         (i/o) encoder side bwe data handle
    float *mdctSpectrum                 (i/o) mdct spectrum
    float *powerSpectrum                (i)   power spectrum, if available
    int16_t isLongWin                   (i)   long window frame indicator
*/
void BweApplyEnc(
    BweConfigHandle bweConfig,
    BweEncDataHandle bweEncData,
    float *mdctSpectrum,
    float *powerSpectrum,
    int16_t isLongWin
)
{
    // calculate bwe envelope info
    BweGetEnvelope(bweConfig, bweEncData, mdctSpectrum, powerSpectrum);

    // decide whitening level in each tile
    BweGetWhiteningLevel(bweConfig, bweEncData, mdctSpectrum, isLongWin);

    // clear high frequency components
    BweClearHighBand(bweConfig, mdctSpectrum);

    return;
}
