
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "avs3_options.h"
#include "avs3_stat_com.h"
#include "avs3_prot_com.h"


/*
Context model decoding process
for hyper model, context part
I/O params:
    NeuralCodecHandle contextCodecSt                (i/o) context codec st handle
    uint8_t *contextBitstream                       (i)   bitstream for context model
    int16_t *contextNumBytes                        (i)   number of bytes for context bitstream
*/
static int16_t ContextDec(
    NeuralCodecHandle contextCodecSt,
    uint8_t *contextBitstream,
    int16_t contextNumBytes
)
{
    int16_t ctxNumLatentEncode;                     // latent dim for context model
    int16_t ctxNumLatentChannels;                   // latent num channels for context model
    int16_t ctxLatentSize;                          // latent size for context model
    int32_t *ctxFlattenLatent = NULL;               // context model flattend latent, with malloc
    int16_t *ctxCdfIndex = NULL;                    // cdf index for each dim of flattened latent, for ctx model, with malloc
    int32_t *ctxQuantizedLatent = NULL;             // context model quantized latent, with malloc
    float *ctxDequantizedLatent = NULL;             // context model dequantized latent, with malloc

    // get related handle for context model
    ModelStructHandle ctxDecHandle = contextCodecSt->decoderHandle;
    QuantizerHandle ctxQuantizerHandle = contextCodecSt->quantizerHandle;
    RangeCoderConfigHandle ctxRcHandle = contextCodecSt->rangeCoderConfig;
    // get related dims for context model
    ctxNumLatentEncode = contextCodecSt->numLatentEncode;
    ctxNumLatentChannels = contextCodecSt->numLatentChannels;
    ctxLatentSize = ctxNumLatentEncode * ctxNumLatentChannels;

    // malloc flatten latent for context model
    ctxFlattenLatent = (int32_t *)malloc(sizeof(int32_t) * ctxLatentSize);

    // malloc cdf index for context model, set cdf index according to channel idx
    ctxCdfIndex = (int16_t *)malloc(sizeof(int16_t) * ctxLatentSize);
    for (int16_t i = 0; i < ctxNumLatentEncode; i++) {
        for (int16_t j = 0; j < ctxNumLatentChannels; j++) {
            ctxCdfIndex[i * ctxNumLatentChannels + j] = j;
        }
    }

    // perform range decoding
    RangeDecodeProcess(ctxRcHandle, ctxFlattenLatent, ctxLatentSize, ctxCdfIndex,
        contextBitstream, contextNumBytes);

    // malloc and set quantized latent of context model
    ctxQuantizedLatent = (int32_t *)malloc(sizeof(int32_t) * ctxNumLatentEncode * ctxNumLatentChannels);
    for (int16_t i = 0; i < ctxNumLatentEncode; i++) {
        for (int16_t j = 0; j < ctxNumLatentChannels; j++) {
            ctxQuantizedLatent[i + j * ctxNumLatentEncode] = ctxFlattenLatent[i * ctxNumLatentChannels + j];
        }
    }

    // malloc dequantized latent for context model
    ctxDequantizedLatent = (float *)malloc(sizeof(float) * ctxNumLatentEncode * ctxNumLatentChannels);

    // perform dequantization
    LatentDequantize(ctxQuantizerHandle, ctxQuantizedLatent, ctxDequantizedLatent,
        ctxNumLatentEncode, ctxNumLatentChannels);

    // Loop for context model decoder cnn layers
    for (int16_t layerIdx = 0; layerIdx < ctxDecHandle->numLayers; layerIdx++) {

        // cnn layer handle
        CnnStructHandle cnnLayer;
        cnnLayer = ctxDecHandle->cnnLayers[layerIdx];

        // last cnn layer handle
        CnnStructHandle cnnLayerLast = NULL;
        if (layerIdx != 0) {
            cnnLayerLast = ctxDecHandle->cnnLayers[layerIdx - 1];
        }

        // perform transpose cnn convolution
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
    free(ctxFlattenLatent);
    ctxFlattenLatent = NULL;

    free(ctxCdfIndex);
    ctxCdfIndex = NULL;

    free(ctxQuantizedLatent);
    ctxQuantizedLatent = NULL;

    free(ctxDequantizedLatent);
    ctxDequantizedLatent = NULL;

    return 0;
}


/*
MDCT dequantization and decode
includes RC, dequantization and synthesis transform
I/O params:
    NeuralCodecHandle baseCodecSt                   (i) top level struct handle for base codec
    NeuralCodecHandle contextCodecSt                (i) top level struct handle for context codec
    NeuralQcData *neuralQcData                      (i) neural Q/C module data structure, including bs, feature scale and NF params
    float featureDec[][2]                           (o) decoded features
    int16_t numLinesNoiseFill                       (i) number of mdct lines for noise filling
    int16_t numGroups                               (i) number of groups for current frame
    int16_t *groupIndicator                         (i) group indicator vector, 0 for transient, 1 for others
*/
int16_t MdctDequantDecodeHyper(
    NeuralCodecHandle baseCodecSt,
    NeuralCodecHandle contextCodecSt,
    NeuralQcData *neuralQcData,
    float featureDec[][2],
    int16_t numLinesNoiseFill,
    int16_t numGroups,
    int16_t *groupIndicator
)
{
    float *ctxDecOutput = NULL;                      // context dec model output buffer, no malloc

    int16_t baseNumLatentEncode;                     // latent dim for base model
    int16_t baseNumLatentChannels;                   // latent num channels for base model
    int16_t baseLatentSize;                          // latent size for base model
    int32_t *baseFlattenLatent = NULL;               // base model flattened latent, with malloc
    int16_t *baseCdfIndex = NULL;                    // cdf index for each dim of flattened latent, for base model, with malloc
    int32_t *baseQuantizedLatent = NULL;             // base model quantized latent, with malloc
    float *baseDequantizedLatent = NULL;             // base model dequantized latent, with malloc
    float *baseDecOutput = NULL;                     // base dec model output buffer, no malloc
    int16_t baseDecDim;                              // base dec model output dim
    int16_t baseDecChannel;                          // base dec model output channels

    // get related handle for base model
    ModelStructHandle baseDecHandle = baseCodecSt->decoderHandle;
    QuantizerHandle baseQuantizerHandle = baseCodecSt->quantizerHandle;
    RangeCoderConfigHandle baseRcHandle = baseCodecSt->rangeCoderConfig;
    // get related dims for base model
    baseNumLatentEncode = baseCodecSt->numLatentEncode;
    baseNumLatentChannels = baseCodecSt->numLatentChannels;
    baseLatentSize = baseNumLatentEncode * baseNumLatentChannels;

    // Get related handle of qc data
    uint8_t *baseBitstream = neuralQcData->baseBitstream;
    int16_t baseNumBytes = neuralQcData->baseNumBytes;
    uint8_t *contextBitstream = neuralQcData->contextBitstream;
    int16_t contextNumBytes = neuralQcData->contextNumBytes;

    // number of latent for noise filling
    int16_t numLatentNF = 0;

    // Decode context bistream
    ContextDec(contextCodecSt, contextBitstream, contextNumBytes);

    // set context dec output to the output buffer of last cnnlayer in context dec model
    ModelStructHandle ctxDecHandle = contextCodecSt->decoderHandle;
    ctxDecOutput = ctxDecHandle->cnnLayers[ctxDecHandle->numLayers - 1]->featOut;
    
    // malloc flatten latent for base model
    baseFlattenLatent = (int32_t *)malloc(sizeof(int32_t) * baseLatentSize);

    // malloc cdf index for base model
    // set cdf index according to ctx model output and context scale table
    baseCdfIndex = (int16_t *)malloc(sizeof(int16_t) * baseLatentSize);
    for (int16_t i = 0; i < baseNumLatentEncode; i++) {
        for (int16_t j = 0; j < baseNumLatentChannels; j++) {
            int16_t index;
            for (index = 0; index < baseCodecSt->numContextScale; index++) {
                if (baseCodecSt->contextScale[index] >= ctxDecOutput[i + j * baseNumLatentEncode]) {
                    baseCdfIndex[i * baseNumLatentChannels + j] = index;
                    break;
                }
            }
            if (index == baseCodecSt->numContextScale) {
                baseCdfIndex[i * baseCodecSt->numLatentChannels + j] = baseCodecSt->numContextScale - 1;
            }
        }
    }

    // perform range decoding
    RangeDecodeProcess(baseRcHandle, baseFlattenLatent, baseLatentSize, baseCdfIndex, baseBitstream, baseNumBytes);

    // malloc and set quantized latent of base model
    baseQuantizedLatent = (int32_t *)malloc(sizeof(int32_t) * baseNumLatentEncode * baseNumLatentChannels);
    for (int16_t i = 0; i < baseNumLatentEncode; i++) {
        for (int16_t j = 0; j < baseNumLatentChannels; j++) {
            baseQuantizedLatent[i + j * baseNumLatentEncode] = baseFlattenLatent[i * baseNumLatentChannels + j];
        }
    }

    // malloc dequantized latent for base model
    baseDequantizedLatent = (float *)malloc(sizeof(float) * baseNumLatentEncode * baseNumLatentChannels);

    // perform dequantization
    LatentDequantize(baseQuantizerHandle, baseQuantizedLatent, baseDequantizedLatent,
        baseNumLatentEncode, baseNumLatentChannels);

    // get number of latent to perform noise filling
    numLatentNF = numLinesNoiseFill;
    for (int16_t i = 0; i < baseDecHandle->numLayers; i++) {
        numLatentNF /= baseDecHandle->cnnLayers[i]->stride;
    }
    // perform noise filling
    LatentNoiseFilling(baseDequantizedLatent, baseQuantizerHandle->quantileMedian, baseNumLatentEncode,
        baseNumLatentChannels, numLatentNF, numGroups, groupIndicator, neuralQcData->nfParam, neuralQcData->nfParamQIdx);

    // dequantization of feature scale
    if (neuralQcData->isFeatAmplified == 0) {
        neuralQcData->featureScale = (float)(neuralQcData->scaleQIdx) / 127.0f;
    }
    else {
        neuralQcData->featureScale = (float)pow(10.0f, (float)(neuralQcData->scaleQIdx) / 86.0f);
    }

    if (neuralQcData->featureScale == 0.0f) {
        neuralQcData->featureScale = 1.0f;
    }

    // inverse feature scaling
    for (int16_t i = 0; i < baseNumLatentEncode * baseNumLatentChannels; i++) {
        baseDequantizedLatent[i] /= neuralQcData->featureScale;
    }

    // Loop for base model decoder cnn layers
    for (int16_t layerIdx = 0; layerIdx < baseDecHandle->numLayers; layerIdx++) {

        // cnn layer handle
        CnnStructHandle cnnLayer;
        cnnLayer = baseDecHandle->cnnLayers[layerIdx];

        // last cnn layer handle
        CnnStructHandle cnnLayerLast = NULL;
        if (layerIdx != 0) {
            cnnLayerLast = baseDecHandle->cnnLayers[layerIdx - 1];
        }

        // perform transpose cnn convolution
        if (layerIdx == 0) {
            if (cnnLayer->stride == 2) {
                Conv1DTranspose2Part(cnnLayer, baseDequantizedLatent);
            }
            else {
                Conv1DTranspose(cnnLayer, baseDequantizedLatent);
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

    // copy last layer output to featureDec
    baseDecOutput = baseDecHandle->cnnLayers[baseDecHandle->numLayers - 1]->featOut;
    baseDecDim = baseDecHandle->cnnLayers[baseDecHandle->numLayers - 1]->featDimOut;
    baseDecChannel = baseDecHandle->cnnLayers[baseDecHandle->numLayers - 1]->numChannelsOut;
    for (int16_t i = 0; i < baseDecDim; i++) {
        for (int16_t j = 0; j < baseDecChannel; j++) {
            featureDec[i][j] = baseDecOutput[i + j * baseDecDim];
        }
    }

    // free memory
    free(baseFlattenLatent);
    baseFlattenLatent = NULL;

    free(baseCdfIndex);
    baseCdfIndex = NULL;

    free(baseQuantizedLatent);
    baseQuantizedLatent = NULL;

    free(baseDequantizedLatent);
    baseDequantizedLatent = NULL;

    return 0;
}


/*
MDCT dequantization and decode, LC profile
includes RC, dequantization and synthesis transform
I/O params:
    NeuralCodecHandle baseCodecSt                   (i) top level struct handle for base codec
    NeuralCodecHandle contextCodecSt                (i) top level struct handle for context codec
    NeuralQcData *neuralQcData                      (i) neural Q/C module data structure, including bs, feature scale and NF params
    float featureDec[][2]                           (o) decoded features
    int16_t numLinesNoiseFill                       (i) number of mdct lines for noise filling
    int16_t numGroups                               (i) number of groups for current frame
    int16_t *groupIndicator                         (i) group indicator vector, 0 for transient, 1 for others
*/
int16_t MdctDequantDecodeHyperLc(
    NeuralCodecHandle baseCodecSt,
    NeuralCodecHandle contextCodecSt,
    NeuralQcData *neuralQcData,
    float featureDec[][2],
    int16_t numLinesNoiseFill,
    int16_t numGroups,
    int16_t *groupIndicator
)
{
    float *ctxDecOutput = NULL;                      // context dec model output buffer, no malloc

    int16_t baseNumLatentEncode;                     // latent dim for base model
    int16_t baseNumLatentChannels;                   // latent num channels for base model
    int16_t baseLatentSize;                          // latent size for base model
    int32_t *baseFlattenLatent = NULL;               // base model flattened latent, with malloc
    int16_t *baseCdfIndex = NULL;                    // cdf index for each dim of flattened latent, for base model, with malloc
    int32_t *baseQuantizedLatent = NULL;             // base model quantized latent, with malloc
    float *baseDequantizedLatent = NULL;             // base model dequantized latent, with malloc
    float *baseDecOutput = NULL;                     // base dec model output buffer, no malloc

    // get related handle for base model
    ModelStructHandle baseDecHandle = baseCodecSt->decoderHandle;
    QuantizerHandle baseQuantizerHandle = baseCodecSt->quantizerHandle;
    RangeCoderConfigHandle baseRcHandle = baseCodecSt->rangeCoderConfig;
    // get related dims for base model
    baseNumLatentEncode = baseCodecSt->numLatentEncode;
    baseNumLatentChannels = baseCodecSt->numLatentChannels;
    baseLatentSize = baseNumLatentEncode * baseNumLatentChannels;

    // Get related handle of qc data
    uint8_t *baseBitstream = neuralQcData->baseBitstream;
    int16_t baseNumBytes = neuralQcData->baseNumBytes;
    uint8_t *contextBitstream = neuralQcData->contextBitstream;
    int16_t contextNumBytes = neuralQcData->contextNumBytes;

    // number of latent for noise filling
    int16_t numLatentNF = 0;

    // Decode context bistream
    ContextDec(contextCodecSt, contextBitstream, contextNumBytes);

    // set context dec output to the output buffer of last cnnlayer in context dec model
    ModelStructHandle ctxDecHandle = contextCodecSt->decoderHandle;
    ctxDecOutput = ctxDecHandle->cnnLayers[ctxDecHandle->numLayers - 1]->featOut;

    // malloc flatten latent for base model
    baseFlattenLatent = (int32_t *)malloc(sizeof(int32_t) * baseLatentSize);

    // malloc cdf index for base model
    // set cdf index according to ctx model output and context scale table
    baseCdfIndex = (int16_t *)malloc(sizeof(int16_t) * baseLatentSize);
    for (int16_t i = 0; i < baseNumLatentEncode; i++) {
        for (int16_t j = 0; j < baseNumLatentChannels; j++) {
            int16_t index;
            for (index = 0; index < baseCodecSt->numContextScale; index++) {
                if (baseCodecSt->contextScale[index] >= ctxDecOutput[i + j * baseNumLatentEncode]) {
                    baseCdfIndex[i * baseNumLatentChannels + j] = index;
                    break;
                }
            }
            if (index == baseCodecSt->numContextScale) {
                baseCdfIndex[i * baseCodecSt->numLatentChannels + j] = baseCodecSt->numContextScale - 1;
            }
        }
    }

    // perform range decoding
    RangeDecodeProcess(baseRcHandle, baseFlattenLatent, baseLatentSize, baseCdfIndex, baseBitstream, baseNumBytes);

    // malloc and set quantized latent of base model
    baseQuantizedLatent = (int32_t *)malloc(sizeof(int32_t) * baseNumLatentEncode * baseNumLatentChannels);
    for (int16_t i = 0; i < baseNumLatentEncode; i++) {
        for (int16_t j = 0; j < baseNumLatentChannels; j++) {
            baseQuantizedLatent[i + j * baseNumLatentEncode] = baseFlattenLatent[i * baseNumLatentChannels + j];
        }
    }

    // malloc dequantized latent for base model
    baseDequantizedLatent = (float *)malloc(sizeof(float) * baseNumLatentEncode * baseNumLatentChannels);

    // perform dequantization
    LatentDequantize(baseQuantizerHandle, baseQuantizedLatent, baseDequantizedLatent,
        baseNumLatentEncode, baseNumLatentChannels);

    // get number of latent to perform noise filling
    numLatentNF = numLinesNoiseFill / baseCodecSt->numLatentChannels;
    // perform noise filling
    LatentNoiseFilling(baseDequantizedLatent, baseQuantizerHandle->quantileMedian, baseNumLatentEncode,
        baseNumLatentChannels, numLatentNF, numGroups, groupIndicator, neuralQcData->nfParam, neuralQcData->nfParamQIdx);

    // dequantization of feature scale
    neuralQcData->featureScale = (float)pow(10.0f, ((float)(neuralQcData->scaleQIdx) - 255.0f) / 31.875f);

    if (neuralQcData->featureScale == 0.0f) {
        neuralQcData->featureScale = 1.0f;
    }

    // inverse feature scaling
    for (int16_t i = 0; i < baseNumLatentEncode; i++) {
        for (int16_t j = 0; j < baseNumLatentChannels; j++) {
            featureDec[i * baseNumLatentChannels + j][0] = baseDequantizedLatent[i + j * baseNumLatentEncode] /
                neuralQcData->featureScale;
        }
    }

    // free memory
    free(baseFlattenLatent);
    baseFlattenLatent = NULL;

    free(baseCdfIndex);
    baseCdfIndex = NULL;

    free(baseQuantizedLatent);
    baseQuantizedLatent = NULL;

    free(baseDequantizedLatent);
    baseDequantizedLatent = NULL;

    return 0;
}
