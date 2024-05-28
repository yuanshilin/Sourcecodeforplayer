
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "avs3_options.h"
#include "avs3_stat_com.h"
#include "avs3_prot_com.h"


/*
Context model encoding and decoding process
for hyper model, context part
I/O params:
    NeuralCodecHandle contextCodecSt                (i/o) context codec st handle
    float *ctxEncInput                              (i)   input feature of context model
    uint8_t *contextBitstream                       (o)   bitstream for context model
    int16_t *contextNumBytes                        (o)   number of bytes for context bitstream
    int16_t isBrEst                                 (i)   flag for bitrate estimation or real encoding, 1 for BR est
*/
static int16_t ContextEncDec(
    NeuralCodecHandle contextCodecSt,
    float *ctxEncInput,
    uint8_t *contextBitstream,
    int16_t *contextNumBytes,
    int16_t isBrEst
)
{
    float *ctxEncOutput = NULL;                     // pointer to context cnn model output, no malloc
    int32_t *ctxQuantizedLatent = NULL;             // context model quantized latent, with malloc
    int16_t ctxLatentSize;                          // latent size for context model
    int32_t *ctxFlattenLatent = NULL;               // context model flattened latent, with malloc
    int16_t *ctxCdfIndex = NULL;                    // context model, cdf index for each dim of flattened latent, with malloc
    float *ctxDequantizedLatent = NULL;             // context model dequantized latent, with malloc

    // Get related handle of context model
    ModelStructHandle ctxEncHandle = contextCodecSt->encoderHandle;
    ModelStructHandle ctxDecHandle = contextCodecSt->decoderHandle;
    QuantizerHandle ctxQuantizerHandle = contextCodecSt->quantizerHandle;
    RangeCoderConfigHandle ctxRcHandle = contextCodecSt->rangeCoderConfig;

    // Loop for context encoder cnn layers
    for (int16_t layerIdx = 0; layerIdx < ctxEncHandle->numLayers; layerIdx++) {

        // cnn layer handle
        CnnStructHandle cnnLayer;
        cnnLayer = ctxEncHandle->cnnLayers[layerIdx];

        // last cnn layer handle
        CnnStructHandle cnnLayerLast = NULL;
        if (layerIdx != 0) {
            cnnLayerLast = ctxEncHandle->cnnLayers[layerIdx - 1];
        }

        // perform cnn convolution
        if (layerIdx == 0) {
            Conv1D(cnnLayer, ctxEncInput);
        }
        else {
            Conv1D(cnnLayer, cnnLayerLast->featOut);
        }
    }

    // set context enc output to the output buffer of last cnnlayer in context enc model
    ctxEncOutput = ctxEncHandle->cnnLayers[ctxEncHandle->numLayers - 1]->featOut;

    // malloc quantized latent buffer for context layer
    ctxQuantizedLatent = (int32_t *)malloc(sizeof(int32_t) * contextCodecSt->numLatentEncode *
        contextCodecSt->numLatentChannels);

    // latent quantization process
    LatentQuantize(ctxQuantizerHandle, ctxEncOutput, ctxQuantizedLatent,
        contextCodecSt->numLatentEncode, contextCodecSt->numLatentChannels);

    // Range encoding
    // Flatten quantized latent and set index for each dimension, VAE here
    ctxLatentSize = contextCodecSt->numLatentEncode * contextCodecSt->numLatentChannels;
    ctxFlattenLatent = (int32_t *)malloc(sizeof(int32_t) * ctxLatentSize);
    ctxCdfIndex = (int16_t *)malloc(sizeof(int16_t) * ctxLatentSize);
    for (int16_t i = 0; i < contextCodecSt->numLatentEncode; i++) {
        for (int16_t j = 0; j < contextCodecSt->numLatentChannels; j++) {
            ctxFlattenLatent[i * contextCodecSt->numLatentChannels + j] =
                ctxQuantizedLatent[i + j * contextCodecSt->numLatentEncode];
            ctxCdfIndex[i*contextCodecSt->numLatentChannels + j] = j;
        }
    }

    // perform range coding
    if (isBrEst == 0) {
        // real encoding
        RangeEncodeProcess(ctxRcHandle, ctxFlattenLatent, ctxLatentSize, ctxCdfIndex, contextBitstream, contextNumBytes);
    }
    else {
        // bitrate estimation
        RangeEncodeProcessBrEst(ctxRcHandle, ctxFlattenLatent, ctxLatentSize, ctxCdfIndex, contextNumBytes);
    }

    // malloc context dequantized latent
    ctxDequantizedLatent = (float *)malloc(sizeof(float) * contextCodecSt->numLatentEncode *
        contextCodecSt->numLatentChannels);

    // latent dequantization process
    LatentDequantize(ctxQuantizerHandle, ctxQuantizedLatent, ctxDequantizedLatent,
        contextCodecSt->numLatentEncode, contextCodecSt->numLatentChannels);

    // Loop for context decoder cnn layers
    for (int16_t layerIdx = 0; layerIdx < ctxDecHandle->numLayers; layerIdx++) {

        // cnn layer handle
        CnnStructHandle cnnLayer;
        cnnLayer = ctxDecHandle->cnnLayers[layerIdx];

        // last cnn layer handle
        CnnStructHandle cnnLayerLast = NULL;
        if (layerIdx != 0) {
            cnnLayerLast = ctxDecHandle->cnnLayers[layerIdx - 1];
        }

        // perform cnn convolution
        if (layerIdx == 0) {
            if (cnnLayer->stride == 2) {
                Conv1DTranspose2Part(cnnLayer, ctxDequantizedLatent);
            }
            else {
                Conv1DTranspose(cnnLayer, ctxDequantizedLatent);
            }
        }
        else {
            if (cnnLayer->stride == 2) {
                Conv1DTranspose2Part(cnnLayer, cnnLayerLast->featOut);
            }
            else {
                Conv1DTranspose(cnnLayer, cnnLayerLast->featOut);
            }
        }
    }

    // free memory
    free(ctxQuantizedLatent);
    ctxQuantizedLatent = NULL;

    free(ctxFlattenLatent);
    ctxFlattenLatent = NULL;

    free(ctxCdfIndex);
    ctxCdfIndex = NULL;

    free(ctxDequantizedLatent);
    ctxDequantizedLatent = NULL;

    return 0;
}


