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

#include <math.h>

#include "avs3_prot_com.h"
#include "avs3_prot_enc.h"
#include "avs3_rom_com.h"
#include "avs3_stat_meta.h"
#include "avs3_const_meta.h"

extern FILE *fori;

// vrEXT encoding
static int16_t  Avs3VrExtEqEffectEnc(
    EqEffectHandle hEqEffect,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    float tmp;
    int16_t halfFlag, tmpValue;

    // eqType
    fread(&(hEqEffect->eqType), sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hEqEffect->eqType, NBITS_VR_EQTYPE);
    //fprintf(fori, "%d\n", hEqEffect->eqType);

    // eqFc
    fread(&(hEqEffect->eqFc), sizeof(float), 1, fMetadata);
    //convert to db
    tmp = AVS3_MAX(hEqEffect->eqFc, 20.0f);
    tmp = 20.0f * log10f(tmp) - 20.0f * log10f(20.0f);
    tmpValue = (int16_t)floor(tmp / RES_VREXT_EQFC + 0.5f);
    PushNextIndice(bsHandle, tmpValue, NBITS_VR_EQFC);
    //fprintf(fori, "%f\n", hEqEffect->eqFc);

    // eqQ
    fread(&(hEqEffect->eqQ), sizeof(float), 1, fMetadata);
    //int16_t halfFlag, tmpValue;
    if (hEqEffect->eqQ <= 1.0f) {
        halfFlag = 0;
        PushNextIndice(bsHandle, halfFlag, NBITS_META_FLAG);
        tmpValue = (int16_t)floor((hEqEffect->eqQ - 0.1f) / RES_VREXT_EQQ_H1 + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_VR_EQQ);
    }
    else {
        halfFlag = 1;
        PushNextIndice(bsHandle, halfFlag, NBITS_META_FLAG);
        tmpValue = (int16_t)floor((hEqEffect->eqQ - 1.0f) / RES_VREXT_EQQ_H2 + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_VR_EQQ);
    }
    //fprintf(fori, "%f\n", hEqEffect->eqQ);

    // eqGain
    fread(&(hEqEffect->eqGain), sizeof(float), 1, fMetadata);
    tmpValue = (int16_t)floor(hEqEffect->eqGain / RES_VREXT_EQGAIN + 0.5f);
    tmpValue += (1 << (NBITS_VR_EQGAIN - 1));
    tmpValue = AVS3_MIN(tmpValue, (1 << NBITS_VR_EQGAIN) - 1);
    PushNextIndice(bsHandle, tmpValue, NBITS_VR_EQGAIN);
    //fprintf(fori, "%f\n", hEqEffect->eqGain);

    return 0;
}


