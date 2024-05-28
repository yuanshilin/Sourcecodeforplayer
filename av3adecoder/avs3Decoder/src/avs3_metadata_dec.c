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
#include "avs3_stat_com.h"
#include "avs3_stat_dec.h"
#include "avs3_prot_com.h"
#include "avs3_prot_dec.h"
#include "avs3_stat_meta.h"
#include "avs3_const_meta.h"

extern FILE *fori;

int16_t Avs3VrExtEqEffectDec(
    EqEffectHandle hEqEffect,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    hEqEffect->eqType = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_EQTYPE);
    //fprintf(fori, "%d\n", hEqEffect->eqType);

    int16_t tmpValue;
    float tmp;

    tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_EQFC);
    tmp = tmpValue * RES_VREXT_EQFC;
    hEqEffect->eqFc = powf(10, ((tmp + 20 * log10f(20.0f)) / 20));
    //fprintf(fori, "%f\n", hEqEffect->eqFc);

    int16_t halfFlag;
    halfFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);

    if (halfFlag == 0) {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_EQQ);
        hEqEffect->eqQ = tmpValue * RES_VREXT_EQQ_H1 + 0.1f;
        //fprintf(fori, "%f\n", hEqEffect->eqQ);
    }
    else {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_EQQ);
        hEqEffect->eqQ = tmpValue * RES_VREXT_EQQ_H2 + 1.0f;
        //fprintf(fori, "%f\n", hEqEffect->eqQ);
    }

    tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_EQGAIN);
    tmpValue -= (1 << (NBITS_VR_EQGAIN - 1));
    hEqEffect->eqGain = tmpValue * RES_VREXT_EQGAIN;
    //fprintf(fori, "%f\n", hEqEffect->eqGain);

    return 0;
}


int16_t Avs3VrExtAudioEffectDec(
    AudioEffectHandle hAudioEffect,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    hAudioEffect->hasEQExist = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioEffect->hasEQExist);

    hAudioEffect->hasDRCExist = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioEffect->hasDRCExist);

    hAudioEffect->hasGainExist = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioEffect->hasGainExist);

    if (hAudioEffect->hasEQExist || hAudioEffect->hasDRCExist || hAudioEffect->hasGainExist) {
        hAudioEffect->effectChain = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_EFFCHAIN);
        hAudioEffect->effectChain = AVS3_MIN(hAudioEffect->effectChain, 5);
        //fprintf(fori, "%d\n", hAudioEffect->effectChain);
    }

    if (hAudioEffect->hasEQExist) {
        hAudioEffect->numEqband = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_NUMEQBAND);
        hAudioEffect->numEqband = AVS3_MIN(hAudioEffect->numEqband, 10);
        hAudioEffect->numEqband += 1;
        //fprintf(fori, "%d\n", hAudioEffect->numEqband);

        for (int16_t i = 0; i < hAudioEffect->numEqband; i++) {
            EqEffectHandle hEqEffect = &hAudioEffect->eqEffect[i];
            Avs3VrExtEqEffectDec(hEqEffect, hBitstream);
        }
    }

    int16_t tmpValue;

    if (hAudioEffect->hasDRCExist) {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_ATT_TIME);
        hAudioEffect->attackTime = tmpValue * RES_VREXT_ATT_TIME + 1.0f;
        //fprintf(fori, "%f\n",hAudioEffect->attackTime);

        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_REL_TIME);
        hAudioEffect->releaseTime = tmpValue * RES_VREXT_REL_TIME + 50.0f;
        //fprintf(fori, "%f\n",hAudioEffect->releaseTime);

        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_THRESHOLD);
        hAudioEffect->threshold= tmpValue * RES_VREXT_THRESHOLD - 80.0f;
        //fprintf(fori, "%f\n",hAudioEffect->threshold);

        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_PREGAIN);
        hAudioEffect->preGain = (tmpValue - (1<<(NBITS_VR_PREGAIN -1)))* RES_VREXT_PREGAIN;
        //fprintf(fori, "%f\n",hAudioEffect->preGain);

        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_POSTGAIN);
        hAudioEffect->postGain = tmpValue * RES_VREXT_POSTGAIN;
        //fprintf(fori, "%f\n",hAudioEffect->postGain);

        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_RATIO);
        hAudioEffect->ratio = tmpValue * RES_VREXT_RATIO + 1.0f;
        //fprintf(fori, "%f\n",hAudioEffect->ratio);
    }

    if (hAudioEffect->hasGainExist) {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_EFFGAIN);
        hAudioEffect->effectGain = (tmpValue - (1 << (NBITS_VR_EFFGAIN - 1)))* RES_VREXT_EFFGAIN;
        //fprintf(fori, "%f\n",hAudioEffect->effectGain);
    }

    return 0;
}


int16_t Avs3VrExtVertexDec(
    VertexHandle hVertex,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t tmpValue;

    tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_VER_X);
    tmpValue -= (1 << (NBITS_VR_VER_X - 1));
    hVertex->x = tmpValue * RES_VREXT_VERTEX_X;
    //fprintf(fori, "%f\n", hVertex->x);

    tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_VER_Y);
    tmpValue -= (1 << (NBITS_VR_VER_Y - 1));
    hVertex->y = tmpValue * RES_VREXT_VERTEX_Y;
    //fprintf(fori, "%f\n", hVertex->y);

    tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_VER_Z);
    tmpValue -= (1 << (NBITS_VR_VER_Z - 1));
    hVertex->z = tmpValue * RES_VREXT_VERTEX_Z;
    //fprintf(fori, "%f\n", hVertex->z);
    
    return 0;
}


int16_t Avs3VrExtSurfaceDec(
    SurfaceHandle hSurface,
    int16_t numSurface,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    hSurface->material = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_MARTERIAL);
    //fprintf(fori, "%d\n", hSurface->material);

    if (hSurface->material == 31) {
        int16_t tmpValue;
        for (int16_t i = 0; i < 8; i++) {
            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_ABSORPTION);
            hSurface->absorption[i] = tmpValue * RES_VREXT_ABSORPTION;
            //fprintf(fori, "%f\n", hSurface->absorption[i]);

            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_SCATTERING);
            hSurface->scattering[i] = tmpValue * RES_VREXT_SCATTERING;
            //fprintf(fori, "%f\n", hSurface->scattering[i]);
        }
    }

    hSurface->numVertices = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_NUM_VER);
    hSurface->numVertices += 1;
    hSurface->numVertices = AVS3_MIN(AVS3_MAX(hSurface->numVertices, (int16_t)ceil(8.0f / numSurface)), (int16_t)floor(36.0f / numSurface));
    //fprintf(fori, "%d\n", hSurface->numVertices);

    for (int16_t i = 0; i < hSurface->numVertices; i++) {
        VertexHandle hVertex = &hSurface->vertex[i];
        Avs3VrExtVertexDec(hVertex, hBitstream);
    }
    
    return 0;
}


