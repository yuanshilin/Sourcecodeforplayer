
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "avs3_options.h"
#include "avs3_stat_com.h"
#include "avs3_prot_com.h"

// #ifdef AVS_NEON_ON
// #include <arm_neon.h>
// #endif


// Static function declaration
static void KernelToCol1D(float ***kernel, float *kernelCol, int16_t kernelSize,
    int16_t numChannelsIn, int16_t numChannelsOut);

static void KernelToCol1DTranspose(float ***kernel, float *kernelCol, int16_t kernelSize,
    int16_t numChannelsIn, int16_t numChannelsOut);

static void KernelToCol1DTransposeOdd(float ***kernel, float *kernelCol, int16_t kernelSize,
    int16_t numChannelsIn, int16_t numChannelsOut);

static void KernelToCol1DTransposeEven(float ***kernel, float *kernelCol, int16_t kernelSize,
    int16_t numChannelsIn, int16_t numChannelsOut);


/*
Malloc runtime buffers for cnn layer
Note: for conv1D and conv1DTranspose, size of buffers are different
I/O params:
    CnnStructHandle cnnLayer    (i/o) CNN layer structure
*/
void CnnMallocRuntimeBuffer(
    CnnStructHandle cnnLayer
)
{
    int16_t paddingSize;
    int16_t featDimPadded;
    int16_t featDimInterPolated;

    if (cnnLayer->isTranspose == 0) {

        // get total padding size
        paddingSize = (cnnLayer->featDimOut - 1) * cnnLayer->stride + cnnLayer->kernelSize - cnnLayer->featDimIn;
        // get padded feat dim
        featDimPadded = cnnLayer->featDimIn + paddingSize;

        // init InterPolated feature to NULL
        cnnLayer->featureInterPolated = NULL;

        // malloc padded feature
        cnnLayer->featurePadded = (float *)malloc(sizeof(float) * featDimPadded * cnnLayer->numChannelsIn);

        // malloc columned feature
        cnnLayer->featureCol = (float *)malloc(sizeof(float) * (cnnLayer->featDimIn / cnnLayer->stride) *
            (cnnLayer->kernelSize * cnnLayer->numChannelsIn));

        cnnLayer->kernelCol = (float *)malloc(sizeof(float) * (cnnLayer->kernelSize * cnnLayer->numChannelsIn) *
            cnnLayer->numChannelsOut);
    }
    else if (cnnLayer->isTranspose == 1) {

        // Condition 1: stride is 1
        // use conventional conv1DTranspose, 4 buffers
        // interpolated, padded, featureCol, kernelCol
        if (cnnLayer->stride == 1) {
            // get InterPolated feat dim
            featDimInterPolated = cnnLayer->featDimIn * cnnLayer->stride;
            // get total padding size
            paddingSize = (cnnLayer->featDimOut - 1) + cnnLayer->kernelSize - featDimInterPolated;
            // get padded feat dim
            featDimPadded = featDimInterPolated + paddingSize;

            // malloc InterPolated feature
            cnnLayer->featureInterPolated = (float *)malloc(sizeof(float) * featDimInterPolated * cnnLayer->numChannelsIn);

            // malloc padded feature
            cnnLayer->featurePadded = (float *)malloc(sizeof(float) * featDimPadded * cnnLayer->numChannelsIn);

            // malloc columned feature
            cnnLayer->featureCol = (float *)malloc(sizeof(float) * (cnnLayer->featDimIn * cnnLayer->stride) *
                (cnnLayer->kernelSize * cnnLayer->numChannelsIn));

            // malloc columned kernel
            cnnLayer->kernelCol = (float *)malloc(sizeof(float) * (cnnLayer->kernelSize * cnnLayer->numChannelsIn) *
                cnnLayer->numChannelsOut);
        }

        // Condition 2: stride is 2
        // use two part conv1DTranspose (even / odd), 7 buffers
        if (cnnLayer->stride == 2) {
            // get total padding size
            paddingSize = 2;                                        // 2, fixed padding size
            // get padded feat dim
            featDimPadded = cnnLayer->featDimIn + paddingSize;

            // malloc padded feature
            cnnLayer->featurePadded = (float *)malloc(sizeof(float) * featDimPadded * cnnLayer->numChannelsIn);

            // Odd/even kernelCol
            cnnLayer->kernelColOdd = (float *)malloc(sizeof(float) * (cnnLayer->kernelSize + 1) / 2 *
                cnnLayer->numChannelsIn * cnnLayer->numChannelsOut);

            cnnLayer->kernelColEven = (float *)malloc(sizeof(float) * (cnnLayer->kernelSize - 1) / 2 *
                cnnLayer->numChannelsIn * cnnLayer->numChannelsOut);

            // Odd/even featureCol
            cnnLayer->featureColOdd = (float *)malloc(sizeof(float) * cnnLayer->featDimIn *
                (cnnLayer->kernelSize + 1) / 2 * cnnLayer->numChannelsIn);

            cnnLayer->featureColEven = (float *)malloc(sizeof(float) * cnnLayer->featDimIn *
                (cnnLayer->kernelSize - 1) / 2 * cnnLayer->numChannelsIn);

            // Odd/even featOut, tmp buffer not used for next layer
            cnnLayer->featOutOdd = (float *)malloc(sizeof(float) * cnnLayer->featDimIn * cnnLayer->numChannelsOut);

            cnnLayer->featOutEven = (float *)malloc(sizeof(float) * cnnLayer->featDimIn * cnnLayer->numChannelsOut);
        }
    }
}


