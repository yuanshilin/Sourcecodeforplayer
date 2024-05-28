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

#include "avs3_stat_dec.h"
#include "avs3_prot_dec.h"
#include "avs3_prot_com.h"


void Avs3StereoDec(AVS3DecoderHandle hAvs3Dec, float synth[MAX_CHANNELS][FRAME_LEN])
{
    short i;
    AVS3_DEC_CORE_HANDLE hDecCore = NULL;
    AVS3_DEC_CORE_HANDLE hDecCoreL = hAvs3Dec->hDecCore[0];
    AVS3_DEC_CORE_HANDLE hDecCoreR = hAvs3Dec->hDecCore[1];
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream = hAvs3Dec->hBitstream;

    short totalBits = 0;
    short availableBits = 0;
    short availableBytes = 0;
    short channelBytes[STEREO_CHANNELS] = { 0 };

    int16_t numGroups[STEREO_CHANNELS];

    // decode core side info
    for (i = 0; i < STEREO_CHANNELS; i++) {
        DecodeCoreSideBits(hAvs3Dec->hDecCore[i], hBitstream);
    }

    // grouping info
    for (i = 0; i < STEREO_CHANNELS; i++) {
        DecodeGroupBits(hAvs3Dec->hDecCore[i], hBitstream);
        numGroups[i] = hAvs3Dec->hDecCore[i]->numGroups;
    }

    // decode mode side info
    DecodeStereoSideBits(hAvs3Dec, hBitstream);

    // bit split
    availableBits = (short)GetAvailableBits(hAvs3Dec->bitsPerFrame, hBitstream->nextBitPos, numGroups, STEREO_CHANNELS, hAvs3Dec->nnTypeConfig);
    StereoBitsAllocation(availableBits, hAvs3Dec->hDecStereo->bitsRatio, channelBytes);

    // decode QC bits
    for (i = 0; i < STEREO_CHANNELS; i++) {
        DecodeQcBits(hAvs3Dec->hDecCore[i], hAvs3Dec->nnTypeConfig, hBitstream, channelBytes[i]);
    }

    // inverse QC for all channels
    Avs3InverseQC(hAvs3Dec, STEREO_CHANNELS);

    // inverse ms process
    if (hAvs3Dec->hDecStereo->isMS == 1) {
        StereoInvMsProcess(hDecCoreL->origSpectrum, hDecCoreR->origSpectrum, hAvs3Dec->frameLength, hAvs3Dec->hDecStereo->ILD);
    }

    for (i = 0; i < STEREO_CHANNELS; i++)
    {
        hDecCore = hAvs3Dec->hDecCore[i];

        // post synthesis, including bwe, tns, fd shaping, degrouping and inv MDCT
        Avs3PostSynthesis(hDecCore, synth[i], 0);
    }

    return;
}


void Avs3StereoMcrDec(AVS3DecoderHandle hAvs3Dec, float synth[MAX_CHANNELS][FRAME_LEN])
{
    short i;
    AVS3_DEC_CORE_HANDLE hDecCore = NULL;
    AVS3_DEC_CORE_HANDLE hDecCoreL = hAvs3Dec->hDecCore[0];
    AVS3_DEC_CORE_HANDLE hDecCoreR = hAvs3Dec->hDecCore[1];
    AVS3_STEREO_DEC_HANDLE hDecStereo = hAvs3Dec->hDecStereo;
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream = hAvs3Dec->hBitstream;

    short availableBits = 0;
    short channelBytes[STEREO_CHANNELS] = { 0 };

    int16_t numGroups;

    // decode core side info
    for (i = 0; i < STEREO_CHANNELS; i++) {
        DecodeCoreSideBits(hAvs3Dec->hDecCore[i], hBitstream);
    }

    // grouping info
    // only left channel for MCR mode
    DecodeGroupBits(hDecCoreL, hBitstream);
    numGroups = hDecCoreL->numGroups;

    // decode mode side info
    DecodeStereoSideBits(hAvs3Dec, hBitstream);

    // bit split
    // number channel is 1 for MCR mode
    availableBits = (short)GetAvailableBits(hAvs3Dec->bitsPerFrame, hBitstream->nextBitPos, &numGroups, 1, hAvs3Dec->nnTypeConfig);

    // allcoation bits between channels
    // all bits for left channel in mcr mode
    channelBytes[0] = (short)floor((float)availableBits / 8.0f);
    channelBytes[1] = 0;

    // decode QC bits
    // only left channel for MCR mode
    DecodeQcBits(hDecCoreL, hAvs3Dec->nnTypeConfig, hBitstream, channelBytes[0]);

    // inverse QC for all channels
    // number channel is 1 for MCR mode
    Avs3InverseQC(hAvs3Dec, 1);

    // inverse MCR process
    McrDecode(&hDecStereo->mcrData, &hDecStereo->mcrConfig, hDecCoreL->origSpectrum, hDecCoreR->origSpectrum,
        hDecCoreL->transformType == ONLY_SHORT_WINDOW);

    for (i = 0; i < STEREO_CHANNELS; i++)
    {
        hDecCore = hAvs3Dec->hDecCore[i];

        // post synthesis, including bwe, tns, fd shaping, degrouping and inv MDCT
        Avs3PostSynthesis(hDecCore, synth[i], 0);
    }

    return;
}