int16_t Avs3VrExtRenderInfoDec(
    RenderInfoHandle hRenderInfo,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    hRenderInfo->targetDevice = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hRenderInfo->targetDevice);

    hRenderInfo->hrtfType = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_HRTFTYPE);
    //fprintf(fori, "%d\n", hRenderInfo->hrtfType);

    for (int16_t i = 0; i < 16; i++) {
        hRenderInfo->headphoneType[i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_HPTYPE);
        //fprintf(fori, "%d\n", hRenderInfo->headphoneType[i]);
    }

    AudioEffectHandle hAudioEffect = &hRenderInfo->audioEffect;
    Avs3VrExtAudioEffectDec(hAudioEffect, hBitstream);

    return 0;
}


int16_t Avs3VrExtAcousticEnvDec(
    AcousticEnvHandle hAcousticEnv,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t tmpValue;

    hAcousticEnv->hasEarlyReflectionGain = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAcousticEnv->hasEarlyReflectionGain);

    hAcousticEnv->hasLateReverbGain = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAcousticEnv->hasLateReverbGain);

    hAcousticEnv->reverbType = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_REVERBTYPE);
    //fprintf(fori, "%d\n", hAcousticEnv->reverbType);

    if (hAcousticEnv->hasEarlyReflectionGain == 1) {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_EARLY_REFGAIN);
        hAcousticEnv->earlyReflectionGain = tmpValue * RES_VREXT_EAR_REF_GAIN;
        //fprintf(fori, "%f\n", hAcousticEnv->earlyReflectionGain);
    }

    if (hAcousticEnv->hasLateReverbGain == 1) {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_LATE_REVGAIN);
        hAcousticEnv->lateReverbGain = tmpValue * RES_VREXT_LATE_REV_GAIN;
        //fprintf(fori, "%f\n", hAcousticEnv->lateReverbGain);
    }

    hAcousticEnv->lowFreqProFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAcousticEnv->lowFreqProFlag);

    if (hAcousticEnv->reverbType == 2) {
        hAcousticEnv->convolutionReverbType = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_CONV_REVTYPE);
        //fprintf(fori, "%d\n", hAcousticEnv->convolutionReverbType);
    }

    hAcousticEnv->numSurface = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_NUMSURFACE);
    hAcousticEnv->numSurface += 1;
    //fprintf(fori, "%d\n", hAcousticEnv->numSurface);

    for (int16_t i = 0; i < hAcousticEnv->numSurface; i++) {
        SurfaceHandle hSurface = &hAcousticEnv->surface[i];
        Avs3VrExtSurfaceDec(hSurface, hAcousticEnv->numSurface, hBitstream);
    }
    
    return 0;
}


int16_t Avs3VrExtDec(
    Avs3VrExtL1MetaDataHandle hVrExtMetaData,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    hVrExtMetaData->hasAcousticEnv = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hVrExtMetaData->hasAcousticEnv);

    hVrExtMetaData->hasRenderInfo = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hVrExtMetaData->hasRenderInfo);

    hVrExtMetaData->ambisonicOrder = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VR_AMBIS_ORDER);
    //fprintf(fori, "%d\n", hVrExtMetaData->ambisonicOrder);

    if (hVrExtMetaData->hasAcousticEnv == 1) {
        AcousticEnvHandle hAcousticEnv = &hVrExtMetaData->acousticEnv;
        Avs3VrExtAcousticEnvDec(hAcousticEnv, hBitstream);
    }

    if (hVrExtMetaData->hasRenderInfo == 1) {
        RenderInfoHandle hRenderInfo = &hVrExtMetaData->renderInfo;
        Avs3VrExtRenderInfoDec(hRenderInfo, hBitstream);
    }

    return 0;
}


static int16_t Avs3DirectSpeakersPositionDec(
    DirectSpeakersPositionHandle hDirectSpeakersPosition,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t tmp;
    float tmpValue;

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_DIRECTSPEAKER_AZI);
    tmpValue = (tmp - (1 << (NBITS_DIRECTSPEAKER_AZI - 1))) * RES_STATIC_AZIMUTH;
    hDirectSpeakersPosition->azimuth = tmpValue;

    //fprintf(fori, "%f\n", tmpValue);

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_DIRECTSPEAKER_ELE);
    tmpValue = (tmp - (1 << (NBITS_DIRECTSPEAKER_ELE - 1))) * RES_STATIC_ELEVATION;
    hDirectSpeakersPosition->elevation = tmpValue;

    //fprintf(fori, "%f\n", tmpValue);

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_DIRECTSPEAKER_DIS);
    tmpValue = (tmp) * RES_STATIC_DISTANCE;
    hDirectSpeakersPosition->distance = tmpValue;
    //fprintf(fori, "%f\n", tmpValue);

    hDirectSpeakersPosition->screenEdgeLock = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SCREEN_EDGELOCK);
    //fprintf(fori, "%d\n", hDirectSpeakersPosition->screenEdgeLock);

    return 0;
}


static int16_t Avs3DialogueDec(
    DialogueHandle hDialogue,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t tmpValue;

    tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_DIA_ATTRIBUTE);
    hDialogue->dialogueAttribute = tmpValue;
    //fprintf(fori, "%d\n", tmpValue);

    tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_DIA_TYPE);
    hDialogue->dialogueType = tmpValue;
    //fprintf(fori, "%d\n", tmpValue);
    return 0;
}