/*
Free runtime buffers for cnn layer
Note: for conv1D and conv1DTranspose, size of buffers are different
I/O params:
    CnnStructHandle cnnLayer    (i/o) CNN layer structure
*/
void CnnFreeRuntimeBuffer(
    CnnStructHandle cnnLayer
)
{
    int16_t paddingSize;
    int16_t featDimPadded;
    int16_t featDimInterPolated;

    if (cnnLayer->isTranspose == 0) {

        // get total padding size
        paddingSize = (cnnLayer->featDimOut - 1) * cnnLayer->stride + cnnLayer->kernelSize - cnnLayer->featDimIn;
        // get padded feat dim
        featDimPadded = cnnLayer->featDimIn + paddingSize;

        // free memory
        free(cnnLayer->featurePadded);
        cnnLayer->featurePadded = NULL;

        free(cnnLayer->featureCol);
        cnnLayer->featureCol = NULL;

        free(cnnLayer->kernelCol);
        cnnLayer->kernelCol = NULL;
    }
    else if (cnnLayer->isTranspose == 1) {

        if (cnnLayer->stride == 1) {
            // get InterPolated feat dim
            featDimInterPolated = cnnLayer->featDimIn * cnnLayer->stride;
            // get total padding size
            paddingSize = (cnnLayer->featDimOut - 1) + cnnLayer->kernelSize - featDimInterPolated;
            // get padded feat dim
            featDimPadded = featDimInterPolated + paddingSize;

            // free memory
            free(cnnLayer->featureInterPolated);
            cnnLayer->featureInterPolated = NULL;

            free(cnnLayer->featurePadded);
            cnnLayer->featurePadded = NULL;

            free(cnnLayer->featureCol);
            cnnLayer->featureCol = NULL;

            free(cnnLayer->kernelCol);
            cnnLayer->kernelCol = NULL;
        }

        if (cnnLayer->stride == 2) {
            // get total padding size
            paddingSize = 2;                                        // 2, fixed padding size
            // get padded feat dim
            featDimPadded = cnnLayer->featDimIn + paddingSize;

            free(cnnLayer->featurePadded);
            cnnLayer->featurePadded = NULL;

            free(cnnLayer->kernelColOdd);
            cnnLayer->kernelColOdd = NULL;

            free(cnnLayer->kernelColEven);
            cnnLayer->kernelColEven = NULL;

            free(cnnLayer->featureColOdd);
            cnnLayer->featureColOdd = NULL;

            free(cnnLayer->featureColEven);
            cnnLayer->featureColEven = NULL;

            free(cnnLayer->featOutOdd);
            cnnLayer->featOutOdd = NULL;

            free(cnnLayer->featOutEven);
            cnnLayer->featOutEven = NULL;
        }
    }
}


/*
Init CNN layer structure
I/O params:
    FILE *fModel                (i) model file
    CnnStructHandle cnnLayer    (o) CNN layer structure
    int16_t isTranspose         (i) flag for conv transpose
*/
int16_t InitCnnLayer(
    modul_structure *fModel,
    CnnStructHandle cnnLayer,
    int16_t isTranspose, 
    int16_t featDimIn
)
{
    int16_t padding;
    int16_t stride;
    int16_t useBias;
    int16_t activationFunc;
    int16_t kernelSize;
    int16_t numChannelsIn;
    int16_t numChannelsOut;
    float kernelCoef;
    float biasCoef;

    // set isTranspose flag
    cnnLayer->isTranspose = isTranspose;

    // get padding info
//    fread(&padding, sizeof(int16_t), 1, fModel);
	memcpy(&padding, fModel->data + fModel->nIndex, sizeof(int16_t));
	fModel->nIndex += sizeof(int16_t);

    if (padding == 0) {
        cnnLayer->padding = SAME;
    }
    else if (padding == 1) {
        cnnLayer->padding = VALID;
    }
    //fprintf(stdout, "Layer padding type: %d\n", cnnLayer->padding);

    // get stride
//    fread(&stride, sizeof(int16_t), 1, fModel);
	memcpy(&stride, fModel->data + fModel->nIndex, sizeof(int16_t));
	fModel->nIndex += sizeof(int16_t);
	cnnLayer->stride = stride;
    //fprintf(stdout, "Layer stride: %d\n", cnnLayer->stride);

    // get useBias
//    fread(&useBias, sizeof(int16_t), 1, fModel);
	memcpy(&useBias, fModel->data + fModel->nIndex, sizeof(int16_t));
	fModel->nIndex += sizeof(int16_t);
	cnnLayer->useBias = useBias;
    //fprintf(stdout, "Layer useBias: %d\n", cnnLayer->useBias);

    // get activation function
//    fread(&activationFunc, sizeof(int16_t), 1, fModel);
	memcpy(&activationFunc, fModel->data + fModel->nIndex, sizeof(int16_t));
	fModel->nIndex += sizeof(int16_t);
	cnnLayer->activationFunc = (TypeActFunc)activationFunc;
    //fprintf(stdout, "Layer activationFunc: %d\n", cnnLayer->activationFunc);

    // get kernel size
//    fread(&kernelSize, sizeof(int16_t), 1, fModel);
	memcpy(&kernelSize, fModel->data + fModel->nIndex, sizeof(int16_t));
	fModel->nIndex += sizeof(int16_t);
	cnnLayer->kernelSize = kernelSize;
    //fprintf(stdout, "Layer kernel size: %d\n", cnnLayer->kernelSize);

    // get num channels in
//    fread(&numChannelsIn, sizeof(int16_t), 1, fModel);
	memcpy(&numChannelsIn, fModel->data + fModel->nIndex, sizeof(int16_t));
	fModel->nIndex += sizeof(int16_t);
	cnnLayer->numChannelsIn = numChannelsIn;
    //fprintf(stdout, "Layer number channels in: %d\n", cnnLayer->numChannelsIn);

    // get num channels out
//    fread(&numChannelsOut, sizeof(int16_t), 1, fModel);
	memcpy(&numChannelsOut, fModel->data + fModel->nIndex, sizeof(int16_t));
	fModel->nIndex += sizeof(int16_t);
	cnnLayer->numChannelsOut = numChannelsOut;
    //fprintf(stdout, "Layer number channels out: %d\n", cnnLayer->numChannelsOut);

    // get kernel parameters
    cnnLayer->kernel = NULL;

    int16_t chNum1, chNum2;
    if (cnnLayer->isTranspose == 0) {
        // conv, kernelSize * numChannelsIn * numChannelsOut
        chNum1 = cnnLayer->numChannelsIn;
        chNum2 = cnnLayer->numChannelsOut;
    }
    else {
        // conv transpose, kernelSize * numChannelsOut * numChannelsIn
        chNum1 = cnnLayer->numChannelsOut;
        chNum2 = cnnLayer->numChannelsIn;
    }

    // memory malloc
    cnnLayer->kernel = (float ***)malloc(sizeof(float**) * cnnLayer->kernelSize);
    for (int16_t i = 0; i < cnnLayer->kernelSize; i++) {
        cnnLayer->kernel[i] = (float **)malloc(sizeof(float *) * chNum1);
        for (int16_t j = 0; j < chNum1; j++) {
            cnnLayer->kernel[i][j] = (float *)malloc(sizeof(float) * chNum2);
        }
    }

    // read coefficients from file
    // dim 1: kernel size
    for (int16_t i = 0; i < cnnLayer->kernelSize; i++) {
        // dim 2: num channels input
        for (int16_t j = 0; j < chNum1; j++) {
            // dim 3: num channels output
            for (int16_t k = 0; k < chNum2; k++) {
//                fread(&kernelCoef, sizeof(float), 1, fModel);
				memcpy(&kernelCoef, fModel->data + fModel->nIndex, sizeof(float));
				fModel->nIndex += sizeof(float);
				cnnLayer->kernel[i][j][k] = kernelCoef;
            }
        }
    }

    // get bias parameters
    cnnLayer->bias = NULL;
    if (cnnLayer->useBias == 1) {
        cnnLayer->bias = (float *)malloc(sizeof(float) * cnnLayer->numChannelsOut);
        for (int16_t i = 0; i < cnnLayer->numChannelsOut; i++) {
//            fread(&biasCoef, sizeof(float), 1, fModel);
			memcpy(&biasCoef, fModel->data + fModel->nIndex, sizeof(float));
			fModel->nIndex += sizeof(float);
			cnnLayer->bias[i] = biasCoef;
        }
    }

    // get GDN/DN related parameters
    // TBD: DN parameters
    cnnLayer->gdnActFuncParam = NULL;
    if (cnnLayer->activationFunc == GDN || cnnLayer->activationFunc == IGDN) {

        cnnLayer->gdnActFuncParam = (GdnActFuncHandle)malloc(sizeof(GdnActFuncStruct));
        if (cnnLayer->gdnActFuncParam == NULL) {
			LOGD("Error in malloc GdnActFuncStruct in initCnnLayer func!!\n");
            exit(-1);
        }

        InitGdnParam(fModel, cnnLayer->gdnActFuncParam, cnnLayer->numChannelsOut);
    }

    // get in/output feature dim
    cnnLayer->featDimIn = featDimIn;
    if (cnnLayer->isTranspose == 0) {
        cnnLayer->featDimOut = featDimIn / cnnLayer->stride;
    }
    else {
        cnnLayer->featDimOut = featDimIn * cnnLayer->stride;
    }

    // malloc output feature buffer
    cnnLayer->featOut = (float *)malloc(sizeof(float) * cnnLayer->featDimOut * cnnLayer->numChannelsOut);

    // malloc runtime buffer for cnn
    CnnMallocRuntimeBuffer(cnnLayer);

    // Perform Kernel2Col at init stage
    if (cnnLayer->isTranspose == 0) {
        KernelToCol1D(cnnLayer->kernel, cnnLayer->kernelCol, cnnLayer->kernelSize,
            cnnLayer->numChannelsIn, cnnLayer->numChannelsOut);
    }
    else {
        // Condition 1: stride is 1
        // use conventional kernel to col transform
        if (cnnLayer->stride == 1) {
            KernelToCol1DTranspose(cnnLayer->kernel, cnnLayer->kernelCol, cnnLayer->kernelSize,
                cnnLayer->numChannelsIn, cnnLayer->numChannelsOut);
        }

        // Condition 2: stride is 2
        // use two part kernel to col transform
        if (cnnLayer->stride == 2) {
            KernelToCol1DTransposeOdd(cnnLayer->kernel, cnnLayer->kernelColOdd, cnnLayer->kernelSize,
                cnnLayer->numChannelsIn, cnnLayer->numChannelsOut);

            KernelToCol1DTransposeEven(cnnLayer->kernel, cnnLayer->kernelColEven, cnnLayer->kernelSize,
                cnnLayer->numChannelsIn, cnnLayer->numChannelsOut);
        }
    }

    return 0;
}