static int16_t  Avs3VrExtAudioEffectEnc(
    AudioEffectHandle hAudioEffect,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t tmpValue;

    // hasEQExist
    fread(&hAudioEffect->hasEQExist, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioEffect->hasEQExist, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioEffect->hasEQExist);

    // hasDRCExist
    fread(&hAudioEffect->hasDRCExist, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioEffect->hasDRCExist, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioEffect->hasDRCExist);

    // hasGainExist
    fread(&hAudioEffect->hasGainExist, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioEffect->hasGainExist, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioEffect->hasGainExist);

    // effectChain
    if (hAudioEffect->hasEQExist || hAudioEffect->hasDRCExist || hAudioEffect->hasGainExist) {
        fread(&(hAudioEffect->effectChain), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioEffect->effectChain, NBITS_VR_EFFCHAIN);
        //fprintf(fori, "%d\n", hAudioEffect->effectChain);
    }

    if (hAudioEffect->hasEQExist) {
        // numEqband
        fread(&(hAudioEffect->numEqband), sizeof(int16_t), 1, fMetadata);
        tmpValue = hAudioEffect->numEqband - 1;
        PushNextIndice(bsHandle, tmpValue, NBITS_VR_NUMEQBAND);
        //fprintf(fori, "%d\n", hAudioEffect->numEqband);

        // EqEffect params
        for (int16_t i = 0; i < hAudioEffect->numEqband; i++) {
            EqEffectHandle hEqEffect = &hAudioEffect->eqEffect[i];
            Avs3VrExtEqEffectEnc(hEqEffect, fMetadata, bsHandle);
        }
    }

    // DRC params
    if (hAudioEffect->hasDRCExist) {
        // attackTime
        fread(&(hAudioEffect->attackTime), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor((hAudioEffect->attackTime - 1.0f) / RES_VREXT_ATT_TIME + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_VR_ATT_TIME);
        //fprintf(fori, "%f\n", hAudioEffect->attackTime);

        // releaseTime
        fread(&(hAudioEffect->releaseTime), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor((hAudioEffect->releaseTime - 50.0f) / RES_VREXT_REL_TIME + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_VR_REL_TIME);
        //fprintf(fori, "%f\n", hAudioEffect->releaseTime);

        // threshold
        fread(&(hAudioEffect->threshold), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor((hAudioEffect->threshold + 80.0f) / RES_VREXT_THRESHOLD + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_VR_THRESHOLD);
        //fprintf(fori, "%f\n", hAudioEffect->threshold);

        // preGain
        fread(&(hAudioEffect->preGain), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor((hAudioEffect->preGain) / RES_VREXT_PREGAIN + 0.5f);
        tmpValue += (1 << (NBITS_VR_PREGAIN - 1));
        tmpValue = AVS3_MIN(tmpValue, (1 << NBITS_VR_PREGAIN) - 1);
        PushNextIndice(bsHandle, tmpValue, NBITS_VR_PREGAIN);
        //fprintf(fori, "%f\n", hAudioEffect->preGain);

        // postGain
        fread(&(hAudioEffect->postGain), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor((hAudioEffect->postGain) / RES_VREXT_POSTGAIN + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_VR_POSTGAIN);
        //fprintf(fori, "%f\n", hAudioEffect->postGain);

        // ratio
        fread(&(hAudioEffect->ratio), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor((hAudioEffect->ratio - 1.0f) / RES_VREXT_RATIO + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_VR_RATIO);
        //fprintf(fori, "%f\n", hAudioEffect->ratio);
    }

    // effecGain
    if (hAudioEffect->hasGainExist) {
        fread(&(hAudioEffect->effectGain), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor((hAudioEffect->effectGain) / RES_VREXT_EFFGAIN + 0.5f);
        tmpValue += (1 << (NBITS_VR_EFFGAIN - 1));
        tmpValue = AVS3_MIN(tmpValue, (1 << NBITS_VR_EFFGAIN) - 1);
        PushNextIndice(bsHandle, tmpValue, NBITS_VR_EFFGAIN);
        //fprintf(fori, "%f\n", hAudioEffect->effectGain);
    }

    return 0;
}


static int16_t  Avs3VrExtVertexEnc(
    VertexHandle hVertex,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t tmpValue;

    // vertex x
    fread(&hVertex->x, sizeof(float), 1, fMetadata);
    tmpValue = (int16_t)floor(hVertex->x / RES_VREXT_VERTEX_X + 0.5f);
    tmpValue += (1 << (NBITS_VR_VER_X - 1));
    tmpValue = AVS3_MIN(tmpValue, (1 << NBITS_VR_VER_X) - 1);
    PushNextIndice(bsHandle, tmpValue, NBITS_VR_VER_X);
    //fprintf(fori, "%f\n", hVertex->x);

    // vertex y
    fread(&hVertex->y, sizeof(float), 1, fMetadata);
    tmpValue = (int16_t)floor(hVertex->y / RES_VREXT_VERTEX_Y + 0.5f);
    tmpValue += (1 << (NBITS_VR_VER_Y - 1));
    tmpValue = AVS3_MIN(tmpValue, (1 << NBITS_VR_VER_Y) - 1);
    PushNextIndice(bsHandle, tmpValue, NBITS_VR_VER_Y);
    //fprintf(fori, "%f\n", hVertex->y);

    // vertex z
    fread(&hVertex->z, sizeof(float), 1, fMetadata);
    tmpValue = (int16_t)floor(hVertex->z / RES_VREXT_VERTEX_Z + 0.5f);
    tmpValue += (1 << (NBITS_VR_VER_Z - 1));
    tmpValue = AVS3_MIN(tmpValue, (1 << NBITS_VR_VER_Z) - 1);
    PushNextIndice(bsHandle, tmpValue, NBITS_VR_VER_Z);
    //fprintf(fori, "%f\n", hVertex->z);

    return 0;
}


static int16_t  Avs3VrExtSurfaceEnc(
    SurfaceHandle hSurface,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t tmpValue;

    // material
    fread(&(hSurface->material), sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hSurface->material, NBITS_VR_MARTERIAL);
    //fprintf(fori, "%d\n", hSurface->material);

    if (hSurface->material == 31) {
        for (int16_t i = 0; i < 8; i++) {
            // absorption
            fread(&hSurface->absorption[i], sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hSurface->absorption[i] / RES_VREXT_ABSORPTION + 0.5f);
            PushNextIndice(bsHandle, tmpValue, NBITS_VR_ABSORPTION);
            //fprintf(fori, "%f\n", hSurface->absorption[i]);

            // scattering
            fread(&hSurface->scattering[i], sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hSurface->scattering[i] / RES_VREXT_SCATTERING + 0.5f);
            PushNextIndice(bsHandle, tmpValue, NBITS_VR_SCATTERING);
            //fprintf(fori, "%f\n", hSurface->scattering[i]);
        }
    }

    // numVertices
    fread(&(hSurface->numVertices), sizeof(int16_t), 1, fMetadata);
    tmpValue = hSurface->numVertices - 1;
    PushNextIndice(bsHandle, tmpValue, NBITS_VR_NUM_VER);
    //fprintf(fori, "%d\n", hSurface->numVertices);

    // vertex encoding
    for (int16_t i = 0; i < hSurface->numVertices; i++) {
        VertexHandle hVertex = &hSurface->vertex[i];
        Avs3VrExtVertexEnc(hVertex, fMetadata, bsHandle);
    }

    return 0;
}


static int16_t Avs3VrExtRenderInfoEnc(
    RenderInfoHandle hRenderInfo,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    // targetDevice
    fread(&(hRenderInfo->targetDevice), sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hRenderInfo->targetDevice, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hRenderInfo->targetDevice);

    // hrtfType
    fread(&(hRenderInfo->hrtfType), sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hRenderInfo->hrtfType, NBITS_VR_HRTFTYPE);
    //fprintf(fori, "%d\n", hRenderInfo->hrtfType);

    // headphoneType
    for (int16_t i = 0; i < 16; i++) {
        fread(&(hRenderInfo->headphoneType[i]), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hRenderInfo->headphoneType[i], NBITS_VR_HPTYPE);
        //fprintf(fori, "%d\n", hRenderInfo->headphoneType[i]);
    }

    // audio effect encoding
    AudioEffectHandle hAudioEffect = &hRenderInfo->audioEffect;
    Avs3VrExtAudioEffectEnc(hAudioEffect, fMetadata, bsHandle);

    return 0;
}


static int16_t  Avs3VrExtAcousticEnvEnc(
    AcousticEnvHandle hAcousticEnv,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t tmpValue;

    // hasEarlyReflectionGain
    fread(&hAcousticEnv->hasEarlyReflectionGain, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAcousticEnv->hasEarlyReflectionGain, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAcousticEnv->hasEarlyReflectionGain);

    // hasLateReverbGain
    fread(&hAcousticEnv->hasLateReverbGain, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAcousticEnv->hasLateReverbGain, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAcousticEnv->hasLateReverbGain);

    // reverbType
    fread(&hAcousticEnv->reverbType, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAcousticEnv->reverbType, NBITS_VR_REVERBTYPE);
    //fprintf(fori, "%d\n", hAcousticEnv->reverbType);

    // earlyReflectionGain
    if (hAcousticEnv->hasEarlyReflectionGain == 1) {
        fread(&(hAcousticEnv->earlyReflectionGain), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hAcousticEnv->earlyReflectionGain / RES_VREXT_EAR_REF_GAIN + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_VR_EARLY_REFGAIN);
        //fprintf(fori, "%f\n", hAcousticEnv->earlyReflectionGain);
    }

    // lateReverbGain
    if (hAcousticEnv->hasLateReverbGain == 1) {
        fread(&(hAcousticEnv->lateReverbGain), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hAcousticEnv->lateReverbGain / RES_VREXT_LATE_REV_GAIN + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_VR_LATE_REVGAIN);
        //fprintf(fori, "%f\n", hAcousticEnv->lateReverbGain);
    }

    // lowFreqProFlag
    fread(&(hAcousticEnv->lowFreqProFlag), sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAcousticEnv->lowFreqProFlag, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAcousticEnv->lowFreqProFlag);

    // convolutionReverbType
    if (hAcousticEnv->reverbType == 2) {
        fread(&(hAcousticEnv->convolutionReverbType), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAcousticEnv->convolutionReverbType, NBITS_VR_CONV_REVTYPE);
        //fprintf(fori, "%d\n", hAcousticEnv->convolutionReverbType);
    }

    // numSurface
    fread(&(hAcousticEnv->numSurface), sizeof(int16_t), 1, fMetadata);
    tmpValue = hAcousticEnv->numSurface - 1;
    PushNextIndice(bsHandle, tmpValue, NBITS_VR_NUMSURFACE);
    //fprintf(fori, "%d\n", hAcousticEnv->numSurface);

    // surface encoding
    for (int16_t i = 0; i < hAcousticEnv->numSurface; i++) {
        SurfaceHandle hSurface = &hAcousticEnv->surface[i];
        Avs3VrExtSurfaceEnc(hSurface, fMetadata, bsHandle);
    }

    return 0;
}

static int16_t Avs3VrExtMetaDataEnc(
    Avs3VrExtL1MetaDataHandle hVrExtMetaData,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    // hasAcousticEnv
    fread(&hVrExtMetaData->hasAcousticEnv, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hVrExtMetaData->hasAcousticEnv, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hVrExtMetaData->hasAcousticEnv);

    // hasRenderInfo
    fread(&hVrExtMetaData->hasRenderInfo, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hVrExtMetaData->hasRenderInfo, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hVrExtMetaData->hasRenderInfo);

    // ambisonicOrder
    fread(&hVrExtMetaData->ambisonicOrder, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hVrExtMetaData->ambisonicOrder, NBITS_VR_AMBIS_ORDER);
    //fprintf(fori, "%d\n", hVrExtMetaData->ambisonicOrder);

    if (hVrExtMetaData->hasAcousticEnv == 1) {
        AcousticEnvHandle hAcousticEnv = &hVrExtMetaData->acousticEnv;
        Avs3VrExtAcousticEnvEnc(hAcousticEnv, fMetadata, bsHandle);
    }

    if (hVrExtMetaData->hasRenderInfo == 1) {
        RenderInfoHandle hRenderInfo = &hVrExtMetaData->renderInfo;
        Avs3VrExtRenderInfoEnc(hRenderInfo, fMetadata, bsHandle);
    }

    return 0;
}


static int16_t Avs3DirectSpeakersPositionMetadataEnc(
    DirectSpeakersPositionHandle hDirectSpeakersPositionMeta,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t tmpValue;

    // azimuth
    fread(&hDirectSpeakersPositionMeta->azimuth, sizeof(float), 1, fMetadata);
    tmpValue = (int16_t)floor(hDirectSpeakersPositionMeta->azimuth / RES_STATIC_AZIMUTH + 0.5f);
    tmpValue += (1 << (NBITS_DIRECTSPEAKER_AZI - 1));
    tmpValue = AVS3_MIN(tmpValue, ((1 << NBITS_DIRECTSPEAKER_AZI) - 1));
    PushNextIndice(bsHandle, tmpValue, NBITS_DIRECTSPEAKER_AZI);
    //fprintf(fori, "%f\n", hDirectSpeakersPositionMeta->azimuth);

    // elevation
    fread(&hDirectSpeakersPositionMeta->elevation, sizeof(float), 1, fMetadata);
    tmpValue = (int16_t)floor(hDirectSpeakersPositionMeta->elevation / RES_STATIC_ELEVATION + 0.5f);
    tmpValue += (1 << (NBITS_DIRECTSPEAKER_ELE - 1));
    tmpValue = AVS3_MIN(tmpValue, (1 << NBITS_DIRECTSPEAKER_ELE) - 1);
    PushNextIndice(bsHandle, tmpValue, NBITS_DIRECTSPEAKER_ELE);
    //fprintf(fori, "%f\n", hDirectSpeakersPositionMeta->elevation);

    // distance
    fread(&hDirectSpeakersPositionMeta->distance, sizeof(float), 1, fMetadata);
    tmpValue = (int16_t)floor(hDirectSpeakersPositionMeta->distance / RES_STATIC_DISTANCE + 0.5f);
    PushNextIndice(bsHandle, tmpValue, NBITS_DIRECTSPEAKER_DIS);
    //fprintf(fori, "%f\n", hDirectSpeakersPositionMeta->distance);

    // screenEdgeLock
    fread(&(hDirectSpeakersPositionMeta->screenEdgeLock), sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hDirectSpeakersPositionMeta->screenEdgeLock, NBITS_SCREEN_EDGELOCK);
    //fprintf(fori, "%d\n", hDirectSpeakersPositionMeta->screenEdgeLock);

    return 0;
}


static int16_t AVS3DialogueMetadataEnc(
    DialogueHandle hDialogueMeta,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    fread(&hDialogueMeta->dialogueAttribute, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hDialogueMeta->dialogueAttribute, NBITS_DIA_ATTRIBUTE);
    //fprintf(fori, "%d\n", hDialogueMeta->dialogueAttribute);

    fread(&hDialogueMeta->dialogueType, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hDialogueMeta->dialogueType, NBITS_DIA_TYPE);
    //fprintf(fori, "%d\n", hDialogueMeta->dialogueType);

    return 0;
}


static int16_t  AVS3AudioObjectInteractionMetadataEnc(
    AudioObjectInteractionHandle hAudioObjectInteractionMeta,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t tmpValue;

    // onOffInteract
    fread(&hAudioObjectInteractionMeta->onOffInteract, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioObjectInteractionMeta->onOffInteract, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioObjectInteractionMeta->onOffInteract);

    // gainInteract
    fread(&hAudioObjectInteractionMeta->gainInteract, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioObjectInteractionMeta->gainInteract, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioObjectInteractionMeta->gainInteract);

    // positionInteract
    fread(&hAudioObjectInteractionMeta->positionInteract, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioObjectInteractionMeta->positionInteract, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioObjectInteractionMeta->positionInteract);

    if (hAudioObjectInteractionMeta->gainInteract) {
        // gainUnit
        fread(&(hAudioObjectInteractionMeta->gainUnit), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioObjectInteractionMeta->gainUnit, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAudioObjectInteractionMeta->gainUnit);

        // linear quantization
        if (hAudioObjectInteractionMeta->gainUnit == 0) {
            // gainInteractionRange_Min
            fread(&(hAudioObjectInteractionMeta->gainInteractionRange_Min), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hAudioObjectInteractionMeta->gainInteractionRange_Min / RES_STATIC_RANGE_MIN + 0.5f);
            PushNextIndice(bsHandle, tmpValue, NBITS_GAIN_RANGEMIN);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->gainInteractionRange_Min);

            // gainInteractionRange_Max
            fread(&(hAudioObjectInteractionMeta->gainInteractionRange_Max), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor((hAudioObjectInteractionMeta->gainInteractionRange_Max - 1.0f) / RES_STATIC_RANGE_MAX + 0.5f);
            PushNextIndice(bsHandle, tmpValue, NBITS_GAIN_RANGEMAX);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->gainInteractionRange_Max);
        }
        else {
            // gainInteractionRange_Min
            fread(&(hAudioObjectInteractionMeta->gainInteractionRange_Min), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hAudioObjectInteractionMeta->gainInteractionRange_Min / RES_STATIC_RANGEDB_MIN + 0.5f);
            PushNextIndice(bsHandle, tmpValue, NBITS_GAIN_RANGEMIN);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->gainInteractionRange_Min);

            // gainInteractionRange_Max
            fread(&(hAudioObjectInteractionMeta->gainInteractionRange_Max), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor((hAudioObjectInteractionMeta->gainInteractionRange_Max) / RES_STATIC_RANGEDB_MAX + 0.5f);
            PushNextIndice(bsHandle, tmpValue, NBITS_GAIN_RANGEMAX);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->gainInteractionRange_Max);
        }
    }

    if (hAudioObjectInteractionMeta->positionInteract) {
        // cartesian
        fread(&(hAudioObjectInteractionMeta->cartesian), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioObjectInteractionMeta->cartesian, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAudioObjectInteractionMeta->cartesian);

        int16_t tmpQ;
        if (hAudioObjectInteractionMeta->cartesian) {
            // interactionRange_Xmin
            fread(&hAudioObjectInteractionMeta->interactionRange_Xmin, sizeof(float), 1, fMetadata);
            tmpQ = (int16_t)floor(hAudioObjectInteractionMeta->interactionRange_Xmin / RES_STATIC_X + 0.5f);
            tmpQ += (1 << (8 - 1));
            PushNextIndice(bsHandle, tmpQ, NBITS_INTERACT_RANGE_XMIN);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->interactionRange_Xmin);

            // interactionRange_Xmax
            fread(&hAudioObjectInteractionMeta->interactionRange_Xmax, sizeof(float), 1, fMetadata);
            tmpQ = (int16_t)floor(hAudioObjectInteractionMeta->interactionRange_Xmax / RES_STATIC_X + 0.5f);
            tmpQ += (1 << (8 - 1));
            PushNextIndice(bsHandle, tmpQ, NBITS_INTERACT_RANGE_XMAX);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->interactionRange_Xmax);

            // interactionRange_Ymin
            fread(&hAudioObjectInteractionMeta->interactionRange_Ymin, sizeof(float), 1, fMetadata);
            tmpQ = (int16_t)floor(hAudioObjectInteractionMeta->interactionRange_Ymin / RES_STATIC_Y + 0.5f);
            tmpQ += (1 << (6 - 1));
            PushNextIndice(bsHandle, tmpQ, NBITS_INTERACT_RANGE_YMIN);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->interactionRange_Ymin);

            // interactionRange_Ymax
            fread(&hAudioObjectInteractionMeta->interactionRange_Ymax, sizeof(float), 1, fMetadata);
            tmpQ = (int16_t)floor(hAudioObjectInteractionMeta->interactionRange_Ymax / RES_STATIC_Y + 0.5f);
            tmpQ += (1 << (6 - 1));
            PushNextIndice(bsHandle, tmpQ, NBITS_INTERACT_RANGE_YMAX);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->interactionRange_Ymax);

            // interactionRange_Zmin
            fread(&hAudioObjectInteractionMeta->interactionRange_Zmin, sizeof(float), 1, fMetadata);
            tmpQ = (int16_t)floor(hAudioObjectInteractionMeta->interactionRange_Zmin / RES_STATIC_Z + 0.5f);
            tmpQ += (1 << (4 - 1));
            PushNextIndice(bsHandle, tmpQ, NBITS_INTERACT_RANGE_ZMIN);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->interactionRange_Zmin);

            // interactionRange_Zmax
            fread(&hAudioObjectInteractionMeta->interactionRange_Zmax, sizeof(float), 1, fMetadata);
            tmpQ = (int16_t)floor(hAudioObjectInteractionMeta->interactionRange_Zmax / RES_STATIC_Z + 0.5f);
            tmpQ += (1 << (4 - 1));
            PushNextIndice(bsHandle, tmpQ, NBITS_INTERACT_RANGE_ZMAX);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->interactionRange_Zmax);
        }
        else {
            // polar
            // interactionRange_azimuthMin
            fread(&hAudioObjectInteractionMeta->interactionRange_azimuthMin, sizeof(float), 1, fMetadata);
            tmpQ = (int16_t)floor(hAudioObjectInteractionMeta->interactionRange_azimuthMin / RES_STATIC_AZIMUTHMIN + 0.5f);
            PushNextIndice(bsHandle, tmpQ, NBITS_INTERACT_RANGE_XMIN);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->interactionRange_azimuthMin);

            // interactionRange_azimuthMax
            fread(&hAudioObjectInteractionMeta->interactionRange_azimuthMax, sizeof(float), 1, fMetadata);
            tmpQ = (int16_t)floor(hAudioObjectInteractionMeta->interactionRange_azimuthMax / RES_STATIC_AZIMUTHMAX + 0.5f);
            PushNextIndice(bsHandle, tmpQ, NBITS_INTERACT_RANGE_XMAX);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->interactionRange_azimuthMax);

            // interactionRange_elevationMin
            fread(&hAudioObjectInteractionMeta->interactionRange_elevationMin, sizeof(float), 1, fMetadata);
            tmpQ = (int16_t)floor(hAudioObjectInteractionMeta->interactionRange_elevationMin / RES_STATIC_ELEVATIONMIN + 0.5f);
            PushNextIndice(bsHandle, tmpQ, NBITS_INTERACT_RANGE_YMIN);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->interactionRange_elevationMin);

            // interactionRange_elevationMax
            fread(&hAudioObjectInteractionMeta->interactionRange_elevationMax, sizeof(float), 1, fMetadata);
            tmpQ = (int16_t)floor(hAudioObjectInteractionMeta->interactionRange_elevationMax / RES_STATIC_ELEVATIONMAX + 0.5f);
            PushNextIndice(bsHandle, tmpQ, NBITS_INTERACT_RANGE_YMAX);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->interactionRange_elevationMax);

            // interactionRange_distanceMin
            fread(&hAudioObjectInteractionMeta->interactionRange_distanceMin, sizeof(float), 1, fMetadata);
            tmpQ = (int16_t)floor(hAudioObjectInteractionMeta->interactionRange_distanceMin / RES_STATIC_DISTANCE + 0.5f);
            PushNextIndice(bsHandle, tmpQ, NBITS_INTERACT_RANGE_ZMIN);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->interactionRange_distanceMin);

            // interactionRange_distanceMax
            fread(&hAudioObjectInteractionMeta->interactionRange_distanceMax, sizeof(float), 1, fMetadata);
            tmpQ = (int16_t)floor(hAudioObjectInteractionMeta->interactionRange_distanceMax / RES_STATIC_DISTANCE + 0.5f);
            PushNextIndice(bsHandle, tmpQ, NBITS_INTERACT_RANGE_ZMAX);
            //fprintf(fori, "%f\n", hAudioObjectInteractionMeta->interactionRange_distanceMax);

        }
    }

    return 0;
}


