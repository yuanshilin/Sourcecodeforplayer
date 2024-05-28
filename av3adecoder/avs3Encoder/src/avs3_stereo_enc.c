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
#include "avs3_cnst_enc.h"
#include "avs3_stat_com.h"
#include "avs3_stat_enc.h"
#include "avs3_prot_com.h"
#include "avs3_prot_enc.h"


void Avs3StereoEncoder(
    AVS3EncoderHandle stAvs3, 
    short* channelBytes
)
{
    short frameLength = stAvs3->frameLength;
    float selfCorrL, selfCorrR, crossCorr;

    float lrRatio;

    int16_t numGroups[STEREO_CHANNELS];         // num groups for each channel
    short availableBits = 0;

    AVS3_STEREO_ENC_HANDLE hMdctStereo = stAvs3->hMdctStereo;
    AVS3_ENC_CORE_HANDLE hEncCoreL = stAvs3->hEncCore[0];
    AVS3_ENC_CORE_HANDLE hEncCoreR = stAvs3->hEncCore[1];

    // Init ms flag
    hMdctStereo->isMS = 0;

    // Cross-Correlation
    crossCorr = Dotp(hEncCoreL->origSpectrum, hEncCoreR->origSpectrum, frameLength);
    selfCorrL = Dotp(hEncCoreL->origSpectrum, hEncCoreL->origSpectrum, frameLength);
    selfCorrR = Dotp(hEncCoreR->origSpectrum, hEncCoreR->origSpectrum, frameLength);

    crossCorr = (float)fabs(crossCorr) / (float)(sqrt(selfCorrL) * sqrt(selfCorrR));

    // L/R energy ratio
    lrRatio = (float)(sqrt(selfCorrL) / sqrt(selfCorrR));

    // down mix decision
    if (hEncCoreL->transformType == hEncCoreR->transformType && crossCorr > TH_CROSS_CORR)
    {
        if (lrRatio < LR_ENGERY_RATIO_H && lrRatio > LR_ENGERY_RATIO_L)
        {
            hMdctStereo->isMS = 1;

            StereoMsProcess(hEncCoreL->origSpectrum, hEncCoreR->origSpectrum, frameLength, &hMdctStereo->ILD);
        }
    }

    // grouping for short window
    for (int16_t i = 0; i < STEREO_CHANNELS; i++) {

        AVS3_ENC_CORE_HANDLE hEncCore = stAvs3->hEncCore[i];
        SpectrumGroupingEnc(hEncCore->origSpectrum, hEncCore->frameLength, hEncCore->transformType,
            hEncCore->groupIndicator, &hEncCore->numGroups);

        numGroups[i] = hEncCore->numGroups;
    }
    // write grouping bitstream
    WriteGroupBitstream(stAvs3, STEREO_CHANNELS, stAvs3->bitstream, &stAvs3->totalSideBits);

    // bit allocation
    ComputeBitsRatio(hEncCoreL->origSpectrum, hEncCoreR->origSpectrum, frameLength, hMdctStereo->isMS, &hMdctStereo->bitsRatio);

    // write stereo bits
    WriteStereoBitstream(stAvs3, stAvs3->bitstream, &stAvs3->totalSideBits);

    // calculate available bits
    availableBits = (short)GetAvailableBits(stAvs3->bitsPerFrame, stAvs3->totalSideBits, numGroups, STEREO_CHANNELS, stAvs3->nnTypeConfig);

    // allcoation bits between channels
    StereoBitsAllocation(availableBits, stAvs3->hMdctStereo->bitsRatio, channelBytes);

    return;
}


void Avs3StereoMcrEncoder(
    AVS3EncoderHandle stAvs3,
    short* channelBytes
)
{
    int16_t numGroups;                      // num groups for each channel
    short availableBits = 0;

    AVS3_STEREO_ENC_HANDLE hMdctStereo = stAvs3->hMdctStereo;
    AVS3_ENC_CORE_HANDLE hEncCoreL = stAvs3->hEncCore[0];
    AVS3_ENC_CORE_HANDLE hEncCoreR = stAvs3->hEncCore[1];

    // MCR encoding process, use transformType for left channel
    McrEncode(&hMdctStereo->mcrData, &hMdctStereo->mcrConfig, hEncCoreL->origSpectrum, hEncCoreR->origSpectrum,
        hEncCoreL->transformType == ONLY_SHORT_WINDOW);

    // grouping for short window
    // only for left channel in mcr mode
    SpectrumGroupingEnc(hEncCoreL->origSpectrum, hEncCoreL->frameLength, hEncCoreL->transformType,
        hEncCoreL->groupIndicator, &hEncCoreL->numGroups);
    numGroups = hEncCoreL->numGroups;

    // write grouping bitstream
    // only for left channel in mcr mode
    WriteGroupBitstream(stAvs3, 1, stAvs3->bitstream, &stAvs3->totalSideBits);

    // write stereo bits
    WriteStereoBitstream(stAvs3, stAvs3->bitstream, &stAvs3->totalSideBits);

    // calculate available bits
    // channel num is 1 for mcr mode
    availableBits = (short)GetAvailableBits(stAvs3->bitsPerFrame, stAvs3->totalSideBits, &numGroups, 1, stAvs3->nnTypeConfig);

    // allcoation bits between channels
    // all bits for left channel in mcr mode
    channelBytes[0] = (short)floor((float)availableBits / 8.0f);
    channelBytes[1] = 0;

    return;
}
