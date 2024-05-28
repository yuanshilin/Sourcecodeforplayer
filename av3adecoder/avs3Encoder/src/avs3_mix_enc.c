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


void Avs3MixEncoder(
    AVS3EncoderHandle stAvs3,
    short* channelBytes
)
{
    // use MC encoder
    Avs3McEncoder(stAvs3, channelBytes);

    return;
}