static int16_t Avs3AudioProgrammeRefScreenMetadataEnc(AudioProgrammeRefScreenHandle hAudioProgrammeRefScreenMeta, FILE *fMetadata, AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t tmpValue;

    // hasCartesian
    fread(&hAudioProgrammeRefScreenMeta->hasCartesian, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioProgrammeRefScreenMeta->hasCartesian, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioProgrammeRefScreenMeta->hasCartesian);

    // aspectRatio
    fread(&hAudioProgrammeRefScreenMeta->aspectRatio, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioProgrammeRefScreenMeta->aspectRatio, NBITS_ASPECT_RATIO);
    //fprintf(fori, "%d\n", hAudioProgrammeRefScreenMeta->aspectRatio);

    if (hAudioProgrammeRefScreenMeta->hasCartesian == 0) {
        // Position_azimuth
        fread(&hAudioProgrammeRefScreenMeta->Position_azimuth, sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hAudioProgrammeRefScreenMeta->Position_azimuth / RES_STATIC_AZIMUTH + 0.5f);
        tmpValue += (1 << (NBITS_SCREEN_AZIMUTH - 1));
        tmpValue = AVS3_MIN(tmpValue, ((1 << NBITS_SCREEN_AZIMUTH) - 1));
        PushNextIndice(bsHandle, tmpValue, NBITS_SCREEN_AZIMUTH);
        //fprintf(fori, "%f\n", hAudioProgrammeRefScreenMeta->Position_azimuth);

        // Position_elevation
        fread(&hAudioProgrammeRefScreenMeta->Position_elevation, sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hAudioProgrammeRefScreenMeta->Position_elevation / RES_STATIC_SCREEELEVATION + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_SCREEN_ELEVATION);
        //fprintf(fori, "%f\n", hAudioProgrammeRefScreenMeta->Position_elevation);

        // Position_distance
        fread(&hAudioProgrammeRefScreenMeta->Position_distance, sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hAudioProgrammeRefScreenMeta->Position_distance / RES_STATIC_DISTANCE + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_SCREEN_DISTANCE);
        //fprintf(fori, "%f\n", hAudioProgrammeRefScreenMeta->Position_distance);

        // polarScreenWidth
        fread(&hAudioProgrammeRefScreenMeta->polarScreenWidth, sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hAudioProgrammeRefScreenMeta->polarScreenWidth / RES_STATIC_POLARWIDTH + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_SCREEN_POLAR);
        //fprintf(fori, "%f\n", hAudioProgrammeRefScreenMeta->polarScreenWidth);
    }
    else {
        // Position_X
        fread(&hAudioProgrammeRefScreenMeta->Position_X, sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hAudioProgrammeRefScreenMeta->Position_X / RES_STATIC_X + 0.5f);
        tmpValue += (1 << (NBITS_SCREEN_X - 1));
        PushNextIndice(bsHandle, tmpValue, NBITS_SCREEN_X);
        //fprintf(fori, "%f\n", hAudioProgrammeRefScreenMeta->Position_X);

        // Position_Y
        fread(&hAudioProgrammeRefScreenMeta->Position_Y, sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hAudioProgrammeRefScreenMeta->Position_Y / RES_STATIC_Y + 0.5f);
        tmpValue += (1 << (NBITS_SCREEN_Y - 1));
        PushNextIndice(bsHandle, tmpValue, NBITS_SCREEN_Y);
        //fprintf(fori, "%f\n", hAudioProgrammeRefScreenMeta->Position_Y);

        // Position_Z
        fread(&hAudioProgrammeRefScreenMeta->Position_Z, sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hAudioProgrammeRefScreenMeta->Position_Z / RES_STATIC_Z + 0.5f);
        tmpValue += (1 << (NBITS_SCREEN_Z - 1));
        PushNextIndice(bsHandle, tmpValue, NBITS_SCREEN_Z);
        //fprintf(fori, "%f\n", hAudioProgrammeRefScreenMeta->Position_Z);

        // CartesianScreenWidth
        fread(&hAudioProgrammeRefScreenMeta->CartesianScreenWidth, sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hAudioProgrammeRefScreenMeta->CartesianScreenWidth / RES_STATIC_CARTWIDTH + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_SCREEN_CARTESIAN);
        //fprintf(fori, "%f\n", hAudioProgrammeRefScreenMeta->CartesianScreenWidth);
    }

    return 0;
}


