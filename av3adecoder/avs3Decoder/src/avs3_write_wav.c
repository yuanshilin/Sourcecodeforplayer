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
#include <string.h>
#include "avs3_prot_dec.h"
#include "avs3_stat_com.h"

FILE* WriteWavHeader(const char* fileName, const short nChans, const long fs)
{
    FILE* fWav = NULL;

    AVS3_WAVE_HEADER_DATA wavHeader;

    /* RIFF */
    strncpy(wavHeader.chunkID, "RIFF", sizeof(wavHeader.chunkID));

    /* RIFF size */
    wavHeader.riffSize = 0;

    /* Format */
    strncpy(wavHeader.format, "WAVE", sizeof(wavHeader.format));

    /* Format ID */
    strncpy(wavHeader.subchunkID, "fmt ", sizeof(wavHeader.subchunkID));

    /* Format length */
    wavHeader.subchunkSize = 16;

    /* Wave */
    wavHeader.audioFormat = 1;

    /* Channels */
    wavHeader.numChannels = nChans;

    /* sampling rate */
    wavHeader.sampleRate = (int32_t)fs;
    
    /* bit depth */
    wavHeader.bitDepth = 16;

    /* align */
    wavHeader.blockAlign = (wavHeader.bitDepth >> 3)* wavHeader.numChannels;

    /* Bytes rate */
    wavHeader.byteRate = wavHeader.sampleRate * wavHeader.blockAlign;

    /* Data tag */
    strncpy(wavHeader.dataID, "data", sizeof(wavHeader.dataID));

    wavHeader.dataSize = 0;

    if ((fWav = fopen(fileName, "wb+")) == NULL) 
    {
        fprintf(stderr, "Open wave file error!\n");
        exit(-1);
    }

    fwrite(&wavHeader, sizeof(wavHeader), 1, fWav);

    fflush(fWav);

    return fWav;
}
