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
#include <stdint.h>
#include <math.h>

#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_stat_com.h"
#include "avs3_stat_dec.h"
#include "avs3_prot_com.h"
#include "avs3_prot_dec.h"

#include "avs3_dec_lib.h"


void MvByte2Byte(const int8_t srcByte[], int8_t destByte[], const short n)
{
    for (int16_t i = 0; i < n; i++) {
        destByte[i] = srcByte[i];
    }

    return;
}


int16_t Avs3DecoderLibCreate(
    Avs3DecoderLibHandle * const hAvs3DecLib,
    uint8_t *headerBs,
    const char *modelPath)
{
    FILE* fModel = NULL;

    *hAvs3DecLib = (Avs3DecoderLibHandle)malloc(sizeof(struct Avs3DecoderLib));

    (*hAvs3DecLib)->hAvs3Dec = (AVS3DecoderHandle)malloc(sizeof(AVS3Decoder));

    Avs3ParseBsFrameHeader((*hAvs3DecLib)->hAvs3Dec, headerBs, 1, NULL);

    Avs3InitDecoder((*hAvs3DecLib)->hAvs3Dec, &fModel, modelPath);

    if (fModel != NULL) {
        fclose(fModel);
    }

    return 0;
}


int16_t Avs3DecoderLibParseHeader(
    Avs3DecoderLibHandle const hAvs3DecLib,
    uint8_t *headerBs,
    int16_t *rewind,
    int16_t *bytesPerFrame)
{
    Avs3ParseBsFrameHeader(hAvs3DecLib->hAvs3Dec, headerBs, 0, &hAvs3DecLib->crcBs);

    *rewind = 0;
    if (hAvs3DecLib->hAvs3Dec->isMixedContent == 0) {
        *rewind = 2;
    } else if (hAvs3DecLib->hAvs3Dec->soundBedType == 0) {
        *rewind = 1;
    }

    *bytesPerFrame = (int16_t)(ceil((float)hAvs3DecLib->hAvs3Dec->bitsPerFrame / 8));
    hAvs3DecLib->bytesPerFrame = *bytesPerFrame;

    return 0;
}


int16_t Avs3DecoderLibProcess(
    Avs3DecoderLibHandle const hAvs3DecLib,
    uint8_t *payload,
    int16_t *data,
    Avs3MetaDataHandle avs3MetaData)
{
    uint8_t *bitstream = hAvs3DecLib->hAvs3Dec->hBitstream->bitstream;
    for (int16_t i = 0; i < hAvs3DecLib->bytesPerFrame; i++) {
        bitstream[i] = payload[i];
    }

    uint16_t crcResult;
    crcResult = Crc16(bitstream, (uint32_t)hAvs3DecLib->bytesPerFrame);
    if (crcResult != hAvs3DecLib->crcBs) {
        return -1;
    }

    Avs3Decode(hAvs3DecLib->hAvs3Dec, data);

    MvByte2Byte((int8_t *)(&(hAvs3DecLib->hAvs3Dec->hMetadataDec->avs3MetaData)),
        (int8_t *)avs3MetaData, sizeof(Avs3MetaData));

    ResetBitstream(hAvs3DecLib->hAvs3Dec->hBitstream);

    return 0;
}


int16_t Avs3DecoderLibClose(
    Avs3DecoderLibHandle * const hAvs3DecLib)
{
    Avs3DecoderDestroy((*hAvs3DecLib)->hAvs3Dec);
    (*hAvs3DecLib)->hAvs3Dec = NULL;

    free(*hAvs3DecLib);
    *hAvs3DecLib = NULL;

    return 0;
}


int16_t Avs3DecoderLibGetConfig(
    Avs3DecoderLibHandle const hAvs3DecLib,
    Avs3DecoderLibConfig *hAvs3DecLibConfig)
{
    hAvs3DecLibConfig->numChsOutput = hAvs3DecLib->hAvs3Dec->numChansOutput;
    hAvs3DecLibConfig->sampleRate = (int32_t)hAvs3DecLib->hAvs3Dec->outputFs;

    return 0;
}


FILE *Avs3DecoderLibOpenWavFile(
    Avs3DecoderLibHandle const hAvs3DecLib,
    const char* fileName)
{
    FILE *fWav;

    fWav = WriteWavHeader(fileName, hAvs3DecLib->hAvs3Dec->numChansOutput, hAvs3DecLib->hAvs3Dec->outputFs);

    return fWav;
}


void Avs3DecoderLibWriteWavData(
    Avs3DecoderLibHandle const hAvs3DecLib,
    const int16_t* data,
    FILE* fOutput)
{
    WriteSynthData(data, fOutput, hAvs3DecLib->hAvs3Dec->numChansOutput, hAvs3DecLib->hAvs3Dec->frameLength);

    return;
}


void Avs3DecoderLibUpdateWavHeader(
    FILE* fOutput)
{
    SynthWavHeader(fOutput);

    return;
}