static int16_t Avs3AudioObjectInteractionDec(
    AudioObjectInteractionHandle hAudioObjectInteraction,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t  tmpFlag;
    int16_t tmpValue;

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioObjectInteraction->onOffInteract = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioObjectInteraction->gainInteract = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioObjectInteraction->positionInteract = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    if (hAudioObjectInteraction->gainInteract) {
        hAudioObjectInteraction->gainUnit = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAudioObjectInteraction->gainUnit);

        // linear
        if (hAudioObjectInteraction->gainUnit == 0) {
            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_GAIN_RANGEMIN);
            hAudioObjectInteraction->gainInteractionRange_Min = tmpValue * RES_STATIC_RANGE_MIN;
            //fprintf(fori, "%f\n", hAudioObjectInteraction->gainInteractionRange_Min);

            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_GAIN_RANGEMAX);
            hAudioObjectInteraction->gainInteractionRange_Max = tmpValue * RES_STATIC_RANGE_MAX + 1.0f;
            //fprintf(fori, "%f\n", hAudioObjectInteraction->gainInteractionRange_Max);
        }
        else {
            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_GAIN_RANGEMIN);
            hAudioObjectInteraction->gainInteractionRange_Min = tmpValue * RES_STATIC_RANGEDB_MIN;
            //fprintf(fori, "%f\n", hAudioObjectInteraction->gainInteractionRange_Min);

            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_GAIN_RANGEMAX);
            hAudioObjectInteraction->gainInteractionRange_Max = tmpValue * RES_STATIC_RANGEDB_MAX;
            //fprintf(fori, "%f\n", hAudioObjectInteraction->gainInteractionRange_Max);
        }
    }

    if (hAudioObjectInteraction->positionInteract) {
        hAudioObjectInteraction->cartesian = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAudioObjectInteraction->cartesian);

        int16_t tmpdata;
        float tmpQ;
        if (hAudioObjectInteraction->cartesian) {
            tmpdata = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_INTERACT_RANGE_XMIN);
            tmpQ = (tmpdata - (1 << (NBITS_INTERACT_RANGE_XMIN - 1))) * RES_STATIC_X;
            hAudioObjectInteraction->interactionRange_Xmin = tmpQ;
            //fprintf(fori, "%f\n", tmpQ);

            tmpdata = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_INTERACT_RANGE_XMAX);
            tmpQ = (tmpdata - (1 << (NBITS_INTERACT_RANGE_XMAX - 1))) * RES_STATIC_X;
            hAudioObjectInteraction->interactionRange_Xmax = tmpQ;
            //fprintf(fori, "%f\n", tmpQ);

            tmpdata = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_INTERACT_RANGE_YMIN);
            tmpQ = (tmpdata - (1 << (NBITS_INTERACT_RANGE_YMIN - 1))) * RES_STATIC_Y;
            hAudioObjectInteraction->interactionRange_Ymin = tmpQ;
            //fprintf(fori, "%f\n", tmpQ);

            tmpdata = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_INTERACT_RANGE_YMAX);
            tmpQ = (tmpdata - (1 << (NBITS_INTERACT_RANGE_YMAX - 1))) * RES_STATIC_Y;
            hAudioObjectInteraction->interactionRange_Ymax = tmpQ;
            //fprintf(fori, "%f\n", tmpQ);

            tmpdata = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_INTERACT_RANGE_ZMIN);
            tmpQ = (tmpdata - (1 << (NBITS_INTERACT_RANGE_ZMIN - 1))) * RES_STATIC_Z;
            hAudioObjectInteraction->interactionRange_Zmin = tmpQ;
            //fprintf(fori, "%f\n", tmpQ);

            tmpdata = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_INTERACT_RANGE_ZMAX);
            tmpQ = (tmpdata - (1 << (NBITS_INTERACT_RANGE_ZMAX - 1))) * RES_STATIC_Z;
            hAudioObjectInteraction->interactionRange_Zmax = tmpQ;
            //fprintf(fori, "%f\n", tmpQ);
        }
        else {
            //polar
            tmpdata = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_INTERACT_RANGE_XMIN);
            tmpQ = (tmpdata) * RES_STATIC_AZIMUTHMIN;
            hAudioObjectInteraction->interactionRange_azimuthMin = tmpQ;
            //fprintf(fori, "%f\n", tmpQ);

            tmpdata = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_INTERACT_RANGE_XMAX);
            tmpQ = (tmpdata) * RES_STATIC_AZIMUTHMAX;
            hAudioObjectInteraction->interactionRange_azimuthMax = tmpQ;
            //fprintf(fori, "%f\n", tmpQ);

            tmpdata = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_INTERACT_RANGE_YMIN);
            tmpQ = (tmpdata) * RES_STATIC_ELEVATIONMIN;
            hAudioObjectInteraction->interactionRange_elevationMin = tmpQ;
            //fprintf(fori, "%f\n", tmpQ);

            tmpdata = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_INTERACT_RANGE_YMAX);
            tmpQ = (tmpdata) * RES_STATIC_ELEVATIONMAX;
            hAudioObjectInteraction->interactionRange_elevationMax = tmpQ;
            //fprintf(fori, "%f\n", tmpQ);

            tmpdata = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_INTERACT_RANGE_ZMIN);
            tmpQ = (tmpdata) * RES_STATIC_DISTANCE;
            hAudioObjectInteraction->interactionRange_distanceMin = tmpQ;
            //fprintf(fori, "%f\n", tmpQ);

            tmpdata = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_INTERACT_RANGE_ZMAX);
            tmpQ = (tmpdata) * RES_STATIC_DISTANCE;
            hAudioObjectInteraction->interactionRange_distanceMax = tmpQ;
            //fprintf(fori, "%f\n", tmpQ);;
        }
    }

    return 0;
}


static int16_t Avs3AudioProgrammeRefScreenDec(
    AudioProgrammeRefScreenHandle hAudioProgrammeRefScreen,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t tmp;
    int16_t tmpPosition;
    float tmpValue;

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioProgrammeRefScreen->hasCartesian = tmp;
    //fprintf(fori, "%d\n", tmp);

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_ASPECT_RATIO);
    hAudioProgrammeRefScreen->aspectRatio = tmp;
    //fprintf(fori, "%d\n", tmp);

    if (hAudioProgrammeRefScreen->hasCartesian == 0) {
        tmpPosition = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SCREEN_AZIMUTH);
        tmpValue = (tmpPosition - (1 << (NBITS_SCREEN_AZIMUTH - 1))) * RES_STATIC_AZIMUTH;;
        hAudioProgrammeRefScreen->Position_azimuth = tmpValue;
        //fprintf(fori, "%f\n", tmpValue);

        tmpPosition = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SCREEN_ELEVATION);
        tmpValue = (tmpPosition) * RES_STATIC_SCREEELEVATION;
        hAudioProgrammeRefScreen->Position_elevation = tmpValue;
        //fprintf(fori, "%f\n", tmpValue);

        tmpPosition = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SCREEN_DISTANCE);
        tmpValue = (tmpPosition) * RES_STATIC_DISTANCE;
        hAudioProgrammeRefScreen->Position_distance = tmpValue;
        //fprintf(fori, "%f\n", tmpValue);

        tmpPosition = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SCREEN_POLAR);
        tmpValue = (tmpPosition) * RES_STATIC_POLARWIDTH;
        hAudioProgrammeRefScreen->polarScreenWidth = tmpValue;
        //fprintf(fori, "%f\n", tmpValue);
    }
    else {
        tmpPosition = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SCREEN_X);
        tmpValue = (tmpPosition - (1 << (NBITS_SCREEN_X - 1))) * RES_STATIC_X;
        hAudioProgrammeRefScreen->Position_X = tmpValue;
        //fprintf(fori, "%f\n", tmpValue);

        tmpPosition = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SCREEN_Y);
        tmpValue = (tmpPosition - (1 << (NBITS_SCREEN_Y - 1))) * RES_STATIC_Y;
        hAudioProgrammeRefScreen->Position_Y = tmpValue;
        //fprintf(fori, "%f\n", tmpValue);

        tmpPosition = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SCREEN_Z);
        tmpValue = (tmpPosition - (1 << (NBITS_SCREEN_Z - 1))) * RES_STATIC_Z;
        hAudioProgrammeRefScreen->Position_Z = tmpValue;
        //fprintf(fori, "%f\n", tmpValue);

        tmpPosition = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SCREEN_CARTESIAN);
        tmpValue = (tmpPosition) * RES_STATIC_CARTWIDTH;
        hAudioProgrammeRefScreen->CartesianScreenWidth = tmpValue;
        //fprintf(fori, "%f\n", tmpValue);
    }

    return 0;
}