/*
InterPolating for 1D feature
insert (stride-1) zeros before each dim of feature
I/O params:
    float *featureIn                (i) input feature map, size: featDimInterPolated/stride * numChannelsIn
    float *featureInterPolated      (o) interpolated feature map, size: featDimInterPolated * numChannelsIn
    int16_t featDimInterPolated     (i) feature dim after interpolation
    int16_t stride                  (i) stride
    int16_t numChannelsIn           (i) number of input channels
*/
static void InterPolating1D(
    float *featureIn,
    float *featureInterPolated,
    int16_t featDimInterPolated,
    int16_t stride,
    int16_t numChannelsIn
)
{

    // set InterPolated  feature to zero
    SetFloat(featureInterPolated, 0.0f, featDimInterPolated * numChannelsIn);

    // copy input feature to InterPolated feature
    for (int16_t i = 0; i < (featDimInterPolated / stride) * numChannelsIn; i++) {
        featureInterPolated[stride * (i + 1) - 1] = featureIn[i];
    }

    return;
}

/*
1D padding function for 'SAME'
I/O params:
    float *featureIn                (i) input feature map, size: featDimIn * numChannelsIn
    float *featurePadded            (o) padded feature map, size: (featDimIn + paddingSizeBegin + paddingSizeEnd) * numChannelsIn
    int16_t featDimIn               (i) input feature dim
    int16_t paddingSizeBegin        (i) padding size at the beginning
    int16_t paddingSizeEnd          (i) padding size at the end
    int16_t numChannelsIn           (i) channel of input feature
*/
static void PaddingSame1D(
    float *featureIn,
    float *featurePadded,
    int16_t featDimIn,
    int16_t paddingSizeBegin,
    int16_t paddingSizeEnd,
    int16_t numChannelsIn
)
{
    int16_t featDimPadded;

    featDimPadded = featDimIn + paddingSizeBegin + paddingSizeEnd;

    // set padded feature to zero
    SetFloat(featurePadded, 0.0f, featDimPadded * numChannelsIn);

    // copy input feature to padded feature
    for (int16_t j = 0; j < numChannelsIn; j++) {
        for (int16_t i = paddingSizeBegin; i < featDimIn + paddingSizeBegin; i++) {
            featurePadded[j * featDimPadded + i] = featureIn[j * featDimIn + i - paddingSizeBegin];
        }
    }

    return;
}


/*
Feature to col conversion function for 1D
I/O params:
    float *featurePadded            (i) padded feature map, size: (featDimIn + paddingSize) * numChannelsIn
    float *featureCol               (o) columned feature map, size: (featDimIn/stride) * (kernelSize * numChannelsIn)
    int16_t featDimIn               (i) input feature dim
    int16_t kernelSize              (i) kernel size
    int16_t numChannelsIn           (i) number of input channels
    int16_t stride                  (i) stride
*/
static void FeatureToCol1D(
    float *featurePadded,
    float *featureCol,
    int16_t featDimIn,
    int16_t kernelSize,
    int16_t numChannelsIn,
    int16_t stride, 
    int16_t sizeAfterPadding
)
{
    int16_t temp1, temp2, temp3;
    int16_t featDimTemp = featDimIn / stride;

    temp1 = numChannelsIn * kernelSize;
    for (int16_t j = 0; j < numChannelsIn; j++) {
        temp2 = j * kernelSize;
        temp3 = j * sizeAfterPadding;
        for (int16_t k = 0; k < kernelSize; k++) {
            for (int16_t i = 0; i < featDimTemp; i++) {
                featureCol[i * temp1 + k + temp2] = featurePadded[i * stride + k + temp3];
            }
        }
    }

    return;
}


