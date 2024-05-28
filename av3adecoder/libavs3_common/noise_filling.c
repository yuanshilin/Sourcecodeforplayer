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
#include <float.h>

#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_stat_com.h"
#include "avs3_prot_com.h"


/*
Extract noise filling parameter using latent feature before and after quantization
I/O params:
    float *origLatent               (i) orignal latent feature
    int32_t *quantizedLatent        (i) quantized latent feature
    float *quantileMedian           (i) quantile median for quantizer
    int16_t numLatentDim            (i) dimension of latent feature
    int16_t numLatentChannels       (i) num channels of latent feature
    int16_t numNfDim                (i) dimension of latent feature for NF param extraction
    int16_t numGroups               (i) number of groups for current frame
    int16_t *groupIndicator         (i) group indicator vector, 0 for transient, 1 for others
    float *nfParamQ                 (o) quantized noise filling param
    int16_t *nfParamQIdx            (o) quantization index for noise filling param
*/
void ExtractNfParam(
    float *origLatent,
    int32_t *quantizedLatent,
    float *quantileMedian,
    int16_t numLatentDim,
    int16_t numLatentChannels,
    int16_t numNfDim,
    int16_t numGroups,
    int16_t *groupIndicator,
    float *nfParamQ,
    int16_t *nfParamQIdx
)
{
    int16_t numZeroedParam = 0;
    int16_t numTransientBlock = 0;                          // number of transient blocks
    int16_t numOtherBlock = 0;                              // number of other blocks
    int16_t startIdx[N_GROUP_SHORT_WIN] = { 0 };
    int16_t endIdx[N_GROUP_SHORT_WIN] = { 0 };
    float nfParam[N_GROUP_SHORT_WIN] = { 0.0f };
    float tmp;

    if (numGroups == 1) {
        startIdx[0] = 0;
        startIdx[1] = 0;
        endIdx[0] = numNfDim;
        endIdx[1] = numNfDim;
    }
    else {
        for (int16_t i = 0; i < N_BLOCK_SHORT; i++) {
            if (groupIndicator[i] == 0) {
                numTransientBlock++;
            }
            else {
                numOtherBlock++;
            }
        }

        startIdx[0] = 0;
        endIdx[0] = (short)((float)numNfDim / N_BLOCK_SHORT * numTransientBlock);

        startIdx[1] = numLatentDim / N_BLOCK_SHORT * numTransientBlock;
        endIdx[1] = startIdx[1] + (short)((float)numNfDim / N_BLOCK_SHORT * numOtherBlock);
    }

    for (int16_t group = 0; group < numGroups; group++) {

        // for latent param quantized to zero, sum abs error
        for (int16_t i = 0; i < numLatentChannels; i++) {

            // sum in each channel
            tmp = 0.0f;
            numZeroedParam = 0;

            for (int16_t j = startIdx[group]; j < endIdx[group]; j++) {
                if (quantizedLatent[j + i * numLatentDim] == 0) {
                    numZeroedParam++;
                    tmp += (float)fabs(origLatent[j + i * numLatentDim] - quantileMedian[i]);
                }
            }

            // add channel average to nf param
            if (numZeroedParam == 0) {
                nfParam[group] += 0.0f;
            }
            else {
                nfParam[group] += tmp / numZeroedParam;
            }
        }

        // average over channels
        nfParam[group] /= numLatentChannels;

        // quantization
        // 3 bit uniform Q, [0, almost 0.30]
        nfParamQIdx[group] = (int16_t)(floor(0.5f + nfParam[group] * 23.34f));
        nfParamQIdx[group] = AVS3_MAX(nfParamQIdx[group], 0);
        nfParamQIdx[group] = AVS3_MIN(nfParamQIdx[group], ((1 << NBITS_NF_PARAM) - 1));
        nfParamQ[group] = (float)(nfParamQIdx[group]) / 23.34f;
    }

    return;
}