static int16_t Avs3LoudnessDec(
    LoudnessMetadataHandle hLoudnessMeta,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t tmpValue;
    int16_t tmp;

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hLoudnessMeta->hasIntegratedLoudness = tmp;
    //fprintf(fori, "%d\n", tmp);

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hLoudnessMeta->hasLoudnessRange = tmp;
    //fprintf(fori, "%d\n", tmp);

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hLoudnessMeta->hasMaxTruePeak = tmp;
    //fprintf(fori, "%d\n", tmp);

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hLoudnessMeta->hasMaxMomentary = tmp;
    //fprintf(fori, "%d\n", tmp);

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hLoudnessMeta->hasMaxShortTerm = tmp;
    //fprintf(fori, "%d\n", tmp);

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hLoudnessMeta->hasDialogueLoudness = tmp;
    //fprintf(fori, "%d\n", tmp);

    if (hLoudnessMeta->hasIntegratedLoudness) {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_LOUDNESS);
        hLoudnessMeta->integratedLoudness = tmpValue * RES_STATIC_INTEGRATEDLOUDNESS;
        //fprintf(fori, "%f\n", hLoudnessMeta->integratedLoudness);
    }

    if (hLoudnessMeta->hasLoudnessRange) {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_LOUDNESS_RANGE);
        hLoudnessMeta->loudnessRange = tmpValue * RES_STATIC_LOUDNESSRANGE + 10.0f;
        //fprintf(fori, "%f\n", hLoudnessMeta->loudnessRange);
    }

    if (hLoudnessMeta->hasMaxTruePeak) {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_TURE_PEAK);
        hLoudnessMeta->maxTruePeak = tmpValue * RES_STATIC_MAXTRUEPEAK;
        //fprintf(fori, "%f\n", hLoudnessMeta->maxTruePeak);
    }

    if (hLoudnessMeta->hasMaxMomentary) {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_MAX_MOMNETARY);
        hLoudnessMeta->maxMomentary = tmpValue * RES_STATIC_MAXMOMENTARY;
        //fprintf(fori, "%f\n", hLoudnessMeta->maxMomentary);
    }

    if (hLoudnessMeta->hasMaxShortTerm) {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SHORT_TERM);
        hLoudnessMeta->maxShortTerm = tmpValue * RES_STATIC_MAXSHORTTERM;
        //fprintf(fori, "%f\n", hLoudnessMeta->maxShortTerm);
    }

    if (hLoudnessMeta->hasDialogueLoudness) {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_DL_LOUNDESS);
        hLoudnessMeta->dialogueLoudness = tmpValue * RES_STATIC_DIALOGUELOUDNESS;
        //fprintf(fori, "%f\n", hLoudnessMeta->dialogueLoudness);
    }

    return 0;
}


static int16_t Avs3AudioChannelFormatDec(
    AudioChannelFormatHandle hAudioChannelFormat,
    int16_t pack2ChannelRef[][3],
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t tmp;
    int16_t i, idx;
    int16_t tmpValue;

    hAudioChannelFormat->channelFormatIdx = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_CH_FORMAT_IDX);
    idx = hAudioChannelFormat->channelFormatIdx;
    //fprintf(fori, "%d\n", hAudioChannelFormat->channelFormatIdx);

    hAudioChannelFormat->hasChannelGain = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAudioChannelFormat->hasChannelGain);

    if (hAudioChannelFormat->hasChannelGain) {
        hAudioChannelFormat->gainUnit = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAudioChannelFormat->gainUnit);

        int16_t halfFlag;
        if (hAudioChannelFormat->gainUnit == 0) {
            halfFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_HALF_CHGAIN);

            if (halfFlag == 0) {
                hAudioChannelFormat->channelGain = tmpValue * RES_STATIC_OBJ_CHGAIN_H1;
                //fprintf(fori, "%f\n", hAudioChannelFormat->channelGain);
            }
            else {
                hAudioChannelFormat->channelGain = tmpValue * RES_STATIC_OBJ_CHGAIN_H2 + 1.0f;
                //fprintf(fori, "%f\n", hAudioChannelFormat->channelGain);
            }
        }
        else {
            halfFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_HALF_CHGAIN);

            if (halfFlag == 0) {
                hAudioChannelFormat->channelGain = tmpValue * RES_STATIC_OBJ_CHGAIN_DB_H1;
                //fprintf(fori, "%f\n", hAudioChannelFormat->channelGain);
            }
            else {
                hAudioChannelFormat->channelGain = tmpValue * RES_STATIC_OBJ_GAIN_DB_H2;
                //fprintf(fori, "%f\n", hAudioChannelFormat->channelGain);
            }
        }
    }

    if (pack2ChannelRef[idx][0] == 1) {
        if (pack2ChannelRef[idx][1] == 63) {
            DirectSpeakersPositionHandle hDirectSpeakersPosition = &hAudioChannelFormat->directSpeakersPositionData;
            Avs3DirectSpeakersPositionDec(hDirectSpeakersPosition, hBitstream);
        }
    }

    if (pack2ChannelRef[idx][0] == 2) {
        for (i = 0; i < pack2ChannelRef[idx][2]; i++) {
            tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_MATRIX_COEF);
            hAudioChannelFormat->MatrixCoef[i] = tmp * RES_MTRIXCOEF + 0.1f;
            //fprintf(fori, "%f\n", hAudioChannelFormat->MatrixCoef[i]);
        }
    }

    return 0;
}