static int16_t Avs3LoudnessMetadataEnc(
    LoudnessMetadataHandle hLoudnessMeta,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t tmpValue;

    // hasIntegratedLoudness
    fread(&hLoudnessMeta->hasIntegratedLoudness, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hLoudnessMeta->hasIntegratedLoudness, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hLoudnessMeta->hasIntegratedLoudness);

    // hasLoudnessRange
    fread(&hLoudnessMeta->hasLoudnessRange, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hLoudnessMeta->hasLoudnessRange, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hLoudnessMeta->hasLoudnessRange);

    // hasMaxTruePeak
    fread(&hLoudnessMeta->hasMaxTruePeak, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hLoudnessMeta->hasMaxTruePeak, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hLoudnessMeta->hasMaxTruePeak);

    // hasMaxMomentary
    fread(&hLoudnessMeta->hasMaxMomentary, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hLoudnessMeta->hasMaxMomentary, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hLoudnessMeta->hasMaxMomentary);

    // hasMaxShortTerm
    fread(&hLoudnessMeta->hasMaxShortTerm, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hLoudnessMeta->hasMaxShortTerm, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hLoudnessMeta->hasMaxShortTerm);

    // hasDialogueLoudness
    fread(&hLoudnessMeta->hasDialogueLoudness, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hLoudnessMeta->hasDialogueLoudness, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hLoudnessMeta->hasDialogueLoudness);

    if (hLoudnessMeta->hasIntegratedLoudness) {
        // integratedLoudness
        fread(&(hLoudnessMeta->integratedLoudness), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hLoudnessMeta->integratedLoudness / RES_STATIC_INTEGRATEDLOUDNESS + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_LOUDNESS);
        //fprintf(fori, "%f\n", hLoudnessMeta->integratedLoudness);
    }

    if (hLoudnessMeta->hasLoudnessRange) {
        // loudnessRange
        fread(&(hLoudnessMeta->loudnessRange), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor((hLoudnessMeta->loudnessRange - 10.0f) / RES_STATIC_LOUDNESSRANGE + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_LOUDNESS_RANGE);
        //fprintf(fori, "%f\n", hLoudnessMeta->loudnessRange);
    }

    if (hLoudnessMeta->hasMaxTruePeak) {
        // maxTruePeak
        fread(&(hLoudnessMeta->maxTruePeak), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hLoudnessMeta->maxTruePeak / RES_STATIC_MAXTRUEPEAK + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_TURE_PEAK);
        //fprintf(fori, "%f\n", hLoudnessMeta->maxTruePeak);
    }

    if (hLoudnessMeta->hasMaxMomentary) {
        // maxMomentary
        fread(&(hLoudnessMeta->maxMomentary), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hLoudnessMeta->maxMomentary / RES_STATIC_MAXMOMENTARY + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_MAX_MOMNETARY);
        //fprintf(fori, "%f\n", hLoudnessMeta->maxMomentary);
    }

    if (hLoudnessMeta->hasMaxShortTerm) {
        // maxShortTerm
        fread(&(hLoudnessMeta->maxShortTerm), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hLoudnessMeta->maxShortTerm / RES_STATIC_MAXSHORTTERM + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_SHORT_TERM);
        //fprintf(fori, "%f\n", hLoudnessMeta->maxShortTerm);
    }

    if (hLoudnessMeta->hasDialogueLoudness) {
        // dialogueLoudness
        fread(&(hLoudnessMeta->dialogueLoudness), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hLoudnessMeta->dialogueLoudness / RES_STATIC_DIALOGUELOUDNESS + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_DL_LOUNDESS);
        //fprintf(fori, "%f\n", hLoudnessMeta->dialogueLoudness);
    }

    return 0;
}


static int16_t Avs3AudioChannelFormatMetadataEnc(
    AudioChannelFormatHandle hAudioChannelFormatMeta,
    int16_t pack2ChannelRef[][3],
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t i, idx;
    int16_t tmpValue;

    // channelFormatIdx
    fread(&(hAudioChannelFormatMeta->channelFormatIdx), sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioChannelFormatMeta->channelFormatIdx, NBITS_CH_FORMAT_IDX);
    idx = hAudioChannelFormatMeta->channelFormatIdx;
    //fprintf(fori, "%d\n", hAudioChannelFormatMeta->channelFormatIdx);

    // hasChannelGain
    fread(&(hAudioChannelFormatMeta->hasChannelGain), sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioChannelFormatMeta->hasChannelGain, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioChannelFormatMeta->hasChannelGain);

    // hasChannelGain
    if (hAudioChannelFormatMeta->hasChannelGain) {
        // gainUnit
        fread(&(hAudioChannelFormatMeta->gainUnit), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioChannelFormatMeta->gainUnit, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAudioChannelFormatMeta->gainUnit);

        // linear
        if (hAudioChannelFormatMeta->gainUnit == 0) {
            // channelGain
            fread(&(hAudioChannelFormatMeta->channelGain), sizeof(float), 1, fMetadata);
            int16_t halfFlag;
            if (hAudioChannelFormatMeta->channelGain <= 1.0f) {
                halfFlag = 0;
                PushNextIndice(bsHandle, halfFlag, NBITS_META_FLAG);
                tmpValue = (int16_t)floor(hAudioChannelFormatMeta->channelGain / RES_STATIC_OBJ_CHGAIN_H1 + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_HALF_CHGAIN);
                //fprintf(fori, "%f\n", hAudioChannelFormatMeta->channelGain);
            }
            else {
                halfFlag = 1;
                PushNextIndice(bsHandle, halfFlag, NBITS_META_FLAG);
                tmpValue = (int16_t)floor((hAudioChannelFormatMeta->channelGain - 1.0f) / RES_STATIC_OBJ_CHGAIN_H2 + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_HALF_CHGAIN);
                //fprintf(fori, "%f\n", hAudioChannelFormatMeta->channelGain);
            }
        }
        else {
            // channelGain
            fread(&(hAudioChannelFormatMeta->channelGain), sizeof(float), 1, fMetadata);
            int16_t halfFlag;
            if (hAudioChannelFormatMeta->channelGain <= 0.0f) {
                halfFlag = 0;
                PushNextIndice(bsHandle, halfFlag, NBITS_META_FLAG);
                tmpValue = (int16_t)floor(hAudioChannelFormatMeta->channelGain / RES_STATIC_OBJ_CHGAIN_DB_H1 + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_HALF_CHGAIN);
                //fprintf(fori, "%f\n", hAudioChannelFormatMeta->channelGain);
            }
            else {
                halfFlag = 1;
                PushNextIndice(bsHandle, halfFlag, NBITS_META_FLAG);
                tmpValue = (int16_t)floor((hAudioChannelFormatMeta->channelGain) / RES_STATIC_OBJ_CHGAIN_DB_H2 + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_HALF_CHGAIN);
                //fprintf(fori, "%f\n", hAudioChannelFormatMeta->channelGain);
            }

        }
    }

    if (pack2ChannelRef[idx][0] == 1) {
        if (pack2ChannelRef[idx][1] == 63) {
            DirectSpeakersPositionHandle hDirectSpeakersPosition = &hAudioChannelFormatMeta->directSpeakersPositionData;
            Avs3DirectSpeakersPositionMetadataEnc(hDirectSpeakersPosition, fMetadata, bsHandle);
        }
    }

    if (pack2ChannelRef[idx][0] == 2) {
        for (i = 0; i < pack2ChannelRef[idx][2]; i++) {
            fread(&hAudioChannelFormatMeta->MatrixCoef[i], sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor((hAudioChannelFormatMeta->MatrixCoef[i] - 0.1f) / RES_MTRIXCOEF + 0.5f);
            PushNextIndice(bsHandle, tmpValue, NBITS_MATRIX_COEF);
            //fprintf(fori, "%f\n", hAudioChannelFormatMeta->MatrixCoef[i]);
        }
    }

    return 0;
}


static int16_t Avs3AudioPackFormatMetadataEnc(
    AudioPackFormatHandle hAudioPackFormatMeta,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle,
    int16_t pack2ChannelRef[][3])
{
    int16_t i;
    int16_t tmpValue;

    // packFormatIdx
    fread(&hAudioPackFormatMeta->packFormatIdx, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioPackFormatMeta->packFormatIdx, NBITS_PACKS_IDX);
    //fprintf(fori, "%d\n", hAudioPackFormatMeta->packFormatIdx);

    // hasImportance
    fread(&hAudioPackFormatMeta->hasImportance, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioPackFormatMeta->hasImportance, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioPackFormatMeta->hasImportance);

    // hasChannelReuse
    fread(&hAudioPackFormatMeta->hasChannelReuse, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioPackFormatMeta->hasChannelReuse, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioPackFormatMeta->hasChannelReuse);

    if (hAudioPackFormatMeta->hasImportance) {
        // importance
        fread(&(hAudioPackFormatMeta->importance), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioPackFormatMeta->importance, NBITS_PACK_IMPORTANCE);
        //fprintf(fori, "%d\n", hAudioPackFormatMeta->importance);
    }

    // typeLabel
    fread(&(hAudioPackFormatMeta->typeLabel), sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioPackFormatMeta->typeLabel, NBITS_PACK_LABEL);
    //fprintf(fori, "%d\n", hAudioPackFormatMeta->typeLabel);

    // absoluteDistance
    fread(&(hAudioPackFormatMeta->absoluteDistance), sizeof(float), 1, fMetadata);
    // convert to dB
    float tmp;
    tmp = log10f(hAudioPackFormatMeta->absoluteDistance + 1.0f);
    tmpValue = (int16_t)floor(tmp / RES_STATIC_ABSDISTANCE + 0.5f);
    PushNextIndice(bsHandle, tmpValue, NBITS_PACK_ABSDIS);
    //fprintf(fori, "%f\n", hAudioPackFormatMeta->absoluteDistance);

    if (hAudioPackFormatMeta->typeLabel == 4) {
        // normalization
        fread(&(hAudioPackFormatMeta->normalization), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioPackFormatMeta->normalization, NBITS_NORMALIZATION);
        //fprintf(fori, "%d\n", hAudioPackFormatMeta->normalization);

        // nfcRefDist
        fread(&(hAudioPackFormatMeta->nfcRefDist), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hAudioPackFormatMeta->nfcRefDist / RES_STATIC_NFCREFDIST + 0.5f);
        PushNextIndice(bsHandle, tmpValue, NBITS_NCFRDFDIST);
        //fprintf(fori, "%f\n", hAudioPackFormatMeta->nfcRefDist);

        // screenRef
        fread(&hAudioPackFormatMeta->screenRef, sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioPackFormatMeta->screenRef, NBITS_SCREENREF);
        //fprintf(fori, "%d\n", hAudioPackFormatMeta->screenRef);

        // hoaOrder
        fread(&hAudioPackFormatMeta->hoaOrder, sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioPackFormatMeta->hoaOrder, NBITS_HOAORDER);
        //fprintf(fori, "%d\n", hAudioPackFormatMeta->hoaOrder);
    }

    if (hAudioPackFormatMeta->typeLabel == 2 || hAudioPackFormatMeta->typeLabel == 1) {
        // packFormatID
        fread(&(hAudioPackFormatMeta->packFormatID), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioPackFormatMeta->packFormatID, NBITS_PACKFORMATID);
        //fprintf(fori, "%d\n", hAudioPackFormatMeta->packFormatID);

        if (hAudioPackFormatMeta->typeLabel == 2) {
            // numMatrixOutputChannel
            fread(&(hAudioPackFormatMeta->numMatrixOutputChannel), sizeof(int16_t), 1, fMetadata);
            tmpValue = hAudioPackFormatMeta->numMatrixOutputChannel - 1;
            PushNextIndice(bsHandle, tmpValue, NBITS_NUM_MATRIXCH);
            //fprintf(fori, "%d\n", hAudioPackFormatMeta->numMatrixOutputChannel);

            for (i = 0; i < hAudioPackFormatMeta->numMatrixOutputChannel; i++) {
                DirectSpeakersPositionHandle hDirectSpeakersPosition = &(hAudioPackFormatMeta->directSpeakersPositionData[i]);
                Avs3DirectSpeakersPositionMetadataEnc(hDirectSpeakersPosition, fMetadata, bsHandle);
            }
        }
    }

    if (!hAudioPackFormatMeta->hasChannelReuse) {
        // packFormatStartIdx
        fread(&(hAudioPackFormatMeta->packFormatStartIdx), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioPackFormatMeta->packFormatStartIdx, NBITS_PACKFORMAT_IDX);
        //fprintf(fori, "%d\n", hAudioPackFormatMeta->packFormatStartIdx);
    }

    // numChannels
    fread(&(hAudioPackFormatMeta->numChannels), sizeof(int16_t), 1, fMetadata);
    tmpValue = hAudioPackFormatMeta->numChannels - 1;
    PushNextIndice(bsHandle, tmpValue, NBITS_NUM_CHS);
    //fprintf(fori, "%d\n", hAudioPackFormatMeta->numChannels);

    for (i = 0; i < hAudioPackFormatMeta->numChannels; i++) {
        // refChannelIdx
        fread(&(hAudioPackFormatMeta->refChannelIdx[i]), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioPackFormatMeta->refChannelIdx[i], NBITS_REF_CH_IDX);
        //fprintf(fori, "%d\n", hAudioPackFormatMeta->refChannelIdx[i]);

        int16_t idx = hAudioPackFormatMeta->refChannelIdx[i];

        pack2ChannelRef[idx][0] = hAudioPackFormatMeta->typeLabel;

        if (hAudioPackFormatMeta->typeLabel == 1 || hAudioPackFormatMeta->typeLabel == 2) {
            pack2ChannelRef[idx][1] = hAudioPackFormatMeta->packFormatID;
        }

        if (hAudioPackFormatMeta->typeLabel == 2) {
            pack2ChannelRef[idx][2] = hAudioPackFormatMeta->numMatrixOutputChannel;
        }

        if (hAudioPackFormatMeta->hasChannelReuse) {
            // transChRef
            fread(&(hAudioPackFormatMeta->transChRef[i]), sizeof(int16_t), 1, fMetadata);
            PushNextIndice(bsHandle, hAudioPackFormatMeta->transChRef[i], NBITS_TRANS_CH_REF);
            //fprintf(fori, "%d\n", hAudioPackFormatMeta->transChRef[i]);
        }
    }

    return 0;
}


static int16_t Avs3AudioObjectMetadataEnc(
    AudioObjectHandle hAudioObjectMeta,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t i;
    int16_t tmpValue;

    // objectIdx
    fread(&hAudioObjectMeta->objectIdx, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioObjectMeta->objectIdx, NBITS_OBJ_IDX);
    //fprintf(fori, "%d\n", hAudioObjectMeta->objectIdx);

    // hasAudioObjectLanguage
    fread(&hAudioObjectMeta->hasAudioObjectLanguage, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioObjectMeta->hasAudioObjectLanguage, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioObjectMeta->hasAudioObjectLanguage);

    // hasDialogue
    fread(&hAudioObjectMeta->hasDialogue, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioObjectMeta->hasDialogue, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioObjectMeta->hasDialogue);

    // hasImportance
    fread(&hAudioObjectMeta->hasImportance, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioObjectMeta->hasImportance, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioObjectMeta->hasImportance);

    // hasDisableDucking
    fread(&hAudioObjectMeta->hasDisableDucking, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioObjectMeta->hasDisableDucking, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioObjectMeta->hasDisableDucking);

    // hasInteract
    fread(&hAudioObjectMeta->hasInteract, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioObjectMeta->hasInteract, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioObjectMeta->hasInteract);

    // hasGain
    fread(&hAudioObjectMeta->hasGain, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioObjectMeta->hasGain, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioObjectMeta->hasGain);

    // hasHeadLocked
    fread(&hAudioObjectMeta->hasHeadLocked, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioObjectMeta->hasHeadLocked, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioObjectMeta->hasHeadLocked);

    // hasMute
    fread(&hAudioObjectMeta->hasMute, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioObjectMeta->hasMute, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioObjectMeta->hasMute);

    if (hAudioObjectMeta->hasAudioObjectLanguage) {
        fread(&(hAudioObjectMeta->audioObjectLanguage), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioObjectMeta->audioObjectLanguage, NBITS_OBJ_LANGUAGE);
        //fprintf(fori, "%d\n", hAudioObjectMeta->audioObjectLanguage);
    }

    if (hAudioObjectMeta->hasDialogue) {
        DialogueHandle hDialogue = &hAudioObjectMeta->dialogueData;
        AVS3DialogueMetadataEnc(hDialogue, fMetadata, bsHandle);
    }

    if (hAudioObjectMeta->hasImportance) {
        fread(&(hAudioObjectMeta->importance), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioObjectMeta->importance, NBITS_OBJ_IMPORTANCE);
        //fprintf(fori, "%d\n", hAudioObjectMeta->importance);
    }

    if (hAudioObjectMeta->hasInteract) {
        for (i = 0; i < 24; i++) {
            fread(&(hAudioObjectMeta->ObjectName[i]), sizeof(int16_t), 1, fMetadata);
            PushNextIndice(bsHandle, hAudioObjectMeta->ObjectName[i], NBITS_OBJ_NAME);
            //fprintf(fori, "%d\n", hAudioObjectMeta->ObjectName[i]);
        }

        AudioObjectInteractionHandle hAudioObjectInteraction = &hAudioObjectMeta->audioObjectInteractionData;
        AVS3AudioObjectInteractionMetadataEnc(hAudioObjectInteraction, fMetadata, bsHandle);
    }

    if (hAudioObjectMeta->hasGain) {
        fread(&(hAudioObjectMeta->gainUnit), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioObjectMeta->gainUnit, NBITS_OBJ_GAINUNIT);
        //fprintf(fori, "%d\n", hAudioObjectMeta->gainUnit);

        fread(&(hAudioObjectMeta->gain), sizeof(float), 1, fMetadata);
        int16_t halfFlag;
        // linear
        if (hAudioObjectMeta->gainUnit == 0) {
            if (hAudioObjectMeta->gain <= 1.0f) {
                halfFlag = 0;
                PushNextIndice(bsHandle, halfFlag, NBITS_META_FLAG);
                tmpValue = (int16_t)floor(hAudioObjectMeta->gain / RES_STATIC_OBJ_GAIN_H1 + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_HALFGAIN);
                //fprintf(fori, "%f\n", hAudioObjectMeta->gain);
            }
            else {
                halfFlag = 1;
                PushNextIndice(bsHandle, halfFlag, NBITS_META_FLAG);
                tmpValue = (int16_t)floor((hAudioObjectMeta->gain - 1.0f) / RES_STATIC_OBJ_GAIN_H2 + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_HALFGAIN);
                //fprintf(fori, "%f\n", hAudioObjectMeta->gain);
            }
        }
        else {
            if (hAudioObjectMeta->gain <= 0.0f) {
                halfFlag = 0;
                PushNextIndice(bsHandle, halfFlag, NBITS_META_FLAG);
                tmpValue = (int16_t)floor(hAudioObjectMeta->gain / RES_STATIC_OBJ_GAIN_DB_H1 + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_HALFGAIN);
                //fprintf(fori, "%f\n", hAudioObjectMeta->gain);
            }
            else {
                halfFlag = 1;
                PushNextIndice(bsHandle, halfFlag, NBITS_META_FLAG);
                tmpValue = (int16_t)floor(hAudioObjectMeta->gain / RES_STATIC_OBJ_GAIN_DB_H2 + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_HALFGAIN);
                //fprintf(fori, "%f\n", hAudioObjectMeta->gain);
            }
        }
    }

    // numPacks
    fread(&(hAudioObjectMeta->numPacks), sizeof(int16_t), 1, fMetadata);
    tmpValue = hAudioObjectMeta->numPacks - 1;
    PushNextIndice(bsHandle, tmpValue, NBITS_NUM_PACKS);
    //fprintf(fori, "%d\n", hAudioObjectMeta->numPacks);

    for (i = 0; i < hAudioObjectMeta->numPacks; i++) {
        // refPackFormatIdx
        fread(&(hAudioObjectMeta->refPackFormatIdx[i]), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioObjectMeta->refPackFormatIdx[i], NBITS_REF_PACKS_IDX);
        //fprintf(fori, "%d\n", hAudioObjectMeta->refPackFormatIdx[i]);
    }

    return 0;
}


static int16_t Avs3AudioContentMetadataEnc(
    AudioContentHandle hAudioContentMeta,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t i, j;
    int16_t tmpValue;

    // contentIdx
    fread(&hAudioContentMeta->contentIdx, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioContentMeta->contentIdx, NBITS_CON_IDX);
    //fprintf(fori, "%d\n", hAudioContentMeta->contentIdx);

    // hasAudioContentLanguage
    fread(&hAudioContentMeta->hasAudioContentLanguage, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioContentMeta->hasAudioContentLanguage, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioContentMeta->hasAudioContentLanguage);

    // hasLoudnessMetadata
    fread(&hAudioContentMeta->hasLoudnessMetadata, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioContentMeta->hasLoudnessMetadata, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioContentMeta->hasLoudnessMetadata);

    // hasDialogue
    fread(&hAudioContentMeta->hasDialogue, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioContentMeta->hasDialogue, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioContentMeta->hasDialogue);

    // hasNumComplementaryObjectGroup
    fread(&hAudioContentMeta->hasNumComplementaryObjectGroup, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioContentMeta->hasNumComplementaryObjectGroup, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioContentMeta->hasNumComplementaryObjectGroup);

    // audioContentLanguage
    if (hAudioContentMeta->hasAudioContentLanguage) {
        fread(&(hAudioContentMeta->audioContentLanguage), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioContentMeta->audioContentLanguage, NBITS_CONTENT_LANGUAGE);
        //fprintf(fori, "%d\n", hAudioContentMeta->audioContentLanguage);
    }

    if (hAudioContentMeta->hasLoudnessMetadata) {
        LoudnessMetadataHandle hLoudnessMetadata = &hAudioContentMeta->loudnessData;
        Avs3LoudnessMetadataEnc(hLoudnessMetadata, fMetadata, bsHandle);
    }

    if (hAudioContentMeta->hasDialogue) {
        DialogueHandle hDialogue = &hAudioContentMeta->dialogueData;
        AVS3DialogueMetadataEnc(hDialogue, fMetadata, bsHandle);
    }

    if (hAudioContentMeta->hasNumComplementaryObjectGroup) {
        // numComplementaryObjectGroup
        fread(&(hAudioContentMeta->numComplementaryObjectGroup), sizeof(int16_t), 1, fMetadata);
        tmpValue = hAudioContentMeta->numComplementaryObjectGroup - 1;
        PushNextIndice(bsHandle, tmpValue, NBITS_NUM_GROUPS);
        //fprintf(fori, "%d\n", hAudioContentMeta->numComplementaryObjectGroup);

        for (i = 0; i < hAudioContentMeta->numComplementaryObjectGroup; i++) {
            // numComplementaryObject
            fread(&(hAudioContentMeta->numComplementaryObject[i]), sizeof(int16_t), 1, fMetadata);
            tmpValue = hAudioContentMeta->numComplementaryObject[i] - 1;
            PushNextIndice(bsHandle, tmpValue, NBITS_NUM_COM_OBJS);
            //fprintf(fori, "%d\n", hAudioContentMeta->numComplementaryObject[i]);

            for (j = 0; j < hAudioContentMeta->numComplementaryObject[i]; j++) {
                // ComplementaryObjectIdx
                fread(&(hAudioContentMeta->ComplementaryObjectIdx[i][j]), sizeof(int16_t), 1, fMetadata);
                PushNextIndice(bsHandle, hAudioContentMeta->ComplementaryObjectIdx[i][j], NBITS_COM_OBJ_IDX);
                //fprintf(fori, "%d\n", hAudioContentMeta->ComplementaryObjectIdx[i][j]);
            }
        }
    }

    // numObjects
    fread(&(hAudioContentMeta->numObjects), sizeof(int16_t), 1, fMetadata);
    tmpValue = hAudioContentMeta->numObjects - 1;
    PushNextIndice(bsHandle, tmpValue, NBITS_NUM_OBJECTS);
    //fprintf(fori, "%d\n", hAudioContentMeta->numObjects);

    for (i = 0; i < hAudioContentMeta->numObjects; i++) {
        // refObjectIdx
        fread(&(hAudioContentMeta->refObjectIdx[i]), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioContentMeta->refObjectIdx[i], NBITS_REF_OBJ_IDX);
        //fprintf(fori, "%d\n", hAudioContentMeta->refObjectIdx[i]);
    }

    return 0;
}


static int16_t Avs3AudioProgrammeEnc(
    AudioProgrammeHandle hAudioProgrammeMeta,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t tmpValue;

    // hasAudioProgrammeLanguage
    fread(&hAudioProgrammeMeta->hasAudioProgrammeLanguage, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioProgrammeMeta->hasAudioProgrammeLanguage, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioProgrammeMeta->hasAudioProgrammeLanguage);

    // hasMaxDuckingDepth
    fread(&hAudioProgrammeMeta->hasMaxDuckingDepth, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioProgrammeMeta->hasMaxDuckingDepth, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioProgrammeMeta->hasMaxDuckingDepth);

    // hasLoudnessMetadata
    fread(&hAudioProgrammeMeta->hasLoudnessMetadata, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioProgrammeMeta->hasLoudnessMetadata, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioProgrammeMeta->hasLoudnessMetadata);

    // hasAudioProgrammeRefScreen
    fread(&hAudioProgrammeMeta->hasAudioProgrammeRefScreen, sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAudioProgrammeMeta->hasAudioProgrammeRefScreen, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioProgrammeMeta->hasAudioProgrammeRefScreen);

    if (hAudioProgrammeMeta->hasAudioProgrammeLanguage) {
        // audioProgrammeLanguage
        fread(&(hAudioProgrammeMeta->audioProgrammeLanguage), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioProgrammeMeta->audioProgrammeLanguage, NBITS_AUDIO_PL);
        //fprintf(fori, "%d\n", hAudioProgrammeMeta->audioProgrammeLanguage);
    }

    if (hAudioProgrammeMeta->hasMaxDuckingDepth) {
        // maxDuckingDepth
        fread(&(hAudioProgrammeMeta->maxDuckingDepth), sizeof(float), 1, fMetadata);
        tmpValue = (int16_t)floor(hAudioProgrammeMeta->maxDuckingDepth / RES_DUCKINGDEPTH + 0.5f);
        tmpValue = AVS3_MIN(AVS3_MAX(tmpValue, 0), (1 << NBITS_MAX_DUCK) - 1);
        PushNextIndice(bsHandle, tmpValue, NBITS_MAX_DUCK);
        //fprintf(fori, "%f\n", hAudioProgrammeMeta->maxDuckingDepth);
    }

    // LoudnessMetadata
    if (hAudioProgrammeMeta->hasLoudnessMetadata) {
        LoudnessMetadataHandle hLoudnessMetadata = &hAudioProgrammeMeta->loudnessData;
        Avs3LoudnessMetadataEnc(hLoudnessMetadata, fMetadata, bsHandle);
    }

    // AudioProgrammeRefScreen
    if (hAudioProgrammeMeta->hasAudioProgrammeRefScreen) {
        AudioProgrammeRefScreenHandle  hAudioProgrammeRefScreen = &hAudioProgrammeMeta->audioProgrammeRefScreenData;
        Avs3AudioProgrammeRefScreenMetadataEnc(hAudioProgrammeRefScreen, fMetadata, bsHandle);
    }

    // numContents
    fread(&(hAudioProgrammeMeta->numContents), sizeof(int16_t), 1, fMetadata);
    tmpValue = hAudioProgrammeMeta->numContents - 1;
    PushNextIndice(bsHandle, tmpValue, NBITS_NUMCONTENTS);
    //fprintf(fori, "%d\n", hAudioProgrammeMeta->numContents);

    for (int16_t i = 0; i < hAudioProgrammeMeta->numContents; i++) {
        // refContentIdx
        fread(&hAudioProgrammeMeta->refContentIdx[i], sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAudioProgrammeMeta->refContentIdx[i], NBITS_REF_CON_IDX);
        //fprintf(fori, "%d\n", hAudioProgrammeMeta->refContentIdx[i]);
    }

    return 0;
}


static int16_t Avs3BasicL1Enc(
    Avs3BasicL1Handle hAvs3BasicL1,
    AVS3EncoderHandle stAvs3,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t i;
    int16_t numUsedIndices;
    uint8_t* bitstream = stAvs3->bitstream;

    int16_t tmpValue;
    AudioProgrammeHandle hAudioProgrammeMeta = &hAvs3BasicL1->audioProgrammeMeta;
    Avs3AudioProgrammeEnc(hAudioProgrammeMeta, fMetadata, bsHandle);

    // numOfContents
    fread(&(hAvs3BasicL1->numOfContents), sizeof(int16_t), 1, fMetadata);
    tmpValue = hAvs3BasicL1->numOfContents - 1;
    PushNextIndice(bsHandle, tmpValue, NBITS_NUMCONTENTS);
    //fprintf(fori, "%d\n", hAvs3BasicL1->numOfContents);

    for (i = 0; i < hAvs3BasicL1->numOfContents; i++) {
        AudioContentHandle hAudioContent = &hAvs3BasicL1->audioContentData[i];
        Avs3AudioContentMetadataEnc(hAudioContent, fMetadata, bsHandle);
    }

    // count used indices
    numUsedIndices = 0;
    for (i = 0; i < MAX_NUM_INDICES; i++) {
        if (bsHandle->indiceList[i].nBits != -1) {
            numUsedIndices++;
        }
    }
    // transform to serial by once
    IndicesToSerial(bsHandle->indiceList, numUsedIndices, bitstream, &stAvs3->totalSideBits);
    // reset bitstream buffer
    ResetIndicesEnc(bsHandle, MAX_NUM_INDICES);

    // numOfObjects
    fread(&(hAvs3BasicL1->numOfObjects), sizeof(int16_t), 1, fMetadata);
    tmpValue = hAvs3BasicL1->numOfObjects - 1;
    PushNextIndice(bsHandle, tmpValue, NBITS_NUM_OBJECTS);
    //fprintf(fori, "%d\n", hAvs3BasicL1->numOfObjects);

    for (i = 0; i < hAvs3BasicL1->numOfObjects; i++) {
        AudioObjectHandle hAudioObject = &hAvs3BasicL1->audioObjectData[i];
        Avs3AudioObjectMetadataEnc(hAudioObject, fMetadata, bsHandle);
    }

    // count used indices
    numUsedIndices = 0;
    for (i = 0; i < MAX_NUM_INDICES; i++) {
        if (bsHandle->indiceList[i].nBits != -1) {
            numUsedIndices++;
        }
    }
    // transform to serial by once
    IndicesToSerial(bsHandle->indiceList, numUsedIndices, bitstream, &stAvs3->totalSideBits);
    // reset bitstream buffer
    ResetIndicesEnc(bsHandle, MAX_NUM_INDICES);

    // numOfPacks
    fread(&(hAvs3BasicL1->numOfPacks), sizeof(int16_t), 1, fMetadata);
    tmpValue = hAvs3BasicL1->numOfPacks - 1;
    PushNextIndice(bsHandle, tmpValue, NBITS_NUM_PACKS);
    //fprintf(fori, "%d\n", hAvs3BasicL1->numOfPacks);

    int16_t pack2ChannelRef[32][3];
    for (i = 0; i < hAvs3BasicL1->numOfPacks; i++) {
        AudioPackFormatHandle hAudioPackFormat = &hAvs3BasicL1->audioPackFormatData[i];
        Avs3AudioPackFormatMetadataEnc(hAudioPackFormat, fMetadata, bsHandle, pack2ChannelRef);
    }

    // count used indices
    numUsedIndices = 0;
    for (i = 0; i < MAX_NUM_INDICES; i++) {
        if (bsHandle->indiceList[i].nBits != -1) {
            numUsedIndices++;
        }
    }
    // transform to serial by once
    IndicesToSerial(bsHandle->indiceList, numUsedIndices, bitstream, &stAvs3->totalSideBits);
    // reset bitstream buffer
    ResetIndicesEnc(bsHandle, MAX_NUM_INDICES);

    // numOfChannels
    fread(&(hAvs3BasicL1->numOfChannels), sizeof(int16_t), 1, fMetadata);
    tmpValue = hAvs3BasicL1->numOfChannels - 1;
    PushNextIndice(bsHandle, tmpValue, NBITS_NUM_CHS);
    //fprintf(fori, "%d\n", hAvs3BasicL1->numOfChannels);

    for (i = 0; i < hAvs3BasicL1->numOfChannels; i++) {
        AudioChannelFormatHandle hAudioChannelFormat = &(hAvs3BasicL1->audioChannelFormatData[i]);
        Avs3AudioChannelFormatMetadataEnc(hAudioChannelFormat, pack2ChannelRef, fMetadata, bsHandle);
    }

    // count used indices
    numUsedIndices = 0;
    for (i = 0; i < MAX_NUM_INDICES; i++) {
        if (bsHandle->indiceList[i].nBits != -1) {
            numUsedIndices++;
        }
    }
    // transform to serial by once
    IndicesToSerial(bsHandle->indiceList, numUsedIndices, bitstream, &stAvs3->totalSideBits);
    // reset bitstream buffer
    ResetIndicesEnc(bsHandle, MAX_NUM_INDICES);

    return 0;
}


static int16_t Avs3MetaDataStaticEnc(
    Avs3MetaDataStaticHandle hStaticMetaData,
    AVS3EncoderHandle stAvs3,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    int16_t i;
    int16_t numUsedIndices;
    uint8_t* bitstream = stAvs3->bitstream;

    // hasVrExt
    fread(&(hStaticMetaData->hasVrExt), sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hStaticMetaData->hasVrExt, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hStaticMetaData->hasVrExt);

    // basicLevel
    fread(&(hStaticMetaData->basicLevel), sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hStaticMetaData->basicLevel, NBITS_BASICLEVEL);
    //fprintf(fori, "%d\n", hStaticMetaData->basicLevel);

    // count used indices
    numUsedIndices = 0;
    for (i = 0; i < MAX_NUM_INDICES; i++) {
        if (bsHandle->indiceList[i].nBits != -1) {
            numUsedIndices++;
        }
    }
    // transform to serial by once
    IndicesToSerial(bsHandle->indiceList, numUsedIndices, bitstream, &stAvs3->totalSideBits);
    // reset bitstream buffer
    ResetIndicesEnc(bsHandle, MAX_NUM_INDICES);

    // static meta encoding, level 1
    if (hStaticMetaData->basicLevel == 0) {
        Avs3BasicL1Handle hAvs3BasicL1 = &hStaticMetaData->avs3BasicL1;
        Avs3BasicL1Enc(hAvs3BasicL1, stAvs3, fMetadata, bsHandle);
    }

    if (hStaticMetaData->hasVrExt) {
        // vrExtLevel
        fread(&(hStaticMetaData->vrExtLevel), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hStaticMetaData->vrExtLevel, NBITS_VREXTLEVEL);
        //fprintf(fori, "%d\n", hStaticMetaData->vrExtLevel);

        if (hStaticMetaData->vrExtLevel == 0) {
            Avs3VrExtL1MetaDataHandle hAvs3VrExtL1MetaData = &hStaticMetaData->avs3VrExtL1MetaData;
            Avs3VrExtMetaDataEnc(hAvs3VrExtL1MetaData, fMetadata, bsHandle);
        }

        numUsedIndices = 0;
        for (i = 0; i < MAX_NUM_INDICES; i++) {
            if (bsHandle->indiceList[i].nBits != -1) {
                numUsedIndices++;
            }
        }
        // transform to serial by once
        IndicesToSerial(bsHandle->indiceList, numUsedIndices, bitstream, &stAvs3->totalSideBits);
        // reset bitstream buffer
        ResetIndicesEnc(bsHandle, MAX_NUM_INDICES);
    }

    return 0;
}


// Dynamic meta encoding, level 2
static int16_t Avs3DmL2MetaDataEnc(
    Avs3DmL2MetaDataHandle hAvs3DmL2MetaData,
    int16_t muteFlag,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    if (muteFlag == 0) {
        int16_t tmpValue;

        // hasChannelLock
        fread(&(hAvs3DmL2MetaData->hasChannelLock), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAvs3DmL2MetaData->hasChannelLock, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL2MetaData->hasChannelLock);

        if (hAvs3DmL2MetaData->hasChannelLock == 1) {
            // channelLock
            fread(&(hAvs3DmL2MetaData->channelLock), sizeof(int16_t), 1, fMetadata);
            PushNextIndice(bsHandle, hAvs3DmL2MetaData->channelLock, NBITS_CHANNEL_LOCK);
            //fprintf(fori, "%d\n", hAvs3DmL2MetaData->channelLock);

            if (hAvs3DmL2MetaData->channelLock == 1) {
                // channelLockMaxDist
                fread(&(hAvs3DmL2MetaData->channelLockMaxDist), sizeof(float), 1, fMetadata);
                tmpValue = (int16_t)floor(hAvs3DmL2MetaData->channelLockMaxDist / RES_CHANNELLOCK_MAXDIST + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_CHANNELLOCK_MAXDIST);
                //fprintf(fori, "%f\n", hAvs3DmL2MetaData->channelLockMaxDist);
            }
        }

        // hasObjectDivergence
        fread(&(hAvs3DmL2MetaData->hasObjectDivergence), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAvs3DmL2MetaData->hasObjectDivergence, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL2MetaData->hasObjectDivergence);

        if (hAvs3DmL2MetaData->hasObjectDivergence == 1) {
            // objDivergence
            fread(&(hAvs3DmL2MetaData->objDivergence), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hAvs3DmL2MetaData->objDivergence / RES_OBJ_DIVERGENCE + 0.5f);
            PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_DIVERGENCE);
            //fprintf(fori, "%f\n", hAvs3DmL2MetaData->objDivergence);

            if (hAvs3DmL2MetaData->objDivergence != 0.0f) {
                // objDiverAzimuthRange
                fread(&(hAvs3DmL2MetaData->objDiverAzimuthRange), sizeof(float), 1, fMetadata);
                if (tmpValue != 0) {
                    tmpValue = (int16_t)floor(hAvs3DmL2MetaData->objDiverAzimuthRange / RES_OBJ_DIVERGENCE_AZI_RANGE + 0.5f);
                    PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_DIVERGENCE_AZI_RANGE);
                    //fprintf(fori, "%f\n", hAvs3DmL2MetaData->objDiverAzimuthRange);
                }
            }
        }

        // hasObjectScreenRef
        fread(&(hAvs3DmL2MetaData->hasObjectScreenRef), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAvs3DmL2MetaData->hasObjectScreenRef, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL2MetaData->hasObjectScreenRef);

        if (hAvs3DmL2MetaData->hasObjectScreenRef == 1) {
            // objScreenRef
            fread(&(hAvs3DmL2MetaData->objScreenRef), sizeof(int16_t), 1, fMetadata);
            PushNextIndice(bsHandle, hAvs3DmL2MetaData->objScreenRef, NBITS_OBJ_SCREEN_REF);
            //fprintf(fori, "%d\n", hAvs3DmL2MetaData->objScreenRef);
        }

        // hasScreenEdgeLock
        fread(&(hAvs3DmL2MetaData->hasScreenEdgeLock), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAvs3DmL2MetaData->hasScreenEdgeLock, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL2MetaData->hasScreenEdgeLock);

        if (hAvs3DmL2MetaData->hasScreenEdgeLock == 1) {
            // screenEdgeLock
            fread(&(hAvs3DmL2MetaData->screenEdgeLock), sizeof(int16_t), 1, fMetadata);
            PushNextIndice(bsHandle, hAvs3DmL2MetaData->screenEdgeLock, NBITS_OBJ_SCREEN_EDGE_LOCK);
            //fprintf(fori, "%d\n", hAvs3DmL2MetaData->screenEdgeLock);
        }

    }

    return 0;
}


