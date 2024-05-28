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


void Avs3MonoEncoder(
    AVS3EncoderHandle stAvs3,
    short* channelBytes
)
{
    short availableBits = 0;
    short availableBytes = 0;

    int16_t numGroups;
    AVS3_ENC_CORE_HANDLE hEncCore = stAvs3->hEncCore[0];

    // grouping for short window
    SpectrumGroupingEnc(hEncCore->origSpectrum, hEncCore->frameLength, hEncCore->transformType,
        hEncCore->groupIndicator, &hEncCore->numGroups);
    numGroups = hEncCore->numGroups;

    // write grouping bitstream
    WriteGroupBitstream(stAvs3, 1, stAvs3->bitstream, &stAvs3->totalSideBits);

    // get num of available bytes
    availableBits = (short)GetAvailableBits(stAvs3->bitsPerFrame, stAvs3->totalSideBits, &numGroups, 1, stAvs3->nnTypeConfig);
    availableBytes = (short)floor((float)availableBits / 8.0f);

    channelBytes[0] = availableBytes;

    return;
}
