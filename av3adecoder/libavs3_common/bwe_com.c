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
#include <assert.h>
#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_rom_com.h"
#include "avs3_stat_com.h"
#include "avs3_prot_com.h"


/*
Get bwe present flag
I/O params:
    const int16_t avs3Format            (i) avs3 format info
    const int32_t totalBitrate          (i) codec total bitrate per second
    const int16_t numChannels           (i) number of channels for core coder
    ret                                 (o) bwe present flag
*/
int16_t GetBwePresent(
    const int16_t avs3Format,
    const int32_t totalBitrate,
    const int16_t numChannels
)
{
    int16_t isBwePresent = 0;
    int32_t bitratePerCpe;

    if (avs3Format == AVS3_MONO_FORMAT) {
        if (totalBitrate <= 96000) {
            isBwePresent = 1;
        }
    }
    else if (avs3Format == AVS3_STEREO_FORMAT) {
        if (totalBitrate <= 128000) {
            isBwePresent = 1;
        }
    }
    else if (avs3Format == AVS3_MC_FORMAT || avs3Format == AVS3_MIX_FORMAT) {
        bitratePerCpe = (int32_t)((float)(totalBitrate * STEREO_CHANNELS) / (float)numChannels);
        if (bitratePerCpe <= 128000) {
            isBwePresent = 1;
        }
    }
    else if (avs3Format == AVS3_INNER_FOA_FORMAT || avs3Format == AVS3_INNER_HOA2_FORMAT || avs3Format == AVS3_INNER_HOA3_FORMAT)
    {
        if (avs3Format == AVS3_INNER_FOA_FORMAT)
        {
            bitratePerCpe = (int32_t)((float)(totalBitrate * STEREO_CHANNELS) / (float)numChannels);
            if (bitratePerCpe <= 128000) {
                isBwePresent = 1;
            }
        }
        else if (avs3Format == AVS3_INNER_HOA2_FORMAT)
        {
            if (totalBitrate <= 480000) {
                isBwePresent = 1;
            }
        }
        else {
            isBwePresent = 1;
        }
    }

    return isBwePresent;
}


/*
Get bwe bitrate index for bwe configuration
I/O params:
    const int16_t avs3Format            (i) avs3 format info
    const int32_t totalBitrate          (i) codec total bitrate per second
    const int16_t numChannels           (i) number of channels for core coder
    ret                                 (o) bwe bitrate index
*/
static BweRateIndex BweGetRateIndex(
    const int16_t avs3Format,
    const int32_t totalBitrate,
    const int16_t numChannels
)
{
    int16_t bitRateIndex = BWE_BITRATE_FB_UNKNOWN;
    int32_t bitratePerCpe;

    if (avs3Format == AVS3_MONO_FORMAT) {

        // BWE available bitrate for Mono
        // 32/48/64/96 kbps
        if (totalBitrate <= 32000) {
            bitRateIndex = BWE_BITRATE_FB_MONO_32K;
        }
        else if (totalBitrate == 44000 || totalBitrate == 56000) {
            bitRateIndex = BWE_BITRATE_FB_MONO_48K;
        }
        else if (totalBitrate == 64000 || totalBitrate == 72000) {
            bitRateIndex = BWE_BITRATE_FB_MONO_64K;
        }
        else if (totalBitrate == 80000 || totalBitrate == 96000) {
            bitRateIndex = BWE_BITRATE_FB_MONO_96K;
        }
    }
    else if (avs3Format == AVS3_STEREO_FORMAT) {

        // BWE available bitrate for stereo
        // 48/64/96/128 kbps
        if (totalBitrate <= 48000) {
            bitRateIndex = BWE_BITRATE_FB_STEREO_48K;
        }
        else if (totalBitrate <= 64000) {
            bitRateIndex = BWE_BITRATE_FB_STEREO_64K;
        }
        else if (totalBitrate <= 96000) {
            bitRateIndex = BWE_BITRATE_FB_STEREO_96K;
        }
        else if (totalBitrate <= 128000) {
            bitRateIndex = BWE_BITRATE_FB_STEREO_128K;
        }
    }
    else if (avs3Format == AVS3_MC_FORMAT || avs3Format == AVS3_MIX_FORMAT) {

        // get bitrate per cpe to determine BWE bitrate index
        bitratePerCpe = (int32_t)((float)(totalBitrate * STEREO_CHANNELS) / (float)numChannels);

        if (bitratePerCpe <= 56000) {
            bitRateIndex = BWE_BITRATE_FB_MC_CPE_48K;
        }
        else if (bitratePerCpe <= 75000) {
            bitRateIndex = BWE_BITRATE_FB_MC_CPE_64K;
        }
        else if (bitratePerCpe <= 108000) {
            bitRateIndex = BWE_BITRATE_FB_MC_CPE_96K;
        }
        else if (bitratePerCpe <= 128000) {
            bitRateIndex = BWE_BITRATE_FB_MC_CPE_128K;
        }
    }
    else if (avs3Format == AVS3_INNER_FOA_FORMAT || avs3Format == AVS3_INNER_HOA2_FORMAT
    || avs3Format == AVS3_INNER_HOA3_FORMAT)
    {
    if (avs3Format == AVS3_INNER_FOA_FORMAT) {
        if (totalBitrate <= 128000)
        {
            bitRateIndex = BWE_BITRATE_FB_HOA_LOW;
        }
        else if (totalBitrate == 192000) {
            bitRateIndex = BWE_BITRATE_FB_HOA_MIDDLE;
        }
        else if (totalBitrate == 256000) {
            bitRateIndex = BWE_BITRATE_FB_HOA_HIGH;
        }
    }
    else if (avs3Format == AVS3_INNER_HOA2_FORMAT) {
        if (totalBitrate == 192000) {
            bitRateIndex = BWE_BITRATE_FB_HOA_ELOW;
        }
        else if (totalBitrate == 256000) {
            bitRateIndex = BWE_BITRATE_FB_HOA_LOW;
        }
        else if (totalBitrate == 320000) {
            bitRateIndex = BWE_BITRATE_FB_HOA_MIDDLE;
        }
        else if (totalBitrate >= 384000 && totalBitrate <= 480000) {
            bitRateIndex = BWE_BITRATE_FB_HOA_HIGH;
        }
    }
    else if (avs3Format == AVS3_INNER_HOA3_FORMAT) {
        if (totalBitrate >= 256000 && totalBitrate <= 384000) {
            bitRateIndex = BWE_BITRATE_FB_HOA_LOW;
        }
        else if (totalBitrate == 512000) {
            bitRateIndex = BWE_BITRATE_FB_HOA_MIDDLE;
        }
        else if (totalBitrate >= 640000) {
            bitRateIndex = BWE_BITRATE_FB_HOA_HIGH;
        }
        else
        {
            assert(!"Not support HOA bitrate!\n");
        }
    }
    }

    return bitRateIndex;
}


