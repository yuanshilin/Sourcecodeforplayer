
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "avs3_options.h"
#include "avs3_stat_com.h"


/*
Init quantizer structure
I/O params:
    FILE *fModel                        (i)   model file handle
    QuantizerHandle quantizerHandle     (i/o) quantizer handle
    int16_t numChannels                 (i)   channel number for latent
*/
int16_t InitQuantizer(
    modul_structure *fModel,
    QuantizerHandle quantizerHandle,
    int16_t numChannels
)
{
    float tmp;

    // get number of feature channels for quantization
    quantizerHandle->numChannels = numChannels;

    // get quantile medians
    quantizerHandle->quantileMedian = (float *)malloc(sizeof(float) * quantizerHandle->numChannels);
    if (quantizerHandle->quantileMedian == NULL) {
		LOGD("Malloc quantile median error!\n");
        exit(-1);
    }
    for (int i = 0; i < quantizerHandle->numChannels; i++) {
//        fread(&tmp, sizeof(float), 1, fModel);
		memcpy(&tmp, fModel->data + fModel->nIndex, sizeof(float));
		fModel->nIndex += sizeof(float);
        quantizerHandle->quantileMedian[i] = tmp;
    }

    return 0;
}


/*
Latent parameter quantization
I/O params:
    QuantizerHandle quantizerHandle         (i) quantizer handle, include quantization offset
    float *featureIn                        (i) input feature map, size: featureDim * numChannels
    int32_t *featureOut                     (o) output feature map, size: featureDime * numChannels
    int16_t featureDim                      (i) feature dim
    int16_t numChannels                     (i) number channels
*/
int16_t LatentQuantize(
    QuantizerHandle quantizerHandle,
    float *featureIn,
    int32_t *featureOut,
    int16_t featureDim,
    int16_t numChannels
)
{
    float tmp;
    float half = 0.5f;

    // check number of channels
    if (numChannels != quantizerHandle->numChannels) {
		LOGD("The channel number of input feature does not match quantizer's numChannels!!\n");
        exit(-1);
    }

    // loop over each dim
    for (int16_t i = 0; i < featureDim; i++) {
        for (int16_t j = 0; j < numChannels; j++) {
            tmp = featureIn[i + j * featureDim] + half - quantizerHandle->quantileMedian[j];
            featureOut[i + j * featureDim] = (int32_t)(floor(tmp));
        }
    }

    return 0;
}


/*
Latent parameter dequantization
I/O params:
    QuantizerHandle quantizerHandle         (i) quantizer handle
    int32_t *featureIn                      (i) input feature map
    float *featureOut                       (o) output feature map
    int16_t featureDim                      (i) feature dim
    int16_t numChannels                     (i) number channels
*/
int16_t LatentDequantize(
    QuantizerHandle quantizerHandle,
    int32_t *featureIn,
    float *featureOut,
    int16_t featureDim,
    int16_t numChannels
)
{
    // check number of channels
    if (numChannels != quantizerHandle->numChannels) {
		LOGD("The channel number of input feature does not match quantizer's numChannels!!\n");
        exit(-1);
    }

    // loop over each dim
    for (int16_t i = 0; i < featureDim; i++) {
        for (int16_t j = 0; j < numChannels; j++) {
            featureOut[i + j * featureDim] = (float)featureIn[i + j * featureDim] + quantizerHandle->quantileMedian[j];
        }
    }

    return 0;
}


int16_t DestroyQuantizer(
    QuantizerHandle quantizerHandle
)
{
    free(quantizerHandle->quantileMedian);
    quantizerHandle->quantileMedian = NULL;

    return 0;
}