/*
Kernel to col conversion function for 1D
I/O params:
    float ***kernel                 (i) kernel param, size: kernelSize * numChannelsIn * numChannelsOut
    float *kernelCol                (o) columned kernel param, (kernelSize * numChannelsIn) * numChannelsOut
    int16_t kernelSize              (i) kernel size
    int16_t numChannelsIn           (i) number of input channels
    int16_t numChannelsOut          (i) number of output channels
*/
static void KernelToCol1D(
    float ***kernel,
    float *kernelCol,
    int16_t kernelSize,
    int16_t numChannelsIn,
    int16_t numChannelsOut
)
{
    for (int16_t k = 0; k < numChannelsOut; k++) {
        for (int16_t j = 0; j < numChannelsIn; j++) {
            for (int16_t i = 0; i < kernelSize; i++) {
                kernelCol[i + j * kernelSize + k * kernelSize * numChannelsIn] = kernel[i][j][k];
            }
        }
    }

    return;
}


/*
Kernel to col conversion function for 1D transpose conv
I/O params:
    float ***kernel                 (i) kernel param, size: kernelSize * numChannelsIn * numChannelsOut
    float *kernelCol                (o) columned kernel param, (kernelSize * numChannelsIn) * numChannelsOut
    int16_t kernelSize              (i) kernel size
    int16_t numChannelsIn           (i) number of input channels
    int16_t numChannelsOut          (i) number of output channels
Note:
    Difference with KernelToCol1D:
        kernelCol[j * kernelSize + i][k] = kernel[i][j][k] to:
        kernelCol[j * kernelSize + i][k] = kernel[kernelSize - i - 1][k][j]
    Important: Flip channel index and kernel index at the same time. 
*/
static void KernelToCol1DTranspose(
    float ***kernel,
    float *kernelCol,
    int16_t kernelSize,
    int16_t numChannelsIn,
    int16_t numChannelsOut
)
{
    for (int16_t k = 0; k < numChannelsOut; k++) {
        for (int16_t j = 0; j < numChannelsIn; j++) {
            for (int16_t i = 0; i < kernelSize; i++) {
                kernelCol[i + j * kernelSize + k * kernelSize * numChannelsIn] = kernel[kernelSize - i - 1][k][j];
            }
        }
    }

    return;
}


/*
Kernel to col conversion function for 1D transpose conv: Odd part of kernel
I/O params:
    float ***kernel                 (i) kernel param, size: kernelSize * numChannelsIn * numChannelsOut
    float *kernelCol                (o) columned kernel param, (kernelSize * numChannelsIn) * numChannelsOut
    int16_t kernelSize              (i) kernel size
    int16_t numChannelsIn           (i) number of input channels
    int16_t numChannelsOut          (i) number of output channels
Note:
    Difference with KernelToCol1D:
        kernelCol[j * kernelSize + i][k] = kernel[i][j][k] to:
        kernelCol[j * kernelSize + i][k] = kernel[kernelSize - i - 1][k][j]
    Important: Flip channel index and kernel index at the same time.
Difference to KernelToCol1DTranspose:
    i for kernel size, step is 2, start from 0
    kernelCol idx: i to i/2
    for odd part:
      if kernelSize = 3, odd part is 0 and 2
      if kernelSize = 5, odd part is 0, 2 and 4
*/
static void KernelToCol1DTransposeOdd(
    float ***kernel,
    float *kernelCol,
    int16_t kernelSize,
    int16_t numChannelsIn,
    int16_t numChannelsOut
)
{
    int16_t kernelSizeOdd = (kernelSize + 1) / 2;

    for (int16_t k = 0; k < numChannelsOut; k++) {
        for (int16_t j = 0; j < numChannelsIn; j++) {
            for (int16_t i = 0; i < kernelSize; i += 2) {
                kernelCol[i / 2 + j * kernelSizeOdd + k * kernelSizeOdd * numChannelsIn] =
                    kernel[kernelSize - i - 1][k][j];
            }
        }
    }

    return;
}


/*
Kernel to col conversion function for 1D transpose conv: Even part of kernel
I/O params:
    float ***kernel                 (i) kernel param, size: kernelSize * numChannelsIn * numChannelsOut
    float *kernelCol                (o) columned kernel param, (kernelSize * numChannelsIn) * numChannelsOut
    int16_t kernelSize              (i) kernel size
    int16_t numChannelsIn           (i) number of input channels
    int16_t numChannelsOut          (i) number of output channels
Note:
    Difference with KernelToCol1D:
        kernelCol[j * kernelSize + i][k] = kernel[i][j][k] to:
        kernelCol[j * kernelSize + i][k] = kernel[kernelSize - i - 1][k][j]
    Important: Flip channel index and kernel index at the same time.
Difference to KernelToCol1DTranspose:
    i for kernel size, step is 2, start from 1
    kernelCol idx: i to (i-1)/2
    for even part:
      if kernelSize = 3, even part is 1
      if kernelSize = 5, even part is 1, 3
*/
static void KernelToCol1DTransposeEven(
    float ***kernel,
    float *kernelCol,
    int16_t kernelSize,
    int16_t numChannelsIn,
    int16_t numChannelsOut
)
{
    int16_t kernelSizeEven = (kernelSize - 1) / 2;

    for (int16_t k = 0; k < numChannelsOut; k++) {
        for (int16_t j = 0; j < numChannelsIn; j++) {
            for (int16_t i = 1; i < kernelSize; i += 2) {
                kernelCol[(i - 1) / 2 + j * kernelSizeEven + k * kernelSizeEven * numChannelsIn] =
                    kernel[kernelSize - i - 1][k][j];
            }
        }
    }

    return;
}


/*
Add bias to feature map
I/O params:
    float *feature              (i/o) feature map, in place add, size: featureDime * numChannels
    float *bias                 (i)   bias parameter, size: numChannels
    int16_t featureDim          (i)   feature dim
    int16_t numChannels         (i)   number of channels
*/
static void AddBias(
    float *feature,
    float *bias,
    int16_t featureDim,
    int16_t numChannels
)
{
    for (int16_t j = 0; j < numChannels; j++) {
        for (int16_t i = 0; i < featureDim; i++) {
            feature[i + j * featureDim] += bias[j];
        }
    }

    return;
}