static int16_t Avs3AudioPackFormatDec(
    AudioPackFormatHandle hAudioPackFormat,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream,
    int16_t pack2ChannelRef[][3])
{
    int16_t tmpFlag;
    int16_t i;
    int16_t tmpValue;

    hAudioPackFormat->packFormatIdx  = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_PACKS_IDX);
    //fprintf(fori, "%d\n", hAudioPackFormat->packFormatIdx);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioPackFormat->hasImportance = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioPackFormat->hasChannelReuse = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    if (hAudioPackFormat->hasImportance) {
        hAudioPackFormat->importance = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_PACK_IMPORTANCE);
        hAudioPackFormat->importance = AVS3_MIN(hAudioPackFormat->importance, 10);
        //fprintf(fori, "%d\n", hAudioPackFormat->importance);
    }

    hAudioPackFormat->typeLabel = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_PACK_LABEL);
    hAudioPackFormat->typeLabel = AVS3_MIN(AVS3_MAX(hAudioPackFormat->typeLabel, 1), 5);
    //fprintf(fori, "%d\n", hAudioPackFormat->typeLabel);

    tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_PACK_ABSDIS);
    hAudioPackFormat->absoluteDistance = tmpValue * RES_STATIC_ABSDISTANCE;
    //convert linear
    hAudioPackFormat->absoluteDistance = powf(10, hAudioPackFormat->absoluteDistance) - 1.0f;
    //fprintf(fori, "%f\n", hAudioPackFormat->absoluteDistance);

    if (hAudioPackFormat->typeLabel == 4) {
        hAudioPackFormat->normalization = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NORMALIZATION);
        //fprintf(fori, "%d\n", hAudioPackFormat->normalization);

        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NCFRDFDIST);
        hAudioPackFormat->nfcRefDist = tmpValue * RES_STATIC_NFCREFDIST;
        //fprintf(fori, "%f\n", hAudioPackFormat->nfcRefDist);

        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SCREENREF);
        hAudioPackFormat->screenRef = tmpValue;
        //fprintf(fori, "%d\n", tmpValue);

        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_HOAORDER);
        hAudioPackFormat->hoaOrder = tmpValue;
        //fprintf(fori, "%d\n", tmpValue);
    }

    if (hAudioPackFormat->typeLabel == 2 || hAudioPackFormat->typeLabel == 1) {
        hAudioPackFormat->packFormatID = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_PACKFORMATID);
        //fprintf(fori, "%d\n", hAudioPackFormat->packFormatID);

        if (hAudioPackFormat->typeLabel == 2) {
            hAudioPackFormat->numMatrixOutputChannel = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NUM_MATRIXCH);
            hAudioPackFormat->numMatrixOutputChannel += 1;
            //fprintf(fori, "%d\n", hAudioPackFormat->numMatrixOutputChannel);

            for (i = 0; i < hAudioPackFormat->numMatrixOutputChannel; i++) {
                DirectSpeakersPositionHandle hDirectSpeakersPosition = &(hAudioPackFormat->directSpeakersPositionData[i]);
                Avs3DirectSpeakersPositionDec(hDirectSpeakersPosition, hBitstream);
            }
        }
    }

    if (!hAudioPackFormat->hasChannelReuse) {
        hAudioPackFormat->packFormatStartIdx = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_PACKFORMAT_IDX);
        //fprintf(fori, "%d\n", hAudioPackFormat->packFormatStartIdx);
    }

    hAudioPackFormat->numChannels = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NUM_CHS);
    hAudioPackFormat->numChannels += 1;
    //fprintf(fori, "%d\n", hAudioPackFormat->numChannels);

    for (i = 0; i < hAudioPackFormat->numChannels; i++) {
        hAudioPackFormat->refChannelIdx[i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_REF_CH_IDX);
        //fprintf(fori, "%d\n", hAudioPackFormat->refChannelIdx[i]);
        int16_t idx = hAudioPackFormat->refChannelIdx[i];

        pack2ChannelRef[idx][0] = hAudioPackFormat->typeLabel;

        if (hAudioPackFormat->typeLabel == 1 || hAudioPackFormat->typeLabel == 2) {
            pack2ChannelRef[idx][1] = hAudioPackFormat->packFormatID;
        }
        if (hAudioPackFormat->typeLabel == 2) {
            pack2ChannelRef[idx][2] = hAudioPackFormat->numMatrixOutputChannel;
        }

        if (hAudioPackFormat->hasChannelReuse) {
            hAudioPackFormat->transChRef[i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_TRANS_CH_REF);
            //fprintf(fori, "%d\n", hAudioPackFormat->transChRef[i]);
        }
    }

    return 0;
}


static int16_t Avs3AudioObjectDec(
    AudioObjectHandle hAudioObject,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t tmpFlag;
    int16_t i;
    int16_t tmpValue;

    hAudioObject->objectIdx = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_IDX);
    //fprintf(fori, "%d\n", hAudioObject->objectIdx);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioObject->hasAudioObjectLanguage = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioObject->hasDialogue = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioObject->hasImportance = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioObject->hasDisableDucking = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioObject->hasInteract = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioObject->hasGain = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioObject->hasHeadLocked = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioObject->hasMute = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    if (hAudioObject->hasAudioObjectLanguage) {
        hAudioObject->audioObjectLanguage = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_LANGUAGE);
        //fprintf(fori, "%d\n", hAudioObject->audioObjectLanguage);
    }

    if (hAudioObject->hasDialogue) {
        DialogueHandle hDialogue = &hAudioObject->dialogueData;
        Avs3DialogueDec(hDialogue, hBitstream);
    }

    if (hAudioObject->hasImportance) {
        hAudioObject->importance = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_IMPORTANCE);
        hAudioObject->importance = AVS3_MIN(hAudioObject->importance, 10);
        //fprintf(fori, "%d\n", hAudioObject->importance);
    }

    if (hAudioObject->hasInteract) {
        for (i = 0; i < 24; i++) {
            hAudioObject->ObjectName[i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_NAME);
            //fprintf(fori, "%d\n", hAudioObject->ObjectName[i]);
        }

        AudioObjectInteractionHandle hAudioObjectInteraction = &hAudioObject->audioObjectInteractionData;
        Avs3AudioObjectInteractionDec(hAudioObjectInteraction, hBitstream);
    }

    if (hAudioObject->hasGain) {
        hAudioObject->gainUnit = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_GAINUNIT);
        //fprintf(fori, "%d\n", hAudioObject->gainUnit);

        int16_t halfFlag;
        halfFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        // linear
        if (hAudioObject->gainUnit == 0) {
            if (halfFlag == 0) {
                tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_HALFGAIN);
                hAudioObject->gain = tmpValue * RES_STATIC_OBJ_GAIN_H1;
                //fprintf(fori, "%f\n", hAudioObject->gain);
            }
            else {
                tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_HALFGAIN);
                hAudioObject->gain = tmpValue * RES_STATIC_OBJ_GAIN_H2 + 1.0f;
                //fprintf(fori, "%f\n", hAudioObject->gain);
            }
        }
        else {
            if (halfFlag == 0) {
                tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_HALFGAIN);
                hAudioObject->gain = tmpValue * RES_STATIC_OBJ_GAIN_DB_H1;
                //fprintf(fori, "%f\n", hAudioObject->gain);
            }
            else {
                tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_HALFGAIN);
                hAudioObject->gain = tmpValue * RES_STATIC_OBJ_GAIN_DB_H2;
                //fprintf(fori, "%f\n", hAudioObject->gain);
            }
        }
    }

    hAudioObject->numPacks = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NUM_PACKS);
    hAudioObject->numPacks += 1;
    //fprintf(fori, "%d\n", hAudioObject->numPacks);

    for (i = 0; i < hAudioObject->numPacks; i++) {
        hAudioObject->refPackFormatIdx[i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_REF_PACKS_IDX);
        //fprintf(fori, "%d\n", hAudioObject->refPackFormatIdx[i]);
    }

    return 0;
}


