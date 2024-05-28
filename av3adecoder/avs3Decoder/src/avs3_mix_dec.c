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

#include "avs3_options.h"
#include "avs3_rom_com.h"
#include "avs3_stat_dec.h"
#include "avs3_prot_dec.h"
#include "avs3_prot_com.h"


void Avs3MixDec(AVS3DecoderHandle hAvs3Dec, float synth[MAX_CHANNELS][FRAME_LEN])
{
    Avs3McDec(hAvs3Dec, synth);

    return;
}
