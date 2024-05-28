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
#include "avs3_stat_dec.h"
#include "avs3_prot_com.h"
#include "avs3_prot_dec.h"


void Avs3MonoDec(AVS3DecoderHandle hAvs3Dec, float synth[MAX_CHANNELS][FRAME_LEN])
{
    AVS3_DEC_CORE_HANDLE hDecCore = hAvs3Dec->hDecCore[0];
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream = hAvs3Dec->hBitstream;

    short availableBits = 0;
    short availableBytes = 0;

    int16_t numGroups;

    // decode core side info
    DecodeCoreSideBits(hDecCore, hBitstream);

    // grouping info
    DecodeGroupBits(hDecCore, hBitstream);
    numGroups = hDecCore->numGroups;

    // get num of available bytes
    availableBits = (short)GetAvailableBits(hAvs3Dec->bitsPerFrame, hBitstream->nextBitPos, &numGroups, 1, hAvs3Dec->nnTypeConfig);
    availableBytes = (short)floor((float)availableBits / 8.0f);

    // decode QC bits
    DecodeQcBits(hDecCore, hAvs3Dec->nnTypeConfig, hBitstream, availableBytes);

    // inverse QC for all channels
    Avs3InverseQC(hAvs3Dec, 1);

    // post synthesis, including bwe, tns, fd shaping, degrouping and inv MDCT
    Avs3PostSynthesis(hDecCore, synth[0], 0);

    return;
}