/*
1D convolution with stride and activation function
I/O params:
    CnnStructHandle cnnLayer        (i/o) cnn layer structure, include output feature buffer
    float *featureIn                (i)   input feature map, size: cnnLayer->featDimIn * cnnLayer->numChannelsIn
*/
int16_t Conv1D(
    CnnStructHandle cnnLayer,
    float *featureIn
)
{
    int16_t paddingSize;
    int16_t paddingSizeBegin, paddingSizeEnd;
    int16_t featDimPadded;
    float *featurePadded = NULL;
    float *featureCol = NULL;

    // for GEMM algorithm
    MatrixStruct matA, matB;
    MatrixStructUnconst matM;

    // padding
    if (cnnLayer->padding == VALID) {
        featurePadded = featureIn;
    }
    else if (cnnLayer->padding == SAME) {

        // get total padding size
        paddingSize = (cnnLayer->featDimOut - 1) * cnnLayer->stride + cnnLayer->kernelSize - cnnLayer->featDimIn;
        if (paddingSize < 0) {
			LOGD("Error configuration in Conv1D, paddingSize < 0!!\n");
            exit(-1);
        }
        // get padded feat dim
        featDimPadded = cnnLayer->featDimIn + paddingSize;

        // get padding size at begin and end of feature
        if (paddingSize % 2 != 0) {
            // odd padding size, more padding at the end
            paddingSizeBegin = max((paddingSize - 1) / 2, 0);
            paddingSizeEnd = (paddingSize + 1) / 2;
        }
        else {
            // even padding size, same padding size at both side
            paddingSizeBegin = paddingSize / 2;
            paddingSizeEnd = paddingSize / 2;
        }

        // malloc padded feature
        featurePadded = cnnLayer->featurePadded;
        // perform padding
        PaddingSame1D(featureIn, featurePadded, cnnLayer->featDimIn, paddingSizeBegin, paddingSizeEnd, cnnLayer->numChannelsIn);
    }

    // feature to col
    if (cnnLayer->padding == SAME) {
        // malloc columned feature
        featureCol = cnnLayer->featureCol;
        // perform feature to col
        FeatureToCol1D(featurePadded, featureCol, cnnLayer->featDimIn,
            cnnLayer->kernelSize, cnnLayer->numChannelsIn, cnnLayer->stride,
            featDimPadded);
    }

    // perform convolution by matrix mult
    // using GEMM algorithm
    matA.data = cnnLayer->kernelCol;
    matA.row = cnnLayer->numChannelsOut;
    matA.col = cnnLayer->numChannelsIn * cnnLayer->kernelSize;
    matB.data = featureCol;
    matB.row = cnnLayer->featDimIn / cnnLayer->stride;
    matB.col = cnnLayer->numChannelsIn * cnnLayer->kernelSize;
    matM.data = cnnLayer->featOut;
    matM.row = cnnLayer->numChannelsOut;
    matM.col = cnnLayer->featDimIn / cnnLayer->stride;

    SetFloat(cnnLayer->featOut, 0.0f, (short)(matM.row * matM.col));

    // MatrixMultGemm(&matA, &matB, &matM);
#if defined(_AVX2) && defined(SUPPORT_AVX2)
    MatrixMultGemmAvx2(&matA, &matB, &matM);
#elif defined(_AVX512) && defined(SUPPORT_AVX512)
    MatrixMultGemmAvx512(&matA, &matB, &matM);
#elif defined(_NEON) && defined(SUPPORT_NEON)
    MatrixMultGemmNeon(&matA, &matB, &matM);
#else
    MatrixMultGemm(&matA, &matB, &matM);
#endif  

    // add bias to feature
    if (cnnLayer->useBias == 1) {
        AddBias(cnnLayer->featOut, cnnLayer->bias, cnnLayer->featDimOut, cnnLayer->numChannelsOut);
    }

    // apply activation function
    if (cnnLayer->activationFunc == LINEAR) {
        ApplyLinearActFuncVec(cnnLayer->featOut, cnnLayer->featDimOut * cnnLayer->numChannelsOut, cnnLayer->featOut);
    }
    else if (cnnLayer->activationFunc == RELU) {
        ApplyReluActFuncVec(cnnLayer->featOut, cnnLayer->featDimOut * cnnLayer->numChannelsOut, cnnLayer->featOut);
    }
    else if (cnnLayer->activationFunc == GDN) {
        ApplyGdnActFunc(cnnLayer->gdnActFuncParam, cnnLayer->featOut, cnnLayer->featDimOut, cnnLayer->numChannelsOut, cnnLayer->featOut);
    }
    else if (cnnLayer->activationFunc == IGDN) {
        ApplyIgdnActFunc(cnnLayer->gdnActFuncParam, cnnLayer->featOut, cnnLayer->featDimOut, cnnLayer->numChannelsOut, cnnLayer->featOut);
    }

    return 0;
}


