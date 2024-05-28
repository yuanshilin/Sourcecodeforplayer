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
#include "avs3_stat_com.h"
#include "avs3_prot_com.h"


// Tool function of matrix transpose
// nRow and nCol are for input matrix mat
static __inline void MatrixTranspose1D(
    const float *mat,
    float *matT,
    int nRow,
    int nCol)
{
    int row, col;
    for (row = 0; row < nCol; row++) {
        for (col = 0; col < nRow; col++) {
            matT[row * nRow + col] = mat[col * nCol + row];
        }
    }
}


/*
Apply RELU activation function for vector
I/O params:
    const float *srcVec:      (i) source vector, size: len
    int16_t len:              (i) length of vector
    float *destVec            (o) dest vector, size: len
*/

void ApplyReluActFuncVec(
    const float *srcVec,
    int16_t len,
    float *destVec
)
{
    for (int16_t i = 0; i < len; i++) {
        if (srcVec[i] > 0.0f) {
            destVec[i] = srcVec[i];
        }
        else {
            destVec[i] = 0.0f;
        }
    }
}


/*
Apply RELU activation function for 2D
I/O params:
    const float **srcMat            (i) source matrix, size: numRow * numCol
    int16_t numRow                  (i) number of rows
    int16_t numCol                  (i) number of cols
    float **destMat                 (o) dest matrix, size: numRow * numCol
*/
void ApplyReluActFunc2D(
    const float **srcMat,
    int16_t numRow,
    int16_t numCol,
    float **destMat
)
{
    for (int16_t i = 0; i < numRow; i++) {
        ApplyReluActFuncVec(srcMat[i], numCol, destMat[i]);
    }
}


/*
Apply linear activation function for vector
I/O params:
    const float *srcVec:        (i) source vector, size: len
    int16_t len:                (i) length of vector
    float *destVec:             (o) dest vector, size: len
*/

void ApplyLinearActFuncVec(
    const float *srcVec,
    int16_t len,
    float *destVec
)
{
    for (int16_t i = 0; i < len; i++) {
        destVec[i] = srcVec[i];
    }
}


/*
Apply linear activation function for 2D
I/O params:
    const float **srcMat            (i) source matrix, size: numRow * numCol
    int16_t numRow                  (i) number of rows
    int16_t numCol                  (i) number of cols
    float **destMat                 (o) dest matrix, size: numRow * numCol
*/
void ApplyLinearActFunc2D(
    const float **srcMat,
    int16_t numRow,
    int16_t numCol,
    float **destMat
)
{
    for (int16_t i = 0; i < numRow; i++) {
        ApplyLinearActFuncVec(srcMat[i], numCol, destMat[i]);
    }
}


/*
Apply sigmoid activation function for vector
I/O params:
    const float *srcVec:        (i) source vector, size: len
    int16_t len:                (i) length of vector
    float *destVec              (o) dest vector, size: len
*/

void ApplySigmoidActFuncVec(
    const float *srcVec,
    int16_t len,
    float *destVec
)
{
    for (int16_t i = 0; i < len; i++) {
        destVec[i] = 1.0f / (1.0f + (float)exp(-srcVec[i]));
    }
}


/*
Apply sigmoid activation function for 2D
I/O params:
    const float **srcMat            (i) source matrix, size: numRow * numCol
    int16_t numRow                  (i) number of rows
    int16_t numCol                  (i) number of cols
    float **destMat                 (o) dest matrix, size: numRow * numCol
*/
void ApplySigmoidActFunc2D(
    const float **srcMat,
    int16_t numRow,
    int16_t numCol,
    float **destMat
)
{
    for (int16_t i = 0; i < numRow; i++) {
        ApplySigmoidActFuncVec(srcMat[i], numCol, destMat[i]);
    }
}


/*
Apply TANH activation function for vector
I/O params:
    const float *srcVec:        (i) source vector, size: len
    int16_t len:                (i) length of vector
    float *destVec              (o) dest vector, size: len
*/

void ApplyTanhActFuncVec(
    const float *srcVec,
    int16_t len,
    float *destVec
)
{
    for (int16_t i = 0; i < len; i++) {
        destVec[i] = 2.0f / (1.0f + (float)exp(-2.0f * srcVec[i])) - 1.0f;
    }
}