/*
Get bwe configuration
I/O params:
    BweConfigHandle bweConfig           (i/o) bwe config handle
    const int16_t avs3Format            (i)   avs3 format info
    const int32_t totalBitrate          (i)   codec total bitrate per second
    const int16_t numChannels           (i)   number of channels for core coder
*/
void BweGetConfig(
    BweConfigHandle bweConfig,
    const int16_t avs3Format,
    const int32_t totalBitrate,
    const int16_t numChannels
)
{
    int16_t i;
    int16_t bitRateIndex;
    const int16_t *targetTiles;
    const int16_t *sfbTable;

    // get bwe bitrate index for configuration
    bitRateIndex = BweGetRateIndex(avs3Format, totalBitrate, numChannels);
    if (bitRateIndex == BWE_BITRATE_FB_UNKNOWN) {
        fprintf(stderr, "Error in BWE bitrate configuration!!\n");
    }

    // get number tiles
    bweConfig->numTiles = bweTargetTileTable[bitRateIndex][0];

    // get target tile table
    SetShort(bweConfig->targetTiles, 0, MAX_NUM_TILE + 1);
    targetTiles = &bweTargetTileTable[bitRateIndex][1];
    for (i = 0; i < bweConfig->numTiles + 1; i++) {
        bweConfig->targetTiles[i] = targetTiles[i];
    }

    // get bwe start and stop line
    bweConfig->bweStartLine = bweConfig->targetTiles[0];
    bweConfig->bweStopLine = bweConfig->targetTiles[bweConfig->numTiles];

    // get number of sfbs
    bweConfig->numSfb = bweSfbTable[bitRateIndex][0];

    // get sfb table
    SetShort(bweConfig->sfbTable, 0, MAX_NUM_SFB_BWE + 1);
    sfbTable = &bweSfbTable[bitRateIndex][1];
    for (i = 0; i < bweConfig->numSfb + 1; i++) {
        bweConfig->sfbTable[i] = sfbTable[i];
    }

    // get source tile table
    SetShort(bweConfig->srcTiles, 0, MAX_NUM_TILE);
    for (i = 0; i < bweConfig->numTiles; i++) {
        bweConfig->srcTiles[i] = bweSrcTileTable[bitRateIndex][i];
    }

    // get sfb-tile wrap table
    SetShort(bweConfig->sfbTileWrap, 0, MAX_NUM_TILE + 1);
    for (i = 0; i < bweConfig->numTiles + 1; i++) {
        bweConfig->sfbTileWrap[i] = bweSfbTileWrapTable[bitRateIndex][i];
    }

    return;
}


/*
Get SFM parameter for spectrum whitening decision
I/O params:
    float *enerSpec             (i) mdct energy spectrum
    float *logEnerSpec          (i) log mdct energy spectrum
    int16_t start               (i) start bin index of spectrum
    int16_t stop                (i) stop bin index of spectrum
    ret                         (o) SFM parameter
*/
float BweGetSfm(
    float *enerSpec,
    float *logEnerSpec,
    int16_t start,
    int16_t stop
)
{
    int16_t i;
    float num = 0.0f;
    float denom = 1.0f;
    float sfm = 1.0f;

    for (i = start; i < stop; i++) {
        num += logEnerSpec[i];
        denom += enerSpec[i];
    }

    num /= (float)(stop - start);
    denom /= (float)(stop - start);

    if (denom != 0.0f) {
        sfm = min(1.0f, (float)pow(2.0, num + 0.5) / denom);
    }

    return sfm;
}


/*
Get peak-average-ratio of log mdct energy spectrum
I/O params:
    float *logEnerSpec              (i) log mdct energy spectrum
    int16_t start                   (i) start bin index of spectrum
    int16_t stop                    (i) stop bin index of spectrum
    ret                             (o) peak-average-ratio parameter
*/
float BweGetPeakAvgRatio(
    float *logEnerSpec,
    int16_t start,
    int16_t stop
)
{
    int16_t i;
    float maxLineEner = 0.0f;
    float avgLineEner = 0.0f;
    float peakAvgRatio = 0.0f;

    for (i = start; i < stop; i++) {
        if (maxLineEner < logEnerSpec[i]) {
            maxLineEner = logEnerSpec[i];
        }
        avgLineEner += logEnerSpec[i];
    }
    avgLineEner /= (float)(stop - start);

    if (avgLineEner == 0.0f) {
        avgLineEner = 0.01f;
    }

    peakAvgRatio = max(1.0f, maxLineEner / avgLineEner);

    return peakAvgRatio;
}