/*
1D transpose convolution with stride and activation function
I/O params:
    CnnStructHandle cnnLayer        (i/o) cnn layer structure, include output feature buffer
    float *featureIn                (i)   input feature map, size: cnnLayer->featDimIn * cnnLayer->numChannelsIn
*/
int16_t Conv1DTranspose(
    CnnStructHandle cnnLayer,
    float *featureIn
)
{
    int16_t paddingSize;
    int16_t paddingSizeBegin, paddingSizeEnd;
    int16_t featDimPadded;
    int16_t featDimInterPolated;
    float *featureInterPolated = NULL;
    float *featurePadded = NULL;
    float *featureCol = NULL;

    // for GEMM algorithm
    MatrixStruct matA, matB;
    MatrixStructUnconst matM;

    // padding
    if (cnnLayer->padding == VALID) {
        featurePadded = featureIn;
    }
    else if (cnnLayer->padding == SAME) {

        // get InterPolated feat dim
        featDimInterPolated = cnnLayer->featDimIn * cnnLayer->stride;
        // get total padding size
        paddingSize = (cnnLayer->featDimOut - 1) + cnnLayer->kernelSize - featDimInterPolated;
        if (paddingSize < 0) {
			LOGD("Error configuration in Conv1DTranspose, paddingSize < 0!!\n");
            exit(-1);
        }
        // get padded feat dim
        featDimPadded = featDimInterPolated + paddingSize;

        // get padding size at begin and end of feature
        if (paddingSize % 2 != 0) {
            // odd padding size, more padding at the end
            paddingSizeBegin = max((paddingSize - 1) / 2, 0);
            paddingSizeEnd = (paddingSize + 1) / 2;
        }
        else {
            // even padding size, same padding size at both side
            paddingSizeBegin = paddingSize / 2;
            paddingSizeEnd = paddingSize / 2;
        }

        // malloc InterPolated feature
        featureInterPolated = cnnLayer->featureInterPolated;
        // perform InterPolating
        InterPolating1D(featureIn, featureInterPolated, featDimInterPolated, cnnLayer->stride, cnnLayer->numChannelsIn);

        // malloc padded feature
        featurePadded = cnnLayer->featurePadded;
        // perform padding
        PaddingSame1D(featureInterPolated, featurePadded, featDimInterPolated, paddingSizeBegin, paddingSizeEnd, cnnLayer->numChannelsIn);
    }

    // feature to col
    if (cnnLayer->padding == SAME) {
        // malloc columned feature
        featureCol = cnnLayer->featureCol;
        // perform feature to col
        FeatureToCol1D(featurePadded, featureCol, cnnLayer->featDimIn * cnnLayer->stride,
            cnnLayer->kernelSize, cnnLayer->numChannelsIn, 1,
            featDimPadded);
    }

    // perform convolution by matrix mult
    // using GEMM algorithm
    matA.data = cnnLayer->kernelCol;
    matA.row = cnnLayer->numChannelsOut;
    matA.col = cnnLayer->numChannelsIn * cnnLayer->kernelSize;
    matB.data = featureCol;
    matB.row = cnnLayer->featDimIn * cnnLayer->stride;
    matB.col = cnnLayer->numChannelsIn * cnnLayer->kernelSize;
    matM.data = cnnLayer->featOut;
    matM.row = cnnLayer->numChannelsOut;
    matM.col = cnnLayer->featDimIn * cnnLayer->stride;

    SetFloat(cnnLayer->featOut, 0.0f, (short)(matM.row * matM.col));

// #ifndef AVS_NEON_ON
//     MatrixMultGemm(&matA, &matB, &matM);
// #else
//     MatrixMultGemmNeon(&matA, &matB, &matM);
// #endif
#if defined(_AVX2) && defined(SUPPORT_AVX2)
    MatrixMultGemmAvx2(&matA, &matB, &matM);
#elif defined(_AVX512) && defined(SUPPORT_AVX512)
    MatrixMultGemmAvx512(&matA, &matB, &matM);
#elif defined(_NEON) && defined(SUPPORT_NEON)
    MatrixMultGemmNeon(&matA, &matB, &matM);
#else
    MatrixMultGemm(&matA, &matB, &matM);
#endif

    // add bias to feature
    if (cnnLayer->useBias == 1) {
        AddBias(cnnLayer->featOut, cnnLayer->bias, cnnLayer->featDimOut, cnnLayer->numChannelsOut);
    }

    // apply activation function
    if (cnnLayer->activationFunc == LINEAR) {
        ApplyLinearActFuncVec(cnnLayer->featOut, cnnLayer->featDimOut * cnnLayer->numChannelsOut, cnnLayer->featOut);
    }
    else if (cnnLayer->activationFunc == RELU) {
        ApplyReluActFuncVec(cnnLayer->featOut, cnnLayer->featDimOut * cnnLayer->numChannelsOut, cnnLayer->featOut);
    }
    else if (cnnLayer->activationFunc == GDN) {
        ApplyGdnActFunc(cnnLayer->gdnActFuncParam, cnnLayer->featOut, cnnLayer->featDimOut, cnnLayer->numChannelsOut, cnnLayer->featOut);
    }
    else if (cnnLayer->activationFunc == IGDN) {
        ApplyIgdnActFunc(cnnLayer->gdnActFuncParam, cnnLayer->featOut, cnnLayer->featDimOut, cnnLayer->numChannelsOut, cnnLayer->featOut);
    }

    return 0;
}