static int16_t Avs3DmL1MetaDataEnc(
    Avs3DmL1MetaDataHandle hAvs3DmL1MetaData,
    int16_t muteFlag,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    if (muteFlag == 0) {
        // cartesian
        fread(&(hAvs3DmL1MetaData->cartesian), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAvs3DmL1MetaData->cartesian, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL1MetaData->cartesian);

        // hasObjExtent
        fread(&(hAvs3DmL1MetaData->hasObjExtent), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAvs3DmL1MetaData->hasObjExtent, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL1MetaData->hasObjExtent);

        // hasObjGain
        fread(&(hAvs3DmL1MetaData->hasObjGain), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAvs3DmL1MetaData->hasObjGain, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL1MetaData->hasObjGain);

        // hasObjDiffuse
        fread(&(hAvs3DmL1MetaData->hasObjDiffuse), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAvs3DmL1MetaData->hasObjDiffuse, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL1MetaData->hasObjDiffuse);

        // hasObjImportance
        fread(&(hAvs3DmL1MetaData->hasObjImportance), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAvs3DmL1MetaData->hasObjImportance, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL1MetaData->hasObjImportance);

        int16_t tmpValue;
        if (hAvs3DmL1MetaData->cartesian == 0) {
            // objAzimuth
            fread(&(hAvs3DmL1MetaData->objAzimuth), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hAvs3DmL1MetaData->objAzimuth / RES_OBJ_AZIMUTH + 0.5f);
            tmpValue += (1 << (NBITS_OBJ_AZIMUTH - 1));
            tmpValue = AVS3_MIN(tmpValue, (1 << NBITS_OBJ_AZIMUTH) - 1);
            PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_AZIMUTH);
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objAzimuth);

            // objElevation
            fread(&(hAvs3DmL1MetaData->objElevation), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hAvs3DmL1MetaData->objElevation / RES_OBJ_ELEVATION + 0.5f);
            tmpValue += (1 << (NBITS_OBJ_ELEVATION - 1));
            tmpValue = AVS3_MIN(tmpValue, (1 << NBITS_OBJ_ELEVATION) - 1);
            PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_ELEVATION);
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objElevation);

            // objDistance
            fread(&(hAvs3DmL1MetaData->objDistance), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hAvs3DmL1MetaData->objDistance / RES_OBJ_DISTANCE + 0.5f);
            PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_DISTANCE);
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objDistance);

            if (hAvs3DmL1MetaData->hasObjExtent) {
                // objWidth
                fread(&(hAvs3DmL1MetaData->objWidth), sizeof(float), 1, fMetadata);
                tmpValue = (int16_t)floor(hAvs3DmL1MetaData->objWidth / RES_OBJ_WIDTH + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_WIDTH);
                //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objWidth);

                // objHeight
                fread(&(hAvs3DmL1MetaData->objHeight), sizeof(float), 1, fMetadata);
                tmpValue = (int16_t)floor(hAvs3DmL1MetaData->objHeight / RES_OBJ_HEIGHT + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_HEIGHT);
                //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objHeight);

                // objDepth
                fread(&(hAvs3DmL1MetaData->objDepth), sizeof(float), 1, fMetadata);
                tmpValue = (int16_t)floor(hAvs3DmL1MetaData->objDepth / RES_OBJ_DEPTH + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_DEPTH);
                //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objDepth);
            }
        }
        else {
            // obj_x
            fread(&(hAvs3DmL1MetaData->obj_x), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hAvs3DmL1MetaData->obj_x / RES_OBJ_X + 0.5f);
            tmpValue += (1 << (NBITS_OBJ_X - 1));
            PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_X);
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->obj_x);

            // obj_y
            fread(&(hAvs3DmL1MetaData->obj_y), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hAvs3DmL1MetaData->obj_y / RES_OBJ_Y + 0.5f);
            tmpValue += (1 << (NBITS_OBJ_Y - 1));
            PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_Y);
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->obj_y);

            // obj_z
            fread(&(hAvs3DmL1MetaData->obj_z), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hAvs3DmL1MetaData->obj_z / RES_OBJ_Z + 0.5f);
            tmpValue += (1 << (NBITS_OBJ_Z - 1));
            PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_Z);
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->obj_z);

            if (hAvs3DmL1MetaData->hasObjExtent) {
                // objWidth_x
                fread(&(hAvs3DmL1MetaData->objWidth_x), sizeof(float), 1, fMetadata);
                tmpValue = (int16_t)floor(hAvs3DmL1MetaData->objWidth_x / RES_OBJ_WIDTH_X + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_WIDTH_X);
                //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objWidth_x);

                // objHeight_y
                fread(&(hAvs3DmL1MetaData->objHeight_y), sizeof(float), 1, fMetadata);
                tmpValue = (int16_t)floor(hAvs3DmL1MetaData->objHeight_y / RES_OBJ_HEIGHT_Y + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_HEIGHT_Y);
                //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objHeight_y);

                // objDepth_z
                fread(&(hAvs3DmL1MetaData->objDepth_z), sizeof(float), 1, fMetadata);
                tmpValue = (int16_t)floor(hAvs3DmL1MetaData->objDepth_z / RES_OBJ_DEPTH_Z + 0.5f);
                PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_DEPTH_Z);
                //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objDepth_z);
            }
        }

        // gain
        if (hAvs3DmL1MetaData->hasObjGain) {
            fread(&(hAvs3DmL1MetaData->gain), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hAvs3DmL1MetaData->gain / RES_OBJ_GAIN + 0.5f);
            PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_GAIN);
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->gain);
        }

        // diffuse
        if (hAvs3DmL1MetaData->hasObjDiffuse) {
            fread(&(hAvs3DmL1MetaData->diffuse), sizeof(float), 1, fMetadata);
            tmpValue = (int16_t)floor(hAvs3DmL1MetaData->diffuse / RES_OBJ_DIFFUSE + 0.5f);
            PushNextIndice(bsHandle, tmpValue, NBITS_OBJ_DIFFUSE);
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->diffuse);
        }

        // jumpPosition
        fread(&(hAvs3DmL1MetaData->jumpPosition), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAvs3DmL1MetaData->jumpPosition, NBITS_JUMPPOSITION);
        //fprintf(fori, "%d\n", hAvs3DmL1MetaData->jumpPosition);

        // importance
        if (hAvs3DmL1MetaData->hasObjImportance) {
            fread(&(hAvs3DmL1MetaData->importance), sizeof(int16_t), 1, fMetadata);
            PushNextIndice(bsHandle, hAvs3DmL1MetaData->importance, NBITS_IMPORTANCE);
            //fprintf(fori, "%d\n", hAvs3DmL1MetaData->importance);
        }
    }

    return 0;
}