/*
Apply TANH activation function for 2D
I/O params: 
    const float **srcMat            (i) source matrix, size: numRow * numCol
    int16_t numRow                  (i) number of rows
    int16_t numCol                  (i) number of cols
    float **destMat                 (o) dest matrix, size: numRow * numCol
*/

void ApplyTanhActFunc2D(
    const float **srcMat,
    int16_t numRow,
    int16_t numCol,
    float **destMat
)
{
    for (int16_t i = 0; i < numRow; i++) {
        ApplyTanhActFuncVec(srcMat[i], numCol, destMat[i]);
    }
}


/*
Init parameter structure of GDN/IGDN activation function
I/O params:
    FILE *fModel                                (i) model file
    GdnActFuncHandle gdnActFuncParam            (o) GDN/IGDN param handle
    int16_t numChannelsOut                      (i) number of output channels
*/
int16_t InitGdnParam(
    modul_structure *fModel,
    GdnActFuncHandle gdnActFuncParam,
    int16_t numChannelsOut
)
{
    float beta;
    float gamma;

    // beta param
    gdnActFuncParam->beta = (float *)malloc(sizeof(float) * numChannelsOut);
    for (int16_t i = 0; i < numChannelsOut; i++) {
//        fread(&beta, sizeof(float), 1, fModel);
		memcpy(&beta, fModel->data + fModel->nIndex, sizeof(float));
		fModel->nIndex += sizeof(float);
		gdnActFuncParam->beta[i] = beta;
    }

    // gamma param
    gdnActFuncParam->gamma = (float *)malloc(sizeof(float) * numChannelsOut * numChannelsOut);
    for (int16_t i = 0; i < numChannelsOut; i++) {
        for (int16_t j = 0; j < numChannelsOut; j++) {
//            fread(&gamma, sizeof(float), 1, fModel);
			memcpy(&gamma, fModel->data + fModel->nIndex, sizeof(float));
			fModel->nIndex += sizeof(float);
            gdnActFuncParam->gamma[i + j * numChannelsOut] = gamma;
        }
    }

    return 0;
}


/*
Destroy parameter structure of GDN/IGDN activation function
I/O params:
    GdnActFuncHandle gdnActFuncParam                (i/o) GDN/IGDN param handle
    int16_t numChannelsOut                          (i)   number of output channels
*/
int16_t DestroyGdnParam(
    GdnActFuncHandle gdnActFuncParam,
    int16_t numChannelsOut
)
{
    // beta param
    free(gdnActFuncParam->beta);
    gdnActFuncParam->beta = NULL;

    // gamma param
    free(gdnActFuncParam->gamma);
    gdnActFuncParam->gamma = NULL;

    return 0;
}


/*
Apply GDN activation function for input feature map
Current version is for 2D feature map, i.e. dimFeat * numChannel
I/O params:
    GdnActFuncHandle gdnActFuncParam:           (i) parameter st for GDN activation func
    const float *featureIn                      (i) input feature map, 2D, dimFeat * numChannel
    int16_t dimFeat                             (i) feature dim, 1st dim of feature map
    int16_t numChannel                          (i) channel number, 2nd dim of feature map
    float *featureOut                           (o) output feature map, 2D
*/