/*
1D transpose convolution with stride and activation function
2 parts version for stride is 2
remove useless mult for interpolated zeros, by two time conv op
I/O params:
    CnnStructHandle cnnLayer        (i/o) cnn layer structure, include output feature buffer
    float *featureIn                (i)   input feature map, size: cnnLayer->featDimIn * cnnLayer->numChannelsIn
*/
int16_t Conv1DTranspose2Part(
    CnnStructHandle cnnLayer,
    float *featureIn
)
{
    int16_t paddingSize;
    int16_t paddingSizeBegin, paddingSizeEnd;
    int16_t featDimPadded;
    float *featurePadded = NULL;

    // for GEMM algorithm
    MatrixStruct matA, matB;
    MatrixStructUnconst matMOdd, matMEven;

    // padding
    if (cnnLayer->padding == VALID) {
        featurePadded = featureIn;
    }
    else if (cnnLayer->padding == SAME) {
        // get total padding size
        // for two part version, padding size fixed to 2
        paddingSize = 2;
        // get padded feat dim
        featDimPadded = cnnLayer->featDimIn + paddingSize;

        // get padding size at begin and end of feature
        if (paddingSize % 2 != 0) {
            // odd padding size, more padding at the end
            paddingSizeBegin = max((paddingSize - 1) / 2, 0);
            paddingSizeEnd = (paddingSize + 1) / 2;
        }
        else {
            // even padding size, same padding size at both side
            paddingSizeBegin = paddingSize / 2;
            paddingSizeEnd = paddingSize / 2;
        }

        // malloc padded feature
        featurePadded = cnnLayer->featurePadded;
        // perform padding
        PaddingSame1D(featureIn, featurePadded, cnnLayer->featDimIn, paddingSizeBegin,
            paddingSizeEnd, cnnLayer->numChannelsIn);
    }

    if (cnnLayer->padding == SAME) {
        // perform feature to col
        // Odd part, kernel size is (kernelSize+1)/2
        FeatureToCol1D(featurePadded, cnnLayer->featureColOdd, cnnLayer->featDimIn,
            (cnnLayer->kernelSize + 1) / 2, cnnLayer->numChannelsIn, 1,
            featDimPadded);

        // Even part, kernel size is (kernelSize-1)/2
        if (cnnLayer->kernelSize == 3) {
            // for kernel size 3
            // kernel size for even part is 1, use featureIn instead of featurePadded
            FeatureToCol1D(featureIn, cnnLayer->featureColEven, cnnLayer->featDimIn,
                (cnnLayer->kernelSize - 1) / 2, cnnLayer->numChannelsIn, 1, cnnLayer->featDimIn);
        }
        else if (cnnLayer->kernelSize == 5) {
            // for kernel size 5
            // use featurePadded
            FeatureToCol1D(featurePadded, cnnLayer->featureColEven, cnnLayer->featDimIn,
                (cnnLayer->kernelSize - 1) / 2, cnnLayer->numChannelsIn, 1, featDimPadded);
        }
    }

    // perform convolution by matrix mult
    // using GEMM algorithm
    matA.data = cnnLayer->kernelColOdd;
    matA.row = cnnLayer->numChannelsOut;
    matA.col = cnnLayer->numChannelsIn * (cnnLayer->kernelSize + 1) / 2;
    matB.data = cnnLayer->featureColOdd;
    matB.row = cnnLayer->featDimIn;
    matB.col = cnnLayer->numChannelsIn * (cnnLayer->kernelSize + 1) / 2;
    matMOdd.data = cnnLayer->featOutOdd;
    matMOdd.row = cnnLayer->numChannelsOut;
    matMOdd.col = cnnLayer->featDimIn;

    SetFloat(cnnLayer->featOutOdd, 0.0f, (short)(matMOdd.row * matMOdd.col));

// #ifndef AVS_NEON_ON
//     MatrixMultGemm(&matA, &matB, &matMOdd);
// #else
//     MatrixMultGemmNeon(&matA, &matB, &matMOdd);
// #endif
#if defined(_AVX2) && defined(SUPPORT_AVX2)
    MatrixMultGemmAvx2(&matA, &matB, &matMOdd);
#elif defined(_AVX512) && defined(SUPPORT_AVX512)
    MatrixMultGemmAvx512(&matA, &matB, &matMOdd);
#elif defined(_NEON) && defined(SUPPORT_NEON)
    MatrixMultGemmNeon(&matA, &matB, &matMOdd);
#else
    MatrixMultGemm(&matA, &matB, &matMOdd);
#endif

    // perform convolution by matrix mult
    // using GEMM algorithm
    matA.data = cnnLayer->kernelColEven;
    matA.row = cnnLayer->numChannelsOut;
    matA.col = cnnLayer->numChannelsIn * (cnnLayer->kernelSize - 1) / 2;
    matB.data = cnnLayer->featureColEven;
    matB.row = cnnLayer->featDimIn;
    matB.col = cnnLayer->numChannelsIn * (cnnLayer->kernelSize - 1) / 2;
    matMEven.data = cnnLayer->featOutEven;
    matMEven.row = cnnLayer->numChannelsOut;
    matMEven.col = cnnLayer->featDimIn;

    SetFloat(cnnLayer->featOutEven, 0.0f, (short)(matMEven.row * matMEven.col));

// #ifndef AVS_NEON_ON
//     MatrixMultGemm(&matA, &matB, &matMEven);
// #else
//     MatrixMultGemmNeon(&matA, &matB, &matMEven);
// #endif
#if defined(_AVX2) && defined(SUPPORT_AVX2)
    MatrixMultGemmAvx2(&matA, &matB, &matMEven);
#elif defined(_AVX512) && defined(SUPPORT_AVX512)
    MatrixMultGemmAvx512(&matA, &matB, &matMEven);
#elif defined(_NEON) && defined(SUPPORT_NEON)
    MatrixMultGemmNeon(&matA, &matB, &matMEven);
#else
    MatrixMultGemm(&matA, &matB, &matMEven);
#endif

    // Interleave to get output feature
    if (cnnLayer->kernelSize == 5) {
        // for kernelSize 5, even part first
#if defined(_AVX2) && defined(SUPPORT_AVX2)
    int16_t i;
    for (i = 0; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut - 7; i += 8) {
        __m256 yVecOdd = _mm256_loadu_ps(cnnLayer->featOutOdd + i);
        __m256 yVecEven = _mm256_loadu_ps(cnnLayer->featOutEven + i);
        __m256 yVecLow = _mm256_unpacklo_ps(yVecEven,yVecOdd);
        __m256 yVecHigh = _mm256_unpackhi_ps(yVecEven,yVecOdd);
        __m256 yVec = _mm256_permute2f128_ps(yVecLow, yVecHigh, 0x20);
        _mm256_storeu_ps(cnnLayer->featOut + 2 * i, yVec);
        yVec = _mm256_permute2f128_ps(yVecLow, yVecHigh, 0x31);
        _mm256_storeu_ps(cnnLayer->featOut + 2 * i + 8, yVec);
    }
    for (; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut; i++) {
        cnnLayer->featOut[2 * i] = cnnLayer->featOutEven[i];
        cnnLayer->featOut[2 * i + 1] = cnnLayer->featOutOdd[i];
    }
#elif defined(_AVX512) && defined(SUPPORT_AVX512)
    int16_t i;
    for (i = 0; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut - 7; i += 8) {
        __m256 yVecOdd = _mm256_loadu_ps(cnnLayer->featOutOdd + i);
        __m256 yVecEven = _mm256_loadu_ps(cnnLayer->featOutEven + i);
        // __m256 yVecLow = _mm256_unpacklo_ps(yVecOdd, yVecEven);
        // __m256 yVecHigh = _mm256_unpackhi_ps(yVecOdd, yVecEven);
        __m256 yVecLow = _mm256_unpacklo_ps(yVecEven,yVecOdd);
        __m256 yVecHigh = _mm256_unpackhi_ps(yVecEven,yVecOdd);
        __m256 yVec = _mm256_permute2f128_ps(yVecLow, yVecHigh, 0x20);
        _mm256_storeu_ps(cnnLayer->featOut + 2 * i, yVec);
        yVec = _mm256_permute2f128_ps(yVecLow, yVecHigh, 0x31);
        _mm256_storeu_ps(cnnLayer->featOut + 2 * i + 8, yVec);
    }
    for (; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut; i++) {
        cnnLayer->featOut[2 * i] = cnnLayer->featOutEven[i];
        cnnLayer->featOut[2 * i + 1] = cnnLayer->featOutOdd[i];
    }
#elif defined(_NEON) && defined(SUPPORT_NEON)
    int16_t i;
    for (i = 0; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut - 3; i += 4) {   // 4 num, 3 tail
        float32x4x2_t yVec;
        yVec.val[0] = vld1q_f32(cnnLayer->featOutEven + i);
        yVec.val[1] = vld1q_f32(cnnLayer->featOutOdd + i);
        vst2q_f32(cnnLayer->featOut + 2 * i, yVec);                                 // 2 even odd
    }
    for (; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut; i++) {
        cnnLayer->featOut[2 * i] = cnnLayer->featOutEven[i];                        // 2 even odd
        cnnLayer->featOut[2 * i + 1] = cnnLayer->featOutOdd[i];                     // 2 even odd
    }
#else
    for (int16_t i = 0; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut; i++) {
        cnnLayer->featOut[2 * i] = cnnLayer->featOutEven[i];
        cnnLayer->featOut[2 * i + 1] = cnnLayer->featOutOdd[i];
    }
#endif
    }
    else if (cnnLayer->kernelSize == 3) {
        // for kernelSize 3, odd part first
#if defined(_AVX2) && defined(SUPPORT_AVX2)
    int16_t i;
    // for (i = 0; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut - 3; i += 4) {
    //     __m256 yVec = _mm256_setr_ps(cnnLayer->featOutOdd[i], cnnLayer->featOutEven[i],
    //     cnnLayer->featOutOdd[i+1], cnnLayer->featOutEven[i+1],
    //     cnnLayer->featOutOdd[i+2], cnnLayer->featOutEven[i+2],
    //     cnnLayer->featOutOdd[i+3], cnnLayer->featOutEven[i+3]);
    //     _mm256_storeu_ps(cnnLayer->featOut + 2 * i, yVec);
    // }
    for (i = 0; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut - 7; i += 8) {
        __m256 yVecOdd = _mm256_loadu_ps(cnnLayer->featOutOdd + i);
        __m256 yVecEven = _mm256_loadu_ps(cnnLayer->featOutEven + i);
        __m256 yVecLow = _mm256_unpacklo_ps(yVecOdd, yVecEven);
        __m256 yVecHigh = _mm256_unpackhi_ps(yVecOdd, yVecEven);
        // __m256 yVecLow = _mm256_unpacklo_ps(yVecEven,yVecOdd);
        // __m256 yVecHigh = _mm256_unpackhi_ps(yVecEven,yVecOdd);
        __m256 yVec = _mm256_permute2f128_ps(yVecLow, yVecHigh, 0x20);
        _mm256_storeu_ps(cnnLayer->featOut + 2 * i, yVec);
        yVec = _mm256_permute2f128_ps(yVecLow, yVecHigh, 0x31);
        _mm256_storeu_ps(cnnLayer->featOut + 2 * i + 8, yVec);
    }
    for (; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut; i++) {
        cnnLayer->featOut[2 * i] = cnnLayer->featOutOdd[i];
        cnnLayer->featOut[2 * i + 1] = cnnLayer->featOutEven[i];
    }
#elif defined(_AVX512) && defined(SUPPORT_AVX512)
    int16_t i;
    for (i = 0; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut - 7; i += 8) {
        __m256 yVecOdd = _mm256_loadu_ps(cnnLayer->featOutOdd + i);
        __m256 yVecEven = _mm256_loadu_ps(cnnLayer->featOutEven + i);
        __m256 yVecLow = _mm256_unpacklo_ps(yVecOdd, yVecEven);
        __m256 yVecHigh = _mm256_unpackhi_ps(yVecOdd, yVecEven);
        __m256 yVec = _mm256_permute2f128_ps(yVecLow, yVecHigh, 0x20);
        _mm256_storeu_ps(cnnLayer->featOut + 2 * i, yVec);
        yVec = _mm256_permute2f128_ps(yVecLow, yVecHigh, 0x31);
        _mm256_storeu_ps(cnnLayer->featOut + 2 * i + 8, yVec);
    }
    for (; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut; i++) {
        cnnLayer->featOut[2 * i] = cnnLayer->featOutOdd[i];
        cnnLayer->featOut[2 * i + 1] = cnnLayer->featOutEven[i];
    }
#elif defined(_NEON) && defined(SUPPORT_NEON)
    int16_t i;
    for (i = 0; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut - 3; i += 4) {   // 4 num, 3 tail
        float32x4x2_t yVec;
        yVec.val[0] = vld1q_f32(cnnLayer->featOutOdd + i);
        yVec.val[1] = vld1q_f32(cnnLayer->featOutEven + i);
        vst2q_f32(cnnLayer->featOut + 2 * i, yVec);                                 // 2 even odd
    }
    for (; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut; i++) {
        cnnLayer->featOut[2 * i] = cnnLayer->featOutOdd[i];                         // 2 even odd
        cnnLayer->featOut[2 * i + 1] = cnnLayer->featOutEven[i];                    // 2 even odd
    }
#else
    for (int16_t i = 0; i < cnnLayer->featDimIn * cnnLayer->numChannelsOut; i++) {
        cnnLayer->featOut[2 * i] = cnnLayer->featOutOdd[i];
        cnnLayer->featOut[2 * i + 1] = cnnLayer->featOutEven[i];
    }
#endif
    }

    // add bias to feature
    if (cnnLayer->useBias == 1) {
        AddBias(cnnLayer->featOut, cnnLayer->bias, cnnLayer->featDimOut, cnnLayer->numChannelsOut);
    }

    // apply activation function
    if (cnnLayer->activationFunc == LINEAR) {
        ApplyLinearActFuncVec(cnnLayer->featOut, cnnLayer->featDimOut * cnnLayer->numChannelsOut, cnnLayer->featOut);
    }
    else if (cnnLayer->activationFunc == RELU) {
        ApplyReluActFuncVec(cnnLayer->featOut, cnnLayer->featDimOut * cnnLayer->numChannelsOut, cnnLayer->featOut);
    }
    else if (cnnLayer->activationFunc == GDN) {
        ApplyGdnActFunc(cnnLayer->gdnActFuncParam, cnnLayer->featOut, cnnLayer->featDimOut, cnnLayer->numChannelsOut, cnnLayer->featOut);
    }
    else if (cnnLayer->activationFunc == IGDN) {
        ApplyIgdnActFunc(cnnLayer->gdnActFuncParam, cnnLayer->featOut, cnnLayer->featDimOut, cnnLayer->numChannelsOut, cnnLayer->featOut);
    }

    return 0;
}