static int16_t Avs3MetaDataDynamicEnc(
    Avs3MetaDataDynamicHandle hAvs3MetaDataDynamic,
    AVS3EncoderHandle stAvs3,
    FILE *fMetadata,
    AVS3_BSTREAM_ENC_HANDLE bsHandle)
{
    short i;
    int16_t numUsedIndices;
    uint8_t* bitstream = stAvs3->bitstream;

    // dmLevel
    fread(&(hAvs3MetaDataDynamic->dmLevel), sizeof(int16_t), 1, fMetadata);
    PushNextIndice(bsHandle, hAvs3MetaDataDynamic->dmLevel, NBITS_DMLEVEL);
    //fprintf(fori, "%d\n", hAvs3MetaDataDynamic->dmLevel);

    // Dynamic num chs equals to number input objs
    hAvs3MetaDataDynamic->numDmChans = stAvs3->numObjsInput;

    for (int16_t i = 0; i < hAvs3MetaDataDynamic->numDmChans; i++) {
        // muteFlag
        fread(&(hAvs3MetaDataDynamic->muteFlag[i]), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAvs3MetaDataDynamic->muteFlag[i], NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3MetaDataDynamic->muteFlag[i]);

        // transChRef
        fread(&(hAvs3MetaDataDynamic->transChRef[i]), sizeof(int16_t), 1, fMetadata);
        PushNextIndice(bsHandle, hAvs3MetaDataDynamic->transChRef[i], NBITS_DM_TRANSCHREF);
        //fprintf(fori, "%d\n", hAvs3MetaDataDynamic->transChRef[i]);

        if (hAvs3MetaDataDynamic->dmLevel == 0) {
            Avs3DmL1MetaDataHandle hAvs3DmL1MetaData = &(hAvs3MetaDataDynamic->avs3DmL1MetaData[i]);
            Avs3DmL1MetaDataEnc(hAvs3DmL1MetaData, hAvs3MetaDataDynamic->muteFlag[i], fMetadata, bsHandle);
        }

        if (hAvs3MetaDataDynamic->dmLevel == 1) {
            Avs3DmL1MetaDataHandle hAvs3DmL1MetaData = &(hAvs3MetaDataDynamic->avs3DmL1MetaData[i]);
            Avs3DmL1MetaDataEnc(hAvs3DmL1MetaData, hAvs3MetaDataDynamic->muteFlag[i], fMetadata, bsHandle);

            Avs3DmL2MetaDataHandle hAvs3DmL2MetaData = &(hAvs3MetaDataDynamic->avs3DmL2MetaData[i]);
            Avs3DmL2MetaDataEnc(hAvs3DmL2MetaData, hAvs3MetaDataDynamic->muteFlag[i], fMetadata, bsHandle);
        }
    }

    // count used indices
    numUsedIndices = 0;
    for (i = 0; i < MAX_NUM_INDICES; i++) {
        if (bsHandle->indiceList[i].nBits != -1) {
            numUsedIndices++;
        }
    }
    // transform to serial by once
    IndicesToSerial(bsHandle->indiceList, numUsedIndices, bitstream, &stAvs3->totalSideBits);
    // reset bitstream buffer
    ResetIndicesEnc(bsHandle, MAX_NUM_INDICES);

    return 0;
}


