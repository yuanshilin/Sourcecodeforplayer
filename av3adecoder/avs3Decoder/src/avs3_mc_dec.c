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


void Avs3McDec(AVS3DecoderHandle hAvs3Dec, float synth[MAX_CHANNELS][FRAME_LEN])
{
    short i;
    AVS3_DEC_CORE_HANDLE hDecCore = NULL;
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream = hAvs3Dec->hBitstream;
    AVS3_MC_DEC_HANDLE hMcdec;

    short totalBits = 0;
    int32_t availableBits = 0;
    short availableBytes = 0;
    short channelBytes[MAX_CHANNELS] = { 0 };
    short nChans;
    short chBitRatios[MAX_CHANNELS] = { 0 };
    short isLfe;

    int16_t numGroups[MAX_CHANNELS];

    nChans = hAvs3Dec->numChansOutput;
    hMcdec = hAvs3Dec->hMcDec;

    // decode core side info
    for (i = 0; i < nChans; i++) {
        DecodeCoreSideBits(hAvs3Dec->hDecCore[i], hBitstream);
    }

    // grouping info
    for (i = 0; i < nChans; i++) {
        DecodeGroupBits(hAvs3Dec->hDecCore[i], hBitstream);
        numGroups[i] = hAvs3Dec->hDecCore[i]->numGroups;
    }

    // decode mode side info
    DecodeMcSideBits(hMcdec, hBitstream, chBitRatios);

    // bit alloc
    availableBits = GetAvailableBits(hAvs3Dec->bitsPerFrame, hBitstream->nextBitPos, numGroups, nChans, hAvs3Dec->nnTypeConfig);
    availableBytes = (short)floor((float)availableBits / 8.0f);

    McBitsAllocationHasSiL(availableBits, chBitRatios, hMcdec->channelNum + hMcdec->objNum, channelBytes, hMcdec->silFlag, hMcdec->lfeExist, hMcdec->lfeBytes);

    // decode QC bits
    for (i = 0; i < nChans; i++) {
        DecodeQcBits(hAvs3Dec->hDecCore[i], hAvs3Dec->nnTypeConfig, hBitstream, channelBytes[i]);
    }

    // inverse QC for all channels
    Avs3InverseQC(hAvs3Dec, nChans);

    Avs3McacDec(hMcdec);

    for (i = 0; i < nChans; i++)
    {
        hDecCore = hAvs3Dec->hDecCore[i];

        isLfe = 0;
        if (hMcdec->lfeExist == 1 && hMcdec->lfeChIdx == i) {
            isLfe = 1;
        }

        // post synthesis, including bwe, tns, fd shaping, degrouping and inv MDCT
        Avs3PostSynthesis(hDecCore, synth[i], isLfe);
    }

    return;
}
