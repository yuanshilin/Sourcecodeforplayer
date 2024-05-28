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

#ifndef AVS3_DEC_LIB_H
#define AVS3_DEC_LIB_H

#include "avs3_stat_meta.h"
#include "avs3_stat_dec.h"

struct Avs3DecoderLib {
    AVS3DecoderHandle hAvs3Dec;

    uint16_t crcBs;
    int16_t bytesPerFrame;
};

typedef struct Avs3DecoderLib *Avs3DecoderLibHandle;

typedef struct Avs3DecoderLibConfigStruct {
    int32_t sampleRate;
    int16_t numChsOutput;
}Avs3DecoderLibConfig;

int16_t Avs3DecoderLibCreate(
    Avs3DecoderLibHandle * const hAvs3DecLib,
    uint8_t *headerBs,
    const char *modelPath);

int16_t Avs3DecoderLibParseHeader(
    Avs3DecoderLibHandle const hAvs3DecLib,
    uint8_t *headerBs,
    int16_t *rewind,
    int16_t *bytesPerFrame);

int16_t Avs3DecoderLibProcess(
    Avs3DecoderLibHandle const hAvs3DecLib,
    uint8_t *payload,
    int16_t *data,
    Avs3MetaDataHandle avs3MetaData);

int16_t Avs3DecoderLibClose(
    Avs3DecoderLibHandle * const hAvs3DecLib);

int16_t Avs3DecoderLibGetConfig(
    Avs3DecoderLibHandle const hAvs3DecLib,
    Avs3DecoderLibConfig *hAvs3DecLibConfig);

FILE *Avs3DecoderLibOpenWavFile(
    Avs3DecoderLibHandle const hAvs3DecLib,
    const char* fileName);

void Avs3DecoderLibWriteWavData(
    Avs3DecoderLibHandle const hAvs3DecLib,
    const int16_t* data,
    FILE* fOutput);

void Avs3DecoderLibUpdateWavHeader(
    FILE* fOutput);

#endif