static int16_t Avs3AudioContentDec(
    AudioContentHandle hAudioContent,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t tmpFlag;
    int16_t i, j;

    hAudioContent->contentIdx = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_CON_IDX);
    //fprintf(fori, "%d\n", hAudioContent->contentIdx);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioContent->hasAudioContentLanguage = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioContent->hasLoudnessMetadata = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioContent->hasDialogue = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    tmpFlag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioContent->hasNumComplementaryObjectGroup = tmpFlag;
    //fprintf(fori, "%d\n", tmpFlag);

    if (hAudioContent->hasAudioContentLanguage) {
        hAudioContent->audioContentLanguage = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_CONTENT_LANGUAGE);
        //fprintf(fori, "%d\n", hAudioContent->audioContentLanguage);
    }

    if (hAudioContent->hasLoudnessMetadata) {
        LoudnessMetadataHandle hLoudnessMetadata = &hAudioContent->loudnessData;
        Avs3LoudnessDec(hLoudnessMetadata, hBitstream);
    }

    if (hAudioContent->hasDialogue) {
        DialogueHandle hDialogue = &hAudioContent->dialogueData;
        Avs3DialogueDec(hDialogue, hBitstream);
    }

    if (hAudioContent->hasNumComplementaryObjectGroup) {
        hAudioContent->numComplementaryObjectGroup = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NUM_GROUPS);
        hAudioContent->numComplementaryObjectGroup += 1;
        //fprintf(fori, "%d\n", hAudioContent->numComplementaryObjectGroup);

        for (i = 0; i < hAudioContent->numComplementaryObjectGroup; i++) {
            hAudioContent->numComplementaryObject[i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NUM_COM_OBJS);
            hAudioContent->numComplementaryObject[i] += 1;
            //fprintf(fori, "%d\n", hAudioContent->numComplementaryObject[i]);

            for (j = 0; j < hAudioContent->numComplementaryObject[i]; j++) {
                hAudioContent->ComplementaryObjectIdx[i][j] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_COM_OBJ_IDX);
                //fprintf(fori, "%d\n", hAudioContent->ComplementaryObjectIdx[i][j]);
            }
        }
    }

    hAudioContent->numObjects = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NUM_OBJECTS);
    hAudioContent->numObjects += 1;
    //fprintf(fori, "%d\n", hAudioContent->numObjects);

    for (i = 0; i < hAudioContent->numObjects; i++) {
        hAudioContent->refObjectIdx[i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_REF_OBJ_IDX);
        //fprintf(fori, "%d\n", hAudioContent->refObjectIdx[i]);
    }

    return 0;
}


static int16_t Avs3AudioProgrammeDec(
    AudioProgrammeHandle hAudioProgramme,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t tmpValue;
    int16_t tmp;

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioProgramme->hasAudioProgrammeLanguage = tmp;
    //fprintf(fori, "%d\n", tmp);

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioProgramme->hasMaxDuckingDepth = tmp;
    //fprintf(fori, "%d\n", tmp);

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioProgramme->hasLoudnessMetadata = tmp;
    //fprintf(fori, "%d\n", tmp);

    tmp = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    hAudioProgramme->hasAudioProgrammeRefScreen = tmp;
    //fprintf(fori, "%d\n", tmp);

    if (hAudioProgramme->hasAudioProgrammeLanguage) {
        hAudioProgramme->audioProgrammeLanguage = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_AUDIO_PL);
        //fprintf(fori, "%d\n", hAudioProgramme->audioProgrammeLanguage);
    }

    if (hAudioProgramme->hasMaxDuckingDepth) {
        tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_MAX_DUCK);
        hAudioProgramme->maxDuckingDepth = tmpValue * RES_DUCKINGDEPTH;
        //fprintf(fori, "%f\n", hAudioProgramme->maxDuckingDepth);
    }

    // LoudnessMetadata
    if (hAudioProgramme->hasLoudnessMetadata) {
        LoudnessMetadataHandle hLoudnessMetadata = &hAudioProgramme->loudnessData;
        Avs3LoudnessDec(hLoudnessMetadata, hBitstream);
    }

    // AudioProgrammeRefScreen
    if (hAudioProgramme->hasAudioProgrammeRefScreen) {
        AudioProgrammeRefScreenHandle  hAudioProgrammeRefScreen = &hAudioProgramme->audioProgrammeRefScreenData;
        Avs3AudioProgrammeRefScreenDec(hAudioProgrammeRefScreen, hBitstream);
    }

    // audioContent
    hAudioProgramme->numContents = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NUMCONTENTS);
    hAudioProgramme->numContents += 1;
    //fprintf(fori, "%d\n", hAudioProgramme->numContents);

    for (int i = 0; i < hAudioProgramme->numContents; i++) {
        hAudioProgramme->refContentIdx[i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_REF_CON_IDX);
        //fprintf(fori, "%d\n", hAudioProgramme->refContentIdx[i]);
    }

    return 0;
}