void Avs3MetadataEnc(AVS3EncoderHandle stAvs3, FILE* fMetadata)
{
    Avs3MetaDataHandle hAvs3MetaData = &(stAvs3->hAvs3MetaData->avs3MetaData);

    AVS3_BSTREAM_ENC_HANDLE bsHandle = stAvs3->hAvs3MetaData->bsHandle;

    short i;
    int16_t numUsedIndices;
    uint8_t* bitstream = stAvs3->bitstream;

    // hasStaticMeta
    if (fMetadata == NULL) {
        hAvs3MetaData->hasStaticMeta = 0;
    } else {
        fread(&(hAvs3MetaData->hasStaticMeta), sizeof(int16_t), 1, fMetadata);
    }
    PushNextIndice(bsHandle, hAvs3MetaData->hasStaticMeta, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAvs3MetaData->hasStaticMeta);

    // count used indices
    numUsedIndices = 0;
    for (i = 0; i < MAX_NUM_INDICES; i++) {
        if (bsHandle->indiceList[i].nBits != -1) {
            numUsedIndices++;
        }
    }
    // transform to serial by once
    IndicesToSerial(bsHandle->indiceList, numUsedIndices, bitstream, &stAvs3->totalSideBits);
    // reset bitstream buffer
    ResetIndicesEnc(bsHandle, MAX_NUM_INDICES);

    if (hAvs3MetaData->hasStaticMeta) {
        Avs3MetaDataStaticHandle hAvs3MetaDataStatic = &(hAvs3MetaData->avs3MetaDataStatic);
        Avs3MetaDataStaticEnc(hAvs3MetaDataStatic, stAvs3, fMetadata, bsHandle);
    }

    // hasDynamicMeta
    if (fMetadata == NULL) {
        hAvs3MetaData->hasDynamicMeta = 0;
    }
    else {
        fread(&(hAvs3MetaData->hasDynamicMeta), sizeof(int16_t), 1, fMetadata);
    }
    PushNextIndice(bsHandle, hAvs3MetaData->hasDynamicMeta, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAvs3MetaData->hasDynamicMeta);

    // count used indices
    numUsedIndices = 0;
    for (i = 0; i < MAX_NUM_INDICES; i++) {
        if (bsHandle->indiceList[i].nBits != -1) {
            numUsedIndices++;
        }
    }
    // transform to serial by once
    IndicesToSerial(bsHandle->indiceList, numUsedIndices, bitstream, &stAvs3->totalSideBits);
    // reset bitstream buffer
    ResetIndicesEnc(bsHandle, MAX_NUM_INDICES);

    if (hAvs3MetaData->hasDynamicMeta) {
        Avs3MetaDataDynamicHandle hAvs3MetaDataDynamic = &hAvs3MetaData->avs3MetaDataDynamic;
        Avs3MetaDataDynamicEnc(hAvs3MetaDataDynamic, stAvs3, fMetadata, bsHandle);
    }

    return;
}