/*
Quantization of feature scale param
I/O params:
    float featureScale                  (i) original feature scale param
    int16_t *isFeatAmplified            (o) flag for whether feature is amplified or not
    int16_t *scaleQIdx                  (o) quantization index of feature scale
*/
static float QuantizeFeatureScale(
    float featureScale,
    int16_t *isFeatAmplified,
    int16_t *scaleQIdx
)
{
    float featureScaleQ;

    if (featureScale <= 1.0f) {
        // for feature scale <= 1.0
        // use 7bits uniform quantization
        // range: [0, 1]
        *scaleQIdx = (int16_t)(floor(0.5f + 127.0f * (featureScale)));
        if (*scaleQIdx < 0) {
            *scaleQIdx = 0;
        }
        else if (*scaleQIdx > 127) {
            *scaleQIdx = 127;
        }
        // dequantize for next rate iter
        featureScaleQ = (float)(*scaleQIdx) / 127.0f;
        // set flag of feature amplification
        *isFeatAmplified = 0;
    }
    else {
        // for feature scale > 1.0
        // use 7 bits non-uniform quantization, here log10 transform
        // range: (1.0, almost 30.0]
        *scaleQIdx = (int16_t)(floor(0.5f + 86.0f * log10(featureScale)));
        if (*scaleQIdx < 0) {
            *scaleQIdx = 0;
        }
        else if (*scaleQIdx > 127) {
            *scaleQIdx = 127;
        }
        // dequantize for next rate iter
        featureScaleQ = (float)pow(10.0f, (float)(*scaleQIdx) / 86.0f);
        // set flag of feature amplification
        *isFeatAmplified = 1;
    }

    return featureScaleQ;
}


static float UpdateFeatureScale(
    int16_t targetBytes,
    int16_t currBytes,
    float currScale,
    float *lowBoundScale,
    float *lowBoundBytes,
    float *upBoundScale,
    float *upBoundBytes,
    int16_t iter,
    int16_t nnTypeConfig
)
{
    float newScale;
    float lowerFactor = 0.825f;

    // update upper and lower bounds
    if (currBytes < targetBytes) {
        // lower than target, but higher than curr low bound
        // update low bound scale and bytes
        if (*lowBoundBytes < currBytes) {
            *lowBoundScale = currScale;
            *lowBoundBytes = (float)currBytes;
        }
        // curr bytes same as low buond bytes
        // update low bound scale to higher value
        else if (*lowBoundBytes == currBytes) {
            *lowBoundScale = max(*lowBoundScale, currScale);
        }
    }
    else if (currBytes > targetBytes){
        // higher than target, but lower than curr up bound
        // update up boudn scale and bytes
        if (*upBoundBytes > currBytes) {
            *upBoundScale = currScale;
            *upBoundBytes = (float)currBytes;
        }
        // cur bytes same as higher bound bytes
        // update up bound scale to lower value
        else if (*upBoundBytes == currBytes) {
            *upBoundScale = min(*upBoundScale, currScale);
        }
    }

    // have found upper or lower bound
    if (*lowBoundScale != 0.0f && *upBoundScale != 0.0f) {

        // For some rare cases, scale between upper and lower does not result in 
        // numBytes between upper and lower
        // in that case, scale update is stuck and a quite low lower point will be used at last
        if ((currBytes > *upBoundBytes || currBytes < *lowBoundBytes) && 
            ((float)targetBytes / *lowBoundBytes) > 1.3f)
        {
            // update scale based on lower scale and target/lowBytes
            // for greater iter, update faster to make it converge
            newScale = *lowBoundScale * ((float)targetBytes / *lowBoundBytes) * 
                (0.7f + 0.2f*((float)iter / MAX_RATE_ITERS));
        }
        else {
            // normal case
            newScale = *lowBoundScale +
                lowerFactor * ((float)targetBytes - *lowBoundBytes) / (*upBoundBytes - *lowBoundBytes) *
                (*upBoundScale - *lowBoundScale);
        }
    }
    else {
        float modScale = (float)targetBytes / (float)currBytes;
        newScale = currScale * (modScale * modScale);

        // make it faster to cross the target line
        if (nnTypeConfig == NN_TYPE_DEFAULT_MAIN) {
            if (newScale < 1.0f) {
                newScale *= 0.5f;
            }
            else if (newScale > 1.0f) {
                newScale /= lowerFactor;
            }
        }
        else if (nnTypeConfig == NN_TYPE_DEFAULT_LC){
            if (modScale < 1.0f) {
                newScale *= 0.5f;
            }
            else if (modScale > 1.0f) {
                newScale /= lowerFactor;
            }
        }
    }

    return newScale;
}