static int16_t Avs3BasicL1Dec(
    Avs3BasicL1Handle hAvs3BasicL1,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t i;

    AudioProgrammeHandle hAudioProgrammeMeta = &hAvs3BasicL1->audioProgrammeMeta;
    Avs3AudioProgrammeDec(hAudioProgrammeMeta, hBitstream);

    // content
    hAvs3BasicL1->numOfContents = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NUMCONTENTS);
    hAvs3BasicL1->numOfContents += 1;
    //fprintf(fori, "%d\n", hAvs3BasicL1->numOfContents);

    for (i = 0; i < hAvs3BasicL1->numOfContents; i++) {
        AudioContentHandle hAudioContent = &hAvs3BasicL1->audioContentData[i];
        Avs3AudioContentDec(hAudioContent, hBitstream);
    }

    // Object
    hAvs3BasicL1->numOfObjects = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NUM_OBJECTS);
    hAvs3BasicL1->numOfObjects += 1;
    //fprintf(fori, "%d\n", hAvs3BasicL1->numOfObjects);

    for (i = 0; i < hAvs3BasicL1->numOfObjects; i++) {
        AudioObjectHandle hAudioObject = &hAvs3BasicL1->audioObjectData[i];

        Avs3AudioObjectDec(hAudioObject, hBitstream);
    }

    // packs
    hAvs3BasicL1->numOfPacks = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NUM_PACKS);
    hAvs3BasicL1->numOfPacks += 1;
    //fprintf(fori, "%d\n", hAvs3BasicL1->numOfPacks);

    int16_t pack2ChannelRef[32][3];

    for (i = 0; i < hAvs3BasicL1->numOfPacks; i++) {
        AudioPackFormatHandle hAudioPackFormat = &hAvs3BasicL1->audioPackFormatData[i];
        Avs3AudioPackFormatDec(hAudioPackFormat, hBitstream, pack2ChannelRef);
    }

    // channel
    hAvs3BasicL1->numOfChannels = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_NUM_CHS);
    hAvs3BasicL1->numOfChannels += 1;
    //fprintf(fori, "%d\n", hAvs3BasicL1->numOfChannels);

    for (i = 0; i < hAvs3BasicL1->numOfChannels; i++) {
        AudioChannelFormatHandle hAudioChannelFormat = &(hAvs3BasicL1->audioChannelFormatData[i]);
        Avs3AudioChannelFormatDec(hAudioChannelFormat, pack2ChannelRef, hBitstream);
    }

    return 0;
}


static int16_t Avs3MetaDataStaticDec(
    Avs3MetaDataStaticHandle hStaticMetaData,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    // hasVrExt
    hStaticMetaData->hasVrExt = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hStaticMetaData->hasVrExt);

    hStaticMetaData->basicLevel = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_BASICLEVEL);
    hStaticMetaData->basicLevel = AVS3_MIN(hStaticMetaData->basicLevel, 1);
    //fprintf(fori, "%d\n", hStaticMetaData->basicLevel);

    if (hStaticMetaData->basicLevel == 0 || hStaticMetaData->basicLevel == 1) {
        Avs3BasicL1Handle  hAvs3BasicL1 = &hStaticMetaData->avs3BasicL1;
        Avs3BasicL1Dec(hAvs3BasicL1, hBitstream);
    }

    if (hStaticMetaData->hasVrExt) {
        hStaticMetaData->vrExtLevel = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_VREXTLEVEL);
        //fprintf(fori, "%d\n", hStaticMetaData->vrExtLevel);

        if (hStaticMetaData->vrExtLevel == 0) {
            Avs3VrExtL1MetaDataHandle hAvs3VrExtL1MetaData = &hStaticMetaData->avs3VrExtL1MetaData;
            Avs3VrExtDec(hAvs3VrExtL1MetaData, hBitstream);
        }
    }

    return 0;
}


// Dynamic
static int16_t Avs3DmL2Dec(
    Avs3DmL2MetaDataHandle hAvs3DmL2MetaData,
    int16_t muteFlag,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    if (muteFlag == 0) {
        int16_t tmpValue;
        hAvs3DmL2MetaData->hasChannelLock = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL2MetaData->hasChannelLock);

        if (hAvs3DmL2MetaData->hasChannelLock == 1) {
            hAvs3DmL2MetaData->channelLock = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_CHANNEL_LOCK);
            //fprintf(fori, "%d\n", hAvs3DmL2MetaData->channelLock);

            if (hAvs3DmL2MetaData->channelLock == 1) {
                tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_CHANNELLOCK_MAXDIST);
                hAvs3DmL2MetaData->channelLockMaxDist = tmpValue * RES_CHANNELLOCK_MAXDIST;
                //fprintf(fori, "%f\n", hAvs3DmL2MetaData->channelLockMaxDist);
            }
        }

        hAvs3DmL2MetaData->hasObjectDivergence = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL2MetaData->hasObjectDivergence);

        if (hAvs3DmL2MetaData->hasObjectDivergence == 1) {
            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_DIVERGENCE);
            hAvs3DmL2MetaData->objDivergence = tmpValue * RES_OBJ_DIVERGENCE;
            //fprintf(fori, "%f\n", hAvs3DmL2MetaData->objDivergence);

            if (hAvs3DmL2MetaData->objDivergence != 0.0f) {
                tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_DIVERGENCE_AZI_RANGE);
                hAvs3DmL2MetaData->objDiverAzimuthRange = tmpValue * RES_OBJ_DIVERGENCE_AZI_RANGE;
                //fprintf(fori, "%f\n", hAvs3DmL2MetaData->objDiverAzimuthRange);
            }
        }

        hAvs3DmL2MetaData->hasObjectScreenRef = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL2MetaData->hasObjectScreenRef);

        if (hAvs3DmL2MetaData->hasObjectScreenRef == 1) {
            hAvs3DmL2MetaData->objScreenRef = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_SCREEN_REF);
            //fprintf(fori, "%d\n", hAvs3DmL2MetaData->objScreenRef);
        }

        hAvs3DmL2MetaData->hasScreenEdgeLock = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL2MetaData->hasObjectScreenRef);

        if (hAvs3DmL2MetaData->hasScreenEdgeLock == 1) {
            hAvs3DmL2MetaData->screenEdgeLock = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_SCREEN_EDGE_LOCK);
            //fprintf(fori, "%d\n", hAvs3DmL2MetaData->screenEdgeLock);
        }
    }

    return 0;
}