/*
Destroy cnn layer structure
I/O params:
    CnnStructHandle cnnLayer        (i/o) cnn layer structure handle
    int16_t isTranspos              (i) flag for conv transpose
*/
int16_t DestroyCnnLayer(
    CnnStructHandle cnnLayer
)
{
	// free kernel
    int16_t chNum;
    if (cnnLayer->isTranspose == 0) {
        // conv, kernelSize * numChannelsIn * numChannelsOut
        chNum = cnnLayer->numChannelsIn;
    }
    else {
        // conv, kernelSize * numChannelsOut * numChannelsIn
        chNum = cnnLayer->numChannelsOut;
    }
    for (int16_t i = 0; i < cnnLayer->kernelSize; i++)
    {
        for (int16_t j = 0; j < chNum; j++) {
            free(cnnLayer->kernel[i][j]);
            cnnLayer->kernel[i][j] = NULL;
        }
        free(cnnLayer->kernel[i]);
        cnnLayer->kernel[i] = NULL;
    }
    free(cnnLayer->kernel);
    cnnLayer->kernel = NULL;

    // free bias
    if (cnnLayer->bias != NULL) {
        free(cnnLayer->bias);
        cnnLayer->bias = NULL;
    }

    // free gdn param
    if (cnnLayer->gdnActFuncParam != NULL) {
        DestroyGdnParam(cnnLayer->gdnActFuncParam, cnnLayer->numChannelsOut);
        free(cnnLayer->gdnActFuncParam);
        cnnLayer->gdnActFuncParam = NULL;
    }

    // free output feature buffer
    free(cnnLayer->featOut);
    cnnLayer->featOut = NULL;

    // free runtime buffer
    CnnFreeRuntimeBuffer(cnnLayer);

	return 0;
}