void ApplyGdnActFunc(
    GdnActFuncHandle gdnActFuncParam,
    const float *featureIn,
    int16_t dimFeat,
    int16_t numChannel,
    float *featureOut
)
{
    float *squaredFeature;
    float *tmpFeature;
    float *featureInT;          // transposed mat of featureIn
    float *tmpFeatureT;         // transposed mat of tmpFeature

    // transpose input feature
    featureInT = (float *)malloc(sizeof(float) * dimFeat * numChannel);
    MatrixTranspose1D(featureIn, featureInT, numChannel, dimFeat);

    // calculate x[j]^2
    squaredFeature = (float *)malloc(sizeof(float) * dimFeat * numChannel);
    for (int16_t i = 0; i < dimFeat * numChannel; i++) {
        squaredFeature[i] = featureInT[i] * featureInT[i];
    }

    // tmp feature buffer
    tmpFeature = (float *)malloc(sizeof(float) * dimFeat * numChannel);

    // apply GDN
    MatrixStruct matA, matB;
    MatrixStructUnconst matM;
    matA.data = squaredFeature;
    matA.row = dimFeat;
    matA.col = numChannel;
    matB.data = gdnActFuncParam->gamma;
    matB.row = numChannel;
    matB.col = numChannel;
    matM.data = tmpFeature;
    matM.row = dimFeat;
    matM.col = numChannel;

    SetFloat(tmpFeature, 0.0f, matM.row * matM.col);

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

    // transpose output matrix
    tmpFeatureT = (float *)malloc(sizeof(float) * dimFeat * numChannel);
    MatrixTranspose1D(tmpFeature, tmpFeatureT, dimFeat, numChannel);

    // division for GDN
    for (int16_t i = 0; i < dimFeat; i++) {
        for (int16_t j = 0; j < numChannel; j++) {
            featureOut[i + j * dimFeat] /= (float)(sqrt(tmpFeatureT[i + j * dimFeat] + gdnActFuncParam->beta[j]));
        }
    }

    free(squaredFeature);
    squaredFeature = NULL;

    free(tmpFeature);
    tmpFeature = NULL;

    free(featureInT);
    featureInT = NULL;

    free(tmpFeatureT);
    tmpFeatureT = NULL;
}


/*
Apply IGDN activation function for input feature map
Current version is for 2D feature map, i.e. dimFeat * numChannel
I/O params:
    GdnActFuncHandle gdnActFuncParam:           (i) parameter st for GDN activation func
    const float *featureIn                      (i) input feature map, 2D, dimFeat * numChannel
    int16_t dimFeat                             (i) feature dim, 1st dim of feature map
    int16_t numChannel                          (i) channel number, 2nd dim of feature map
    float *featureOut                           (o) output feature map, 2D
*/

void ApplyIgdnActFunc(
    GdnActFuncHandle gdnActFuncParam,
    const float *featureIn,
    int16_t dimFeat,
    int16_t numChannel,
    float *featureOut
)
{
    float *squaredFeature;
    float *tmpFeature;
    float *featureInT;          // transposed mat of featureIn
    float *tmpFeatureT = NULL;  // transposed mat of tmpFeature

    // transpose input feature
    featureInT = (float *)malloc(sizeof(float) * dimFeat * numChannel);
    MatrixTranspose1D(featureIn, featureInT, numChannel, dimFeat);

    // calculate x[j]^2
    squaredFeature = (float *)malloc(sizeof(float) * dimFeat * numChannel);
    for (int16_t i = 0; i < dimFeat * numChannel; i++) {
        squaredFeature[i] = featureInT[i] * featureInT[i];
    }

    // tmp feature buffer
    tmpFeature = (float *)malloc(sizeof(float) * dimFeat * numChannel);

    // apply IGDN
    MatrixStruct matA, matB;
    MatrixStructUnconst matM;
    matA.data = squaredFeature;
    matA.row = dimFeat;
    matA.col = numChannel;
    matB.data = gdnActFuncParam->gamma;
    matB.row = numChannel;
    matB.col = numChannel;
    matM.data = tmpFeature;
    matM.row = dimFeat;
    matM.col = numChannel;

    SetFloat(tmpFeature, 0.0f, (short)(matM.row * matM.col));

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

    tmpFeatureT = (float *)malloc(sizeof(float) * dimFeat * numChannel);
    MatrixTranspose1D(tmpFeature, tmpFeatureT, dimFeat, numChannel);

    // multiply for IGDN
    for (int16_t i = 0; i < dimFeat; i++) {
        for (int16_t j = 0; j < numChannel; j++) {
            featureOut[i + j * dimFeat] *= (float)(sqrt(tmpFeatureT[i + j * dimFeat] + gdnActFuncParam->beta[j]));
        }
    }

    free(squaredFeature);
    squaredFeature = NULL;

    free(tmpFeature);
    tmpFeature = NULL;

    free(featureInT);
    featureInT = NULL;

    free(tmpFeatureT);
    tmpFeatureT = NULL;
}