/*
MDCT quantization and encode, using Hyper model
includes analysis transform, context model enc/dec, quantization and RC
I/O params:
    NeuralCodecHandle baseCodecSt           (i) top level struct handle for base codec
    NeuralCodecHandle contextCodecSt        (i) top level struct handle for context codec
    NeuralQcData *neuralQcData              (o) neural Q/C module data structure, including bs, feature scale and NF params
    float mdctCoefs[][2]                    (i) input MDCT coefficients
    int16_t numLinesEncode                  (i) length of input MDCT coefficients
    int16_t numChannels                     (i) number channels of input MDCT coefficients
    int16_t numGroups                       (i) number of groups for current frame
    int16_t *groupIndicator                 (i) group indicator vector, 0 for transient, 1 for others
    int16_t targetNumBytes                  (i) number of target bytes for bitstream
*/
int16_t MdctQuantEncodeHyper(
    NeuralCodecHandle baseCodecSt,
    NeuralCodecHandle contextCodecSt,
    NeuralQcData *neuralQcData,
    float mdctCoefs[][2],
    int16_t numLinesEncode,
    int16_t numChannels,
    int16_t numLinesNonZero,
    int16_t numGroups,
    int16_t *groupIndicator,
    int16_t targetNumBytes
)
{
    float *baseInput = NULL;                        // pointer to base cnn model input, with malloc
    float *baseOutput = NULL;                       // pointer to base cnn model output, no malloc
    float *scaledBaseOutput = NULL;                 // pointer to scaled base model output, with malloc
    int32_t *baseQuantizedLatent = NULL;            // base model quantized latent, with malloc
    int16_t baseLatentSize;                         // latent size for base model
    int32_t *baseFlattenLatent = NULL;              // base model flattened latent, with malloc
    int16_t *baseCdfIndex = NULL;                   // base model, cdf index for each dim of flattened latent, with malloc

    float *ctxEncInput = NULL;                      // pointer to context cnn model input, with malloc
    float *ctxDecOutput = NULL;                     // context dec model output buffer, no malloc

    int16_t sumNumBytes = 0;                        // total number of bytes in bitstream of current frame
    float featureScale = 1.0f;                      // feature scale parameter
    int16_t isFeatAmplified = 0;                    // flag for whether feature is amplified
    int16_t scaleQIdx = -1;                         // quantization index of feature scale
    float nfParamQ[N_GROUP_SHORT_WIN];              // quantized noise filling parameter
    int16_t nfParamQIdx[N_GROUP_SHORT_WIN];         // quantization index of noise filling parameter

    // rate loop parameters
    float lowBoundScale = 0.0f;
    float lowBoundBytes = 0.0f;
    float upBoundScale = 0.0f;
    float upBoundBytes = 10000.0f;
    // back up qc data for rate loop
    NeuralQcData backQcData;

    // Get related handle of base model
    ModelStructHandle baseEncHandle = baseCodecSt->encoderHandle;
    QuantizerHandle baseQuantizerHandle = baseCodecSt->quantizerHandle;
    RangeCoderConfigHandle baseRcHandle = baseCodecSt->rangeCoderConfig;

    // Get related handle of qc data
    uint8_t *baseBitstream = neuralQcData->baseBitstream;
    int16_t *baseNumBytes = &(neuralQcData->baseNumBytes);
    uint8_t *contextBitstream = neuralQcData->contextBitstream;
    int16_t *contextNumBytes = &(neuralQcData->contextNumBytes);

    // number of latent for NF extraction
    int16_t numLatentNF = 0;

    // Malloc input feature buffer for base model
    baseInput = (float *)malloc(sizeof(float) * numLinesEncode * numChannels);
    // copy input to buffer
    for (int16_t i = 0; i < numLinesEncode; i++) {
        for (int16_t j = 0; j < numChannels; j++) {
            baseInput[i + j * numLinesEncode] = mdctCoefs[i][j];
        }
    }

    // Loop for base encoder cnn layers
    for (int16_t layerIdx = 0; layerIdx < baseEncHandle->numLayers; layerIdx++) {

        // cnn layer handle
        CnnStructHandle cnnLayer;
        cnnLayer = baseEncHandle->cnnLayers[layerIdx];

        // last cnn layer handle
        CnnStructHandle cnnLayerLast = NULL;
        if (layerIdx != 0) {
            cnnLayerLast = baseEncHandle->cnnLayers[layerIdx - 1];
        }

        // perform cnn convolution
        if (layerIdx == 0) {
            Conv1D(cnnLayer, baseInput);
        }
        else {
            Conv1D(cnnLayer, cnnLayerLast->featOut);
        }
    }

    // set base layer output to the output buffer of last cnnlayer in base model
    baseOutput = baseEncHandle->cnnLayers[baseEncHandle->numLayers - 1]->featOut;

    // Malloc scaled base output
    scaledBaseOutput = (float *)malloc(sizeof(float) * baseCodecSt->numLatentEncode * baseCodecSt->numLatentChannels);

    // Malloc input feature buffer for context model
    ctxEncInput = (float *)malloc(sizeof(float) * baseCodecSt->numLatentEncode * baseCodecSt->numLatentChannels);

    // Malloc cdf index for base model
    baseLatentSize = baseCodecSt->numLatentEncode * baseCodecSt->numLatentChannels;
    baseCdfIndex = (int16_t *)malloc(sizeof(int16_t) * baseLatentSize);

    // Malloc quantized latent buffer for base layer
    baseQuantizedLatent = (int32_t *)malloc(sizeof(int32_t) * baseCodecSt->numLatentEncode * baseCodecSt->numLatentChannels);

    // Malloc flattened quantized latent for base model
    baseFlattenLatent = (int32_t *)malloc(sizeof(int32_t) * baseLatentSize);

    // Init feature scale
    featureScale = 1.0f;

    // Reset backup qc data
    InitNeuralQcData(&backQcData);

    // Rate iterations
    for (int16_t iter = 0; iter < MAX_RATE_ITERS; iter++) {

        // clear bitstream buffer, reset number of bytes used
        SetUC(baseBitstream, 0, MAX_QC_BS_LENGTH);
        SetUC(contextBitstream, 0, MAX_QC_BS_LENGTH);
        *baseNumBytes = 0;
        *contextNumBytes = 0;

        // context model input is the 'abs' of base model output
        // scale base model output
        for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
            for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
                ctxEncInput[i + j * baseCodecSt->numLatentEncode] = featureScale *
                    (float)fabs(baseOutput[i + j * baseCodecSt->numLatentEncode]);
                scaledBaseOutput[i + j * baseCodecSt->numLatentEncode] = featureScale *
                    baseOutput[i + j * baseCodecSt->numLatentEncode];
            }
        }

        // context model encode and decode process
        // isBrEst set to 1 for rate loop
        ContextEncDec(contextCodecSt, ctxEncInput, contextBitstream, contextNumBytes, 1);

        // set context dec output to the output buffer of last cnnlayer in context dec model
        ModelStructHandle ctxDecHandle = contextCodecSt->decoderHandle;
        ctxDecOutput = ctxDecHandle->cnnLayers[ctxDecHandle->numLayers - 1]->featOut;

        // set cdf index for each dimension, using context model output here
        for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
            for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
                int16_t index;
                for (index = 0; index < baseCodecSt->numContextScale; index++) {
                    if (baseCodecSt->contextScale[index] >= ctxDecOutput[i + j * baseCodecSt->numLatentEncode]) {
                        baseCdfIndex[i * baseCodecSt->numLatentChannels + j] = index;
                        break;
                    }
                }
                if (index == baseCodecSt->numContextScale) {
                    baseCdfIndex[i * baseCodecSt->numLatentChannels + j] = baseCodecSt->numContextScale - 1;
                }
            }
        }

        // latent quantization process
        LatentQuantize(baseQuantizerHandle, scaledBaseOutput, baseQuantizedLatent,
            baseCodecSt->numLatentEncode, baseCodecSt->numLatentChannels);

        // Range encoding
        // Flatten quantized latent
        for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
            for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
                baseFlattenLatent[i*baseCodecSt->numLatentChannels + j] =
                    baseQuantizedLatent[i + j * baseCodecSt->numLatentEncode];
            }
        }

        // perform range coding
        // use bitrate estimation version for rate loop
        RangeEncodeProcessBrEst(baseRcHandle, baseFlattenLatent, baseLatentSize, baseCdfIndex, baseNumBytes);

        // overall number of bytes
        sumNumBytes = *baseNumBytes + *contextNumBytes;
        //sumNumBytes = *baseNumBytes;

        // update feature scale
        if (iter != (MAX_RATE_ITERS - 1)) {

            // save backup qc data if lower than target
            if (sumNumBytes < targetNumBytes && sumNumBytes > lowBoundBytes) {
                backQcData.baseNumBytes = *baseNumBytes;
                backQcData.contextNumBytes = *contextNumBytes;
                backQcData.featureScale = featureScale;
                backQcData.isFeatAmplified = isFeatAmplified;
                backQcData.scaleQIdx = scaleQIdx;
            }

            //featureScale *= (float)targetNumBytes / (float)sumNumBytes;
            featureScale = UpdateFeatureScale(targetNumBytes, sumNumBytes, featureScale,
                &lowBoundScale, &lowBoundBytes, &upBoundScale, &upBoundBytes, iter, NN_TYPE_DEFAULT_MAIN);

            // quantization of feature scale
            // use the dequantized value in next scaling
            featureScale = QuantizeFeatureScale(featureScale, &isFeatAmplified, &scaleQIdx);
        }
    }

    // save feature scale info to qc data
    if (sumNumBytes > targetNumBytes && backQcData.baseNumBytes != 0) {

        // using backup data when last time is above target
        *baseNumBytes = backQcData.baseNumBytes;
        *contextNumBytes = backQcData.contextNumBytes;

        neuralQcData->featureScale = backQcData.featureScale;

        if (backQcData.scaleQIdx == -1) {
            // feature scale is never quantized
            // this happens when low bound is updated only once at the first loop
            // the following loops are always beyond target
            // then the first featureScale = 1.0 is not quantized at all
            neuralQcData->isFeatAmplified = 0;
            neuralQcData->scaleQIdx = 127;
        }
        else {
            neuralQcData->isFeatAmplified = backQcData.isFeatAmplified;
            neuralQcData->scaleQIdx = backQcData.scaleQIdx;
        }
    }
    else {
        // using last time data
        neuralQcData->featureScale = featureScale;
        neuralQcData->isFeatAmplified = isFeatAmplified;
        neuralQcData->scaleQIdx = scaleQIdx;
    }

    // Perform transform and encoding once more, get NF param
    // using RC with bitstream

    // clear bitstream buffer, reset number of bytes used
    SetUC(baseBitstream, 0, MAX_QC_BS_LENGTH);
    SetUC(contextBitstream, 0, MAX_QC_BS_LENGTH);
    *baseNumBytes = 0;
    *contextNumBytes = 0;

    // context model input is the 'abs' of base model output
    // scale base model output
    for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
        for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
            ctxEncInput[i + j * baseCodecSt->numLatentEncode] = neuralQcData->featureScale *
                (float)fabs(baseOutput[i + j * baseCodecSt->numLatentEncode]);
            scaledBaseOutput[i + j * baseCodecSt->numLatentEncode] = neuralQcData->featureScale *
                baseOutput[i + j * baseCodecSt->numLatentEncode];
        }
    }

    // context model encode and decode process
    // isBrEst set to 0 for real encoding
    ContextEncDec(contextCodecSt, ctxEncInput, contextBitstream, contextNumBytes, 0);

    // set context dec output to the output buffer of last cnnlayer in context dec model
    ModelStructHandle ctxDecHandle = contextCodecSt->decoderHandle;
    ctxDecOutput = ctxDecHandle->cnnLayers[ctxDecHandle->numLayers - 1]->featOut;

    // set cdf index for each dimension, using context model output here
    for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
        for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
            int16_t index;
            for (index = 0; index < baseCodecSt->numContextScale; index++) {
                if (baseCodecSt->contextScale[index] >= ctxDecOutput[i + j * baseCodecSt->numLatentEncode]) {
                    baseCdfIndex[i * baseCodecSt->numLatentChannels + j] = index;
                    break;
                }
            }
            if (index == baseCodecSt->numContextScale) {
                baseCdfIndex[i * baseCodecSt->numLatentChannels + j] = baseCodecSt->numContextScale - 1;
            }
        }
    }

    // latent quantization process
    LatentQuantize(baseQuantizerHandle, scaledBaseOutput, baseQuantizedLatent,
        baseCodecSt->numLatentEncode, baseCodecSt->numLatentChannels);

    // Range encoding
    // Flatten quantized latent
    for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
        for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
            baseFlattenLatent[i*baseCodecSt->numLatentChannels + j] =
                baseQuantizedLatent[i + j * baseCodecSt->numLatentEncode];
        }
    }

    // perform range coding
    RangeEncodeProcess(baseRcHandle, baseFlattenLatent, baseLatentSize, baseCdfIndex, baseBitstream, baseNumBytes);

    // get number of latent used for NF extraction
    numLatentNF = numLinesNonZero;
    for (int16_t i = 0; i < baseEncHandle->numLayers; i++) {
        numLatentNF /= baseEncHandle->cnnLayers[i]->stride;
    }
    // extract noise filling parameters
    ExtractNfParam(scaledBaseOutput, baseQuantizedLatent, baseQuantizerHandle->quantileMedian,
        baseCodecSt->numLatentEncode, baseCodecSt->numLatentChannels, numLatentNF,
        numGroups, groupIndicator, nfParamQ, nfParamQIdx);
    // copy NF parameters to st
    for (int16_t i = 0; i < N_GROUP_SHORT_WIN; i++) {
        neuralQcData->nfParam[i] = nfParamQ[i];
        neuralQcData->nfParamQIdx[i] = nfParamQIdx[i];
    }

    // padding zeros for used bytes is lower than target
    if ((*baseNumBytes + *contextNumBytes) < targetNumBytes) {
        SetUC(baseBitstream + *baseNumBytes, 0, targetNumBytes - (*baseNumBytes + *contextNumBytes));
        *baseNumBytes = targetNumBytes - *contextNumBytes;
    }

    // free memory
    free(baseInput);
    baseInput = NULL;

    free(scaledBaseOutput);
    scaledBaseOutput = NULL;

    free(baseQuantizedLatent);
    baseQuantizedLatent = NULL;

    free(baseFlattenLatent);
    baseFlattenLatent = NULL;

    free(baseCdfIndex);
    baseCdfIndex = NULL;

    free(ctxEncInput);
    ctxEncInput = NULL;

    return 0;
}