void NfParamPostProc(
    NeuralQcData *neuralQcData,
    float *mdctSpectrum,
    int16_t numLinesForNf,
    int32_t totalBitrate,
    int16_t nChans
)
{
    int32_t bitratePerCh;
    float mdctEnerSpec[BLOCK_LEN_LONG] = { 0.0f };
    float logMdctEnerSpec[BLOCK_LEN_LONG] = { 0.0f };
    float sfm, peakAvgRatio;

    bitratePerCh = (int32_t)(totalBitrate / (float)nChans);

    if (bitratePerCh <= 32000) {

        // get ener spec and log ener spec for core band
        for (int16_t i = 0; i < numLinesForNf; i++) {
            mdctEnerSpec[i] = mdctSpectrum[i] * mdctSpectrum[i];
            logMdctEnerSpec[i] = max(0.0f, (float)(log10(max(FLT_MIN, mdctEnerSpec[i])) / log10(2.0)));
        }

        // get sfm and peak-average ratio for core band
        sfm = BweGetSfm(mdctEnerSpec, logMdctEnerSpec, 0, numLinesForNf);
        peakAvgRatio = BweGetPeakAvgRatio(logMdctEnerSpec, 0, numLinesForNf);

        // get final sfm param
        sfm /= peakAvgRatio;

        // if sfm smaller than threshold, lower NF param by one step
        if (sfm < 0.003f) {
            for (int16_t i = 0; i < N_GROUP_SHORT_WIN; i++) {
                neuralQcData->nfParamQIdx[i] = max(0, neuralQcData->nfParamQIdx[i] - 1);
                // dequantize noise filling parameter
                neuralQcData->nfParam[i] = (float)(neuralQcData->nfParamQIdx[i]) / 23.34f;
            }
        }
    }

    return;
}


/*
Add generated noise to dequantized latent feature
I/O params:
    float *dequantizedLatent            (i/o) dequantized latent feature
    float *quantileMedian               (i)   quantile median for quantizer
    int16_t numLatentDim                (i)   dimension of latent feature
    int16_t numLatentChannels           (i)   number of channels for latent feature
    int16_t numNfDim                    (i)   dimension of latent feature for NF param extraction
    int16_t numGroups                   (i)   number of groups for current frame
    int16_t *groupIndicator             (i)   group indicator vector, 0 for transient, 1 for others
    float *nfParamQ                     (o)   quantized noise filling param
    int16_t nfParamQIdx                 (i)   quantization index of noise filling param
*/
void LatentNoiseFilling(
    float *dequantizedLatent,
    float *quantileMedian,
    int16_t numLatentDim,
    int16_t numLatentChannels,
    int16_t numNfDim,
    int16_t numGroups,
    int16_t *groupIndicator,
    float *nfParamQ,
    int16_t *nfParamQIdx
)
{
    float noise;
    int16_t numTransientBlock = 0;                          // number of transient blocks
    int16_t numOtherBlock = 0;                              // number of other blocks
    int16_t startIdx[N_GROUP_SHORT_WIN] = { 0 };
    int16_t endIdx[N_GROUP_SHORT_WIN] = { 0 };

    if (numGroups == 1) {
        startIdx[0] = 0;
        startIdx[1] = 0;
        endIdx[0] = numNfDim;
        endIdx[1] = numNfDim;
    }
    else {
        for (int16_t i = 0; i < N_BLOCK_SHORT; i++) {
            if (groupIndicator[i] == 0) {
                numTransientBlock++;
            }
            else {
                numOtherBlock++;
            }
        }

        startIdx[0] = 0;
        endIdx[0] = (short)((float)numNfDim / N_BLOCK_SHORT * numTransientBlock);

        startIdx[1] = numLatentDim / N_BLOCK_SHORT * numTransientBlock;
        endIdx[1] = startIdx[1] + (short)((float)numNfDim / N_BLOCK_SHORT * numOtherBlock);
    }

    for (int16_t group = 0; group < numGroups; group++) {

        // dequantize noise filling parameter
        nfParamQ[group] = (float)(nfParamQIdx[group]) / 23.34f;

        // add noise to dequantized latent
        // if dequantized value equals quantile median
        for (int16_t i = startIdx[group]; i < endIdx[group]; i++) {
            for (int16_t j = 0; j < numLatentChannels; j++) {
                if (dequantizedLatent[i + j * numLatentDim] == quantileMedian[j]) {
                    // generate base noise, [-1, 1]
                    noise = (float)rand() / (float)RAND_MAX;
                    noise = noise * 2.0f - 1.0f;
                    // apply nf param
                    noise *= nfParamQ[group];
                    // add noise
                    dequantizedLatent[i + j * numLatentDim] += noise;
                }
            }
        }
    }

    return;
}