static int16_t Avs3DmL1Dec(
    Avs3DmL1MetaDataHandle hAvs3DmL1MetaData,
    int16_t muteFlag,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    if (muteFlag == 0) {
        hAvs3DmL1MetaData->cartesian = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL1MetaData->cartesian);

        hAvs3DmL1MetaData->hasObjExtent = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL1MetaData->hasObjExtent);

        hAvs3DmL1MetaData->hasObjGain = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL1MetaData->hasObjGain);

        hAvs3DmL1MetaData->hasObjDiffuse = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL1MetaData->hasObjDiffuse);

        hAvs3DmL1MetaData->hasObjImportance = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3DmL1MetaData->hasObjImportance);

        int16_t tmpValue;
        if (hAvs3DmL1MetaData->cartesian == 0) {
            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_AZIMUTH);
            tmpValue -= (1 << (NBITS_OBJ_AZIMUTH - 1));
            hAvs3DmL1MetaData->objAzimuth = tmpValue * RES_OBJ_AZIMUTH;
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objAzimuth);

            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_ELEVATION);
            tmpValue -= (1 << (NBITS_OBJ_ELEVATION - 1));
            hAvs3DmL1MetaData->objElevation= tmpValue * RES_OBJ_ELEVATION;
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objElevation);

            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_DISTANCE);
            hAvs3DmL1MetaData->objDistance = tmpValue * RES_OBJ_DISTANCE;
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objDistance);

            if (hAvs3DmL1MetaData->hasObjExtent) {
                tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_WIDTH);
                hAvs3DmL1MetaData->objWidth = tmpValue * RES_OBJ_WIDTH;
                //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objWidth);

                tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_HEIGHT);
                hAvs3DmL1MetaData->objHeight = tmpValue * RES_OBJ_HEIGHT;
                //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objHeight);

                tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_DEPTH);
                hAvs3DmL1MetaData->objDepth = tmpValue * RES_OBJ_DEPTH;
                //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objDepth);
            }
        }
        else {
            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_X);
            tmpValue -= (1 << (NBITS_OBJ_X - 1));
            hAvs3DmL1MetaData->obj_x = tmpValue * RES_OBJ_X;
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->obj_x);

            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_Y);
            tmpValue -= (1 << (NBITS_OBJ_Y - 1));
            hAvs3DmL1MetaData->obj_y = tmpValue * RES_OBJ_Y;
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->obj_y);

            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_Z);
            tmpValue -= (1 << (NBITS_OBJ_Z - 1));
            hAvs3DmL1MetaData->obj_z = tmpValue * RES_OBJ_Z;
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->obj_z);

            if (hAvs3DmL1MetaData->hasObjExtent) {
                tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_WIDTH_X);
                hAvs3DmL1MetaData->objWidth_x = tmpValue * RES_OBJ_WIDTH_X;
                //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objWidth_x);

                tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_HEIGHT_Y);
                hAvs3DmL1MetaData->objHeight_y = tmpValue * RES_OBJ_HEIGHT_Y;
                //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objHeight_y);

                tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_DEPTH_Z);
                hAvs3DmL1MetaData->objDepth_z = tmpValue * RES_OBJ_DEPTH_Z;
                //fprintf(fori, "%f\n", hAvs3DmL1MetaData->objDepth_z);
            }
        }

        if (hAvs3DmL1MetaData->hasObjGain) {
            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_GAIN);
            hAvs3DmL1MetaData->gain = tmpValue * RES_OBJ_GAIN;
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->gain);
        }

        if (hAvs3DmL1MetaData->hasObjDiffuse) {
            tmpValue = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_OBJ_DIFFUSE);
            hAvs3DmL1MetaData->diffuse = tmpValue * RES_OBJ_DIFFUSE;
            //fprintf(fori, "%f\n", hAvs3DmL1MetaData->diffuse);
        }

        hAvs3DmL1MetaData->jumpPosition = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_JUMPPOSITION);
        //fprintf(fori, "%d\n", hAvs3DmL1MetaData->jumpPosition);

        if (hAvs3DmL1MetaData->hasObjImportance) {
            hAvs3DmL1MetaData->importance = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_IMPORTANCE);
            hAvs3DmL1MetaData->importance = AVS3_MIN(hAvs3DmL1MetaData->importance, 10);
            //fprintf(fori, "%d\n", hAvs3DmL1MetaData->importance);
        }
    }
    
    return 0;
}


static int16_t Avs3MetaDataDynamicDec(
    Avs3MetaDataDynamicHandle hAvs3MetaDataDynamic,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    hAvs3MetaDataDynamic->dmLevel = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_DMLEVEL);
    //fprintf(fori, "%d\n", hAvs3MetaDataDynamic->dmLevel);

    for (int16_t i = 0; i < hAvs3MetaDataDynamic->numDmChans; i++) {
        hAvs3MetaDataDynamic->muteFlag[i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
        //fprintf(fori, "%d\n", hAvs3MetaDataDynamic->muteFlag[i]);

        hAvs3MetaDataDynamic->transChRef[i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_DM_TRANSCHREF);
        //fprintf(fori, "%d\n",  hAvs3MetaDataDynamic->transChRef[i]);

        if (hAvs3MetaDataDynamic->dmLevel == 0) {
            Avs3DmL1MetaDataHandle hAvs3DmL1MetaData = &(hAvs3MetaDataDynamic->avs3DmL1MetaData[i]);
            Avs3DmL1Dec(hAvs3DmL1MetaData, hAvs3MetaDataDynamic->muteFlag[i], hBitstream);
        }

        if (hAvs3MetaDataDynamic->dmLevel == 1) {
            Avs3DmL1MetaDataHandle hAvs3DmL1MetaData = &(hAvs3MetaDataDynamic->avs3DmL1MetaData[i]);
            Avs3DmL1Dec(hAvs3DmL1MetaData, hAvs3MetaDataDynamic->muteFlag[i], hBitstream);

            Avs3DmL2MetaDataHandle hAvs3DmL2MetaData = &(hAvs3MetaDataDynamic->avs3DmL2MetaData[i]);
            Avs3DmL2Dec(hAvs3DmL2MetaData, hAvs3MetaDataDynamic->muteFlag[i], hBitstream);
        }
    }

    return 0;
}


void Avs3MetadataDec(AVS3DecoderHandle hAvs3Dec)
{
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream = hAvs3Dec->hBitstream;

    Avs3MetaDataHandle hAvs3MetaData = &(hAvs3Dec->hMetadataDec->avs3MetaData);
    hAvs3MetaData->hasStaticMeta = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAvs3MetaData->hasStaticMeta);

    if (hAvs3MetaData->hasStaticMeta) {
        Avs3MetaDataStaticHandle hStaticMetaData = &(hAvs3MetaData->avs3MetaDataStatic);
        Avs3MetaDataStaticDec(hStaticMetaData, hBitstream);
    }

    hAvs3MetaData->hasDynamicMeta = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_META_FLAG);
    //fprintf(fori, "%d\n", hAvs3MetaData->hasDynamicMeta);

    if (hAvs3MetaData->hasDynamicMeta) {
        Avs3MetaDataDynamicHandle  hAvs3MetaDataDynamic = &hAvs3MetaData->avs3MetaDataDynamic;
        hAvs3MetaDataDynamic->numDmChans = hAvs3Dec->numObjsOutput;
        Avs3MetaDataDynamicDec(hAvs3MetaDataDynamic, hBitstream);
    }

    return;
}