/*
Quantization of feature scale param, for LC profile
I/O params:
    float featureScale                  (i) original feature scale param
    int16_t *isFeatAmplified            (o) flag for whether feature is amplified or not
    int16_t *scaleQIdx                  (o) quantization index of feature scale
*/
static float QuantizeFeatureScaleLc(
    float featureScale,
    int16_t *isFeatAmplified,
    int16_t *scaleQIdx
)
{
    float featureScaleQ;

    // use 8bits uniform quantization in log domain
    // range: [0, 1]
    *scaleQIdx = (int16_t)(floor(255.5f + 31.875f * log10(featureScale)));

    if (*scaleQIdx < 0) {
        *scaleQIdx = 0;
    }
    else if (*scaleQIdx > 255) {
        *scaleQIdx = 255;
    }
    // dequantize for next rate iter
    featureScaleQ = (float)pow(10.0f, ((float)(*scaleQIdx) - 255.0f) / 31.875f);

    // set flag of feature amplification
    *isFeatAmplified = 0;

    return featureScaleQ;
}


/*
MDCT quantization and encode, using Hyper model, LC profile
includes analysis transform, context model enc/dec, quantization and RC
I/O params:
    NeuralCodecHandle baseCodecSt           (i) top level struct handle for base codec
    NeuralCodecHandle contextCodecSt        (i) top level struct handle for context codec
    NeuralQcData *neuralQcData              (o) neural Q/C module data structure, including bs, feature scale and NF params
    float mdctCoefs[][2]                    (i) input MDCT coefficients
    int16_t numLinesEncode                  (i) length of input MDCT coefficients
    int16_t numChannels                     (i) number channels of input MDCT coefficients
    int16_t numGroups                       (i) number of groups for current frame
    int16_t *groupIndicator                 (i) group indicator vector, 0 for transient, 1 for others
    int16_t targetNumBytes                  (i) number of target bytes for bitstream
*/
int16_t MdctQuantEncodeHyperLc(
    NeuralCodecHandle baseCodecSt,
    NeuralCodecHandle contextCodecSt,
    NeuralQcData *neuralQcData,
    float mdctCoefs[][2],
    int16_t numLinesEncode,
    int16_t numChannels,
    int16_t numLinesNonZero,
    int16_t numGroups,
    int16_t *groupIndicator,
    float *prevFeatureScale,
    int16_t targetNumBytes
)
{
    float *baseInput = NULL;                        // pointer to base cnn model input, with malloc
    float *baseOutput = NULL;                       // pointer to base cnn model output, no malloc
    float *scaledBaseOutput = NULL;                 // pointer to scaled base model output, with malloc
    int32_t *baseQuantizedLatent = NULL;            // base model quantized latent, with malloc
    int16_t baseLatentSize;                         // latent size for base model
    int32_t *baseFlattenLatent = NULL;              // base model flattened latent, with malloc
    int16_t *baseCdfIndex = NULL;                   // base model, cdf index for each dim of flattened latent, with malloc

    float *ctxEncInput = NULL;                     // pointer to context cnn model input, with malloc
    float *ctxDecOutput = NULL;                    // context dec model output buffer, no malloc

    int16_t sumNumBytes = 0;                        // total number of bytes in bitstream of current frame
    float featureScale = 1.0f;                      // feature scale parameter
    int16_t isFeatAmplified = 0;                    // flag for whether feature is amplified
    int16_t scaleQIdx = -1;                         // quantization index of feature scale
    float nfParamQ[N_GROUP_SHORT_WIN];              // quantized noise filling parameter
    int16_t nfParamQIdx[N_GROUP_SHORT_WIN];         // quantization index of noise filling parameter

    // rate loop parameters
    float lowBoundScale = 0.0f;
    float lowBoundBytes = 0.0f;
    float upBoundScale = 0.0f;
    float upBoundBytes = 10000.0f;
    // back up qc data for rate loop
    NeuralQcData backQcData;

    // Get related handle of base model
    ModelStructHandle baseEncHandle = baseCodecSt->encoderHandle;
    QuantizerHandle baseQuantizerHandle = baseCodecSt->quantizerHandle;
    RangeCoderConfigHandle baseRcHandle = baseCodecSt->rangeCoderConfig;

    // Get related handle of qc data
    uint8_t *baseBitstream = neuralQcData->baseBitstream;
    int16_t *baseNumBytes = &(neuralQcData->baseNumBytes);
    uint8_t *contextBitstream = neuralQcData->contextBitstream;
    int16_t *contextNumBytes = &(neuralQcData->contextNumBytes);

    // number of latent for NF extraction
    int16_t numLatentNF = 0;

    // Malloc input feature buffer
    baseInput = (float *)malloc(sizeof(float) * baseCodecSt->numLatentEncode * baseCodecSt->numLatentChannels);
    // copy input to buffer
    for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
        for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
            baseInput[i + j * baseCodecSt->numLatentEncode] = mdctCoefs[i * baseCodecSt->numLatentChannels + j][0];
        }
    }

    // set base layer output to the output buffer of last cnnlayer in base model
    baseOutput = baseEncHandle->cnnLayers[baseEncHandle->numLayers - 1]->featOut;

    // Malloc scaled base output
    scaledBaseOutput = (float *)malloc(sizeof(float) * baseCodecSt->numLatentEncode * baseCodecSt->numLatentChannels);

    // Malloc input feature buffer for context model
    ctxEncInput = (float *)malloc(sizeof(float) * baseCodecSt->numLatentEncode * baseCodecSt->numLatentChannels);

    // Malloc cdf index for base model
    baseLatentSize = baseCodecSt->numLatentEncode * baseCodecSt->numLatentChannels;
    baseCdfIndex = (int16_t *)malloc(sizeof(int16_t) * baseLatentSize);

    // Malloc quantized latent buffer for base layer
    baseQuantizedLatent = (int32_t *)malloc(sizeof(int32_t) * baseCodecSt->numLatentEncode * baseCodecSt->numLatentChannels);

    // Malloc flattened quantized latent for base model
    baseFlattenLatent = (int32_t *)malloc(sizeof(int32_t) * baseLatentSize);

    // Init feature scale
    featureScale = *prevFeatureScale;                                                      // 201, Q idx for feature scale value 0.02
    scaleQIdx = (int16_t)(floor(255.5f + 31.875f*log10(featureScale)));

    // Reset backup qc data
    InitNeuralQcData(&backQcData);

    // Rate iterations
    for (int16_t iter = 0; iter < MAX_RATE_ITERS; iter++) {

        // clear bitstream buffer, reset number of bytes used
        SetUC(baseBitstream, 0, MAX_QC_BS_LENGTH);
        SetUC(contextBitstream, 0, MAX_QC_BS_LENGTH);
        *baseNumBytes = 0;
        *contextNumBytes = 0;

        // context model input is the 'abs' of base model output
        // scale base model output
        for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
            for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
                ctxEncInput[i + j * baseCodecSt->numLatentEncode] = featureScale *
                    (float)fabs(baseInput[i + j * baseCodecSt->numLatentEncode]);
                scaledBaseOutput[i + j * baseCodecSt->numLatentEncode] = featureScale *
                    baseInput[i + j * baseCodecSt->numLatentEncode];
            }
        }

        // context model encode and decode process
        // isBrEst set to 1 for rate loop
        ContextEncDec(contextCodecSt, ctxEncInput, contextBitstream, contextNumBytes, 1);

        // set context dec output to the output buffer of last cnnlayer in context dec model
        ModelStructHandle ctxDecHandle = contextCodecSt->decoderHandle;
        ctxDecOutput = ctxDecHandle->cnnLayers[ctxDecHandle->numLayers - 1]->featOut;

        // set cdf index for each dimension, using context model output here
        for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
            for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
                int16_t index;
                for (index = 0; index < baseCodecSt->numContextScale; index++) {
                    if (baseCodecSt->contextScale[index] >= ctxDecOutput[i + j * baseCodecSt->numLatentEncode]) {
                        baseCdfIndex[i * baseCodecSt->numLatentChannels + j] = index;
                        break;
                    }
                }
                if (index == baseCodecSt->numContextScale) {
                    baseCdfIndex[i * baseCodecSt->numLatentChannels + j] = baseCodecSt->numContextScale - 1;
                }
            }
        }

        // latent quantization process
        LatentQuantize(baseQuantizerHandle, scaledBaseOutput, baseQuantizedLatent,
            baseCodecSt->numLatentEncode, baseCodecSt->numLatentChannels);

        // Range encoding
        // Flatten quantized latent
        for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
            for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
                baseFlattenLatent[i * baseCodecSt->numLatentChannels + j] =
                    baseQuantizedLatent[i + j * baseCodecSt->numLatentEncode];
            }
        }

        // perform range coding
        // use bitrate estimation version for rate loop
        RangeEncodeProcessBrEst(baseRcHandle, baseFlattenLatent, baseLatentSize, baseCdfIndex, baseNumBytes);

        // overall number of bytes
        sumNumBytes = *baseNumBytes + *contextNumBytes;
        //sumNumBytes = *baseNumBytes;

        // update feature scale
        if (iter != (MAX_RATE_ITERS - 1)) {

            // save backup qc data if lower than target
            if (sumNumBytes < targetNumBytes && sumNumBytes > lowBoundBytes) {
                backQcData.baseNumBytes = *baseNumBytes;
                backQcData.contextNumBytes = *contextNumBytes;
                backQcData.featureScale = featureScale;
                backQcData.isFeatAmplified = isFeatAmplified;
                backQcData.scaleQIdx = scaleQIdx;
            }

            float tmpBytesRatio = (float)sumNumBytes / (float)targetNumBytes;
            // todo .. prev stop by 0.9 full of bitrate, influce quality(may 10% bitrate waste,cause every frame size is the same,fill with 0 if not full),but 0.95 or more may not reach
            if (tmpBytesRatio >= 0.9f && tmpBytesRatio <= 1.0f) {
                break;
            }

            //featureScale *= (float)targetNumBytes / (float)sumNumBytes;
            featureScale = UpdateFeatureScale(targetNumBytes, sumNumBytes, featureScale,
                &lowBoundScale, &lowBoundBytes, &upBoundScale, &upBoundBytes, iter, NN_TYPE_DEFAULT_LC);

            // quantization of feature scale
            // use the dequantized value in next scaling
            featureScale = QuantizeFeatureScaleLc(featureScale, &isFeatAmplified, &scaleQIdx);
        }
    }

    // save feature scale info to qc data
    if (sumNumBytes > targetNumBytes && backQcData.baseNumBytes != 0) {

        // using backup data when last time is above target
        *baseNumBytes = backQcData.baseNumBytes;
        *contextNumBytes = backQcData.contextNumBytes;

        neuralQcData->featureScale = backQcData.featureScale;

        neuralQcData->isFeatAmplified = backQcData.isFeatAmplified;
        neuralQcData->scaleQIdx = backQcData.scaleQIdx;
    }
    else {
        // using last time data
        neuralQcData->featureScale = featureScale;
        neuralQcData->isFeatAmplified = isFeatAmplified;
        neuralQcData->scaleQIdx = scaleQIdx;
    }

    // Perform transform and encoding once more, get NF param
    // using RC with bitstream

    //save feature sacale for next frame 
    *prevFeatureScale = neuralQcData->featureScale;

    // clear bitstream buffer, reset number of bytes used
    SetUC(baseBitstream, 0, MAX_QC_BS_LENGTH);
    SetUC(contextBitstream, 0, MAX_QC_BS_LENGTH);
    *baseNumBytes = 0;
    *contextNumBytes = 0;

    // context model input is the 'abs' of base model output
    // scale base model output
    for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
        for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
            ctxEncInput[i + j * baseCodecSt->numLatentEncode] = neuralQcData->featureScale *
                (float)fabs(baseInput[i + j * baseCodecSt->numLatentEncode]);
            scaledBaseOutput[i + j * baseCodecSt->numLatentEncode] = neuralQcData->featureScale *
                baseInput[i + j * baseCodecSt->numLatentEncode];
        }
    }

    // context model encode and decode process
    // isBrEst set to 0 for real encoding
    ContextEncDec(contextCodecSt, ctxEncInput, contextBitstream, contextNumBytes, 0);

    // set context dec output to the output buffer of last cnnlayer in context dec model
    ModelStructHandle ctxDecHandle = contextCodecSt->decoderHandle;
    ctxDecOutput = ctxDecHandle->cnnLayers[ctxDecHandle->numLayers - 1]->featOut;

    // set cdf index for each dimension, using context model output here
    for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
        for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
            int16_t index;
            for (index = 0; index < baseCodecSt->numContextScale; index++) {
                if (baseCodecSt->contextScale[index] >= ctxDecOutput[i + j * baseCodecSt->numLatentEncode]) {
                    baseCdfIndex[i * baseCodecSt->numLatentChannels + j] = index;
                    break;
                }
            }
            if (index == baseCodecSt->numContextScale) {
                baseCdfIndex[i * baseCodecSt->numLatentChannels + j] = baseCodecSt->numContextScale - 1;
            }
        }
    }

    // latent quantization process
    LatentQuantize(baseQuantizerHandle, scaledBaseOutput, baseQuantizedLatent,
        baseCodecSt->numLatentEncode, baseCodecSt->numLatentChannels);

    // Range encoding
    // Flatten quantized latent
    for (int16_t i = 0; i < baseCodecSt->numLatentEncode; i++) {
        for (int16_t j = 0; j < baseCodecSt->numLatentChannels; j++) {
            baseFlattenLatent[i*baseCodecSt->numLatentChannels + j] =
                baseQuantizedLatent[i + j * baseCodecSt->numLatentEncode];
        }
    }

    // perform range coding
    RangeEncodeProcess(baseRcHandle, baseFlattenLatent, baseLatentSize, baseCdfIndex, baseBitstream, baseNumBytes);

    // get number of latent used for NF extraction
    numLatentNF = numLinesNonZero / baseCodecSt->numLatentChannels;
    // extract noise filling parameters
    ExtractNfParam(scaledBaseOutput, baseQuantizedLatent, baseQuantizerHandle->quantileMedian,
        baseCodecSt->numLatentEncode, baseCodecSt->numLatentChannels, numLatentNF,
        numGroups, groupIndicator, nfParamQ, nfParamQIdx);
    // copy NF parameters to st
    for (int16_t i = 0; i < N_GROUP_SHORT_WIN; i++) {
        neuralQcData->nfParam[i] = nfParamQ[i];
        neuralQcData->nfParamQIdx[i] = nfParamQIdx[i];
    }

    // padding zeros for used bytes is lower than target
    if ((*baseNumBytes + *contextNumBytes) < targetNumBytes) {
        SetUC(baseBitstream + *baseNumBytes, 0, targetNumBytes - (*baseNumBytes + *contextNumBytes));
        *baseNumBytes = targetNumBytes - *contextNumBytes;
    }

    // free memory
    free(baseInput);
    baseInput = NULL;

    free(scaledBaseOutput);
    scaledBaseOutput = NULL;

    free(baseQuantizedLatent);
    baseQuantizedLatent = NULL;

    free(baseFlattenLatent);
    baseFlattenLatent = NULL;

    free(baseCdfIndex);
    baseCdfIndex = NULL;

    free(ctxEncInput);
    ctxEncInput = NULL;

    return 0;
}
