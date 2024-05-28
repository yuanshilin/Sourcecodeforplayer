#include <math.h>
#include <assert.h>

#include "avs3_prot_com.h"

// #ifdef AVS_NEON_ON
// #include <arm_neon.h>
// #endif

void SetZero(float *vec, const short len)
{
    short i;

    for (i = 0; i < len; i++)
    {
        *vec++ = 0.f;
    }

    return;
}

void SetFloat(float y[], const float val, const short N)
{
    short i;

    for (i = 0; i < N; i++)
    {
        y[i] = val;
    }

    return;
}

void SetShort(short y[],const short a, const short N)
{
    short i;

    for (i = 0; i < N; i++)
    {
        y[i] = a;
    }

    return;
}


void SetUShort(unsigned short y[], const unsigned short a, const short N)
{
    short i;

    for (i = 0; i < N; i++)
    {
        y[i] = a;
    }

    return;
}


void SetUC(uint8_t y[], const uint8_t a, const short N)
{
    short i;

    for (i = 0; i < N; i++)
    {
        y[i] = a;
    }

    return;
}


void Mvf2f(const float x[], float y[], const short n)
{
    short i;

    if (n <= 0)
    {
        return;
    }

    if (y < x)
    {
        for (i = 0; i < n; i++)
        {
            y[i] = x[i];
        }
    }
    else
    {
        for (i = n - 1; i >= 0; i--)
        {
            y[i] = x[i];
        }
    }

    return;
}

float VLinalgNorm(float* vec,const short len)
{
    assert(vec != NULL && len >= 1);

    float result = 0.f;

    for (short i = 0; i < len; i++)
    {
        result += vec[i] * vec[i];
    }

    result = (float)sqrt(result);

    return result;
}

float Dotp(const float  x[], const float  y[], const short  n)
{
    short i;
    float suma;

    suma = x[0] * y[0];

    for (i = 1; i < n; i++)
    {
        suma += x[i] * y[i];
    }

    return suma;
}

void MvShort2Short(const short x[], short y[], const short n )
{
    short i;

    if (n <= 0)
    {
        /* cannot transfer vectors with size 0 */
        return;
    }

    if (y < x)
    {
        for (i = 0; i < n; i++)
        {
            y[i] = x[i];
        }
    }
    else
    {
        for (i = n - 1; i >= 0; i--)
        {
            y[i] = x[i];
        }
    }

    return;
}


unsigned long MvFloat2Short(const float x[], short y[], const short n)
{
    short i;
    float temp;
    unsigned long noClipping = 0;

    if (n <= 0)
    {
        /* cannot transfer vectors with size 0 */
        return 0;
    }

    if ((void*)y < (const void*)x)
    {
        for (i = 0; i < n; i++)
        {
            temp = x[i];
            temp = (float)floor(temp + 0.5f);

            if (temp > 32767.0f)
            {
                temp = 32767.0f;
                noClipping++;
            }
            else if (temp < -32768.0f)
            {
                temp = -32768.0f;
                noClipping++;
            }

            y[i] = (short)temp;
        }
    }
    else
    {
        for (i = n - 1; i >= 0; i--)
        {
            temp = x[i];
            temp = (float)floor(temp + 0.5f);

            if (temp > 32767.0f)
            {
                temp = 32767.0f;
                noClipping++;
            }
            else if (temp < -32768.0f)
            {
                temp = -32768.0f;
                noClipping++;
            }

            y[i] = (short)temp;
        }
    }

    return noClipping;
}

void Vadd(const float x1[], const float x2[], float y[], const short N)
{
    short i;

    for (i = 0; i < N; i++)
    {
        y[i] = x1[i] + x2[i];
    }

    return;
}

void VMult(const float x1[], const float x2[], float y[], const short N)
{
    short i;

    for (i = 0; i < N; i++)
    {
        y[i] = x1[i] * x2[i];
    }

    return;
}

void SwapS(short *a, short *b)
{
    short tmp = *a;
    *a = *b;
    *b = tmp;
}

void SortS(short *x, const short len)
{
    short i, j;
    short min;

    if (x == NULL)
    {
        return;
    }

    for (i = 0; i < len - 1; i++)
    {
        min = i;
        for (j = i + 1; j < len; j++)
        {
            if (x[j] < x[min])
            {
                min = j;
            }
        }

        SwapS(&x[min], &x[i]);
    }
}

float SumFloat(const float *x, const short len)
{
    short i;
    float tmp;

    tmp = 0.f;
    for (i = 0; i < len; i++)
    {
        tmp += x[i];
    }

    return tmp;
}


void VMultC(const float x[], const float c, float y[], const short N)
{
    short i;

    for (i = 0; i < N; i++)
    {
        y[i] = c * x[i];
    }

    return;
}

void SetC(
    int8_t y[],
    const int8_t a,
    const short N
)
{
    short i;

    for (i = 0; i < N; i++)
    {
        y[i] = a;
    }

    return;
}

float Mean(
    const float *vec,
    const short lvec
)
{
    float tmp;

    tmp = SumFloat(vec, lvec) / (float)lvec;

    return tmp;
}


// GEMM matrix mult function
void MatrixMultGemm(
    const MatrixStruct *matA,
    const MatrixStruct *matB,
    MatrixHandleUnconst matM)
{
    int nRows = matM->row;
    int nCols = matM->col;
    int aNumCols = matA->col;
    const float *dataA = matA->data;
    const float *dataB = matB->data;
    float *dataM = matM->data;
    int aStride = aNumCols;
    int bStride = matB->col;
    int col, row, k;
    int stride = nCols;

#ifndef GEMM_REFORM_ENC
    for (row = 0; row < nRows; row++) {
        for (col = 0; col < nCols; col++) {
            float temp = 0.0;
            for (k = 0; k < aNumCols; k++) {
                temp += dataA[row * aStride + k] * dataB[col * bStride + k];
            }
            dataM[row * stride + col] = dataM[row * stride + col] + temp;
        }
    }
#else
    for (row = 0; row < nRows; row++) {
        for (col = 0; col < nCols; col++) {
            // 8 part
            float tmp1 = 0.0;
            float tmp2 = 0.0;
            float tmp3 = 0.0;
            float tmp4 = 0.0;
            for (k = 0; k < aNumCols - 7; k += 8) {
                tmp1 += dataA[row * aStride + k] * dataB[col * bStride + k];
                tmp1 += dataA[row * aStride + k + 1] * dataB[col * bStride + k + 1];

                tmp2 += dataA[row * aStride + k + 2] * dataB[col * bStride + k + 2];
                tmp2 += dataA[row * aStride + k + 3] * dataB[col * bStride + k + 3];

                tmp3 += dataA[row * aStride + k + 4] * dataB[col * bStride + k + 4];
                tmp3 += dataA[row * aStride + k + 5] * dataB[col * bStride + k + 5];

                tmp4 += dataA[row * aStride + k + 6] * dataB[col * bStride + k + 6];
                tmp4 += dataA[row * aStride + k + 7] * dataB[col * bStride + k + 7];
            }
            float outTmp1, outTmp2;
            outTmp1 = tmp1 + tmp2;
            outTmp2 = tmp3 + tmp4;
            dataM[row * stride + col] = outTmp1 + outTmp2;

            // tail part
            float temp = 0.0;
            for (; k < aNumCols; k++) {
                temp += dataA[row * aStride + k] * dataB[col * bStride + k];
            }
            dataM[row * stride + col] = dataM[row * stride + col] + temp;
        }
    }
#endif
}

#if defined(_AVX2) && defined(SUPPORT_AVX2)
void MatrixVecMultAvx2(
    int matRow,
    const MatrixStruct *matA,
    const MatrixStruct *matB,
    MatrixHandleUnconst matM)
{
    const float *dataA = matA->data;
    int aNumCols = matA->col;
    int aStride = matA->col;
    const float *dataB = matB->data;
    int bStride = matB->col;
    float *dataM = matM->data;
    int numCols = matM->col;

    int numColsCnt = numCols - 3;
    int anumColsCnt = aNumCols - 7;
    int stride = numCols;

    int col, k;
    float tmp1, tmp2;
    float tmpArr1[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float *dataANew = dataA + matRow * aStride;
    float *dataMNew0 = dataM + matRow * stride;
    __m256i perm = _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7);

    for (col = 0; col < numColsCnt; col = col + 4)
    {
        k = 0;
        const float *dataBNew1 = dataB + col * bStride;
        const float *dataBNew2 = dataBNew1 + bStride;
        const float *dataBNew3 = dataBNew2 + bStride;
        const float *dataBNew4 = dataBNew3 + bStride;
        if(k < anumColsCnt)
        {
            __m128 ftmp0t32x4 = _mm_setzero_ps();
            __m128 ftmp1t32x4 = _mm_setzero_ps();
            __m128 ftmp2t32x4 = _mm_setzero_ps();
            __m128 ftmp3t32x4 = _mm_setzero_ps();

            for (; k < anumColsCnt; k = k + 8)
            {
                __m256 fdataA32x8 = _mm256_loadu_ps(dataANew + k);
                __m256 fdataB32x8 = _mm256_loadu_ps(dataBNew1 + k);
                __m256 result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                result1 = _mm256_permutevar8x32_ps(result1, perm);
                ftmp0t32x4 = _mm_add_ps(_mm_add_ps(ftmp0t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));

                fdataB32x8 = _mm256_loadu_ps(dataBNew2 + k);
                result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                result1 = _mm256_permutevar8x32_ps(result1, perm);
                ftmp1t32x4 = _mm_add_ps(_mm_add_ps(ftmp1t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));
                
                fdataB32x8 = _mm256_loadu_ps(dataBNew3 + k);
                result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                result1 = _mm256_permutevar8x32_ps(result1, perm);
                ftmp2t32x4 = _mm_add_ps(_mm_add_ps(ftmp2t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));

                fdataB32x8 = _mm256_loadu_ps(dataBNew4 + k);
                result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                result1 = _mm256_permutevar8x32_ps(result1, perm);
                ftmp3t32x4 = _mm_add_ps(_mm_add_ps(ftmp3t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));
            }

            dataMNew0[col] = (ftmp0t32x4[0] + ftmp0t32x4[1]) + (ftmp0t32x4[2] + ftmp0t32x4[3]);
            dataMNew0[col + 1] = (ftmp1t32x4[0] + ftmp1t32x4[1]) + (ftmp1t32x4[2] + ftmp1t32x4[3]);
            dataMNew0[col + 2] = (ftmp2t32x4[0] + ftmp2t32x4[1]) + (ftmp2t32x4[2] + ftmp2t32x4[3]);
            dataMNew0[col + 3] = (ftmp3t32x4[0] + ftmp3t32x4[1]) + (ftmp3t32x4[2] + ftmp3t32x4[3]);
        }

        if(k < aNumCols)
        {
            __m128 dataMVec = _mm_setzero_ps();
            for (; k < aNumCols; k++) {
                __m128 tmpVec1 = _mm_set1_ps(dataANew[k]);
                __m128 tmpVec2 = _mm_setr_ps(dataBNew1[k], dataBNew2[k], dataBNew3[k], dataBNew4[k]);
                dataMVec = _mm_add_ps(dataMVec, _mm_mul_ps(tmpVec1, tmpVec2));
            }
            _mm_store_ps(dataMNew0 + col, _mm_add_ps(_mm_load_ps(dataMNew0 + col), dataMVec));
        }
    }

    for (; col < numCols; col++)
    {
        k = 0;
        if(k < anumColsCnt)
        {
            tmp1 = 0, tmp2 = 0;
            __m128 ftmp0t32x4 = _mm_setzero_ps();
            for (; k < anumColsCnt; k = k + 8) {
                __m256 fdataA32x8 = _mm256_loadu_ps(dataANew + k);
                __m256 fdataB32x8 = _mm256_loadu_ps(dataB + col * bStride + k);
                __m256 result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                result1 = _mm256_permutevar8x32_ps(result1, perm);
                ftmp0t32x4 = _mm_add_ps(_mm_add_ps(ftmp0t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));
            }
            tmp1 = ftmp0t32x4[0] + ftmp0t32x4[1];
            tmp2 = ftmp0t32x4[2] + ftmp0t32x4[3];
            dataMNew0[col] = tmp1 + tmp2;
        }

        if(k < aNumCols)
        {
            float temp = 0.0;
            for (; k < aNumCols; k++) {
                temp += dataANew[k] * dataB[col * bStride + k];
            }
            dataMNew0[col] += temp;
        }
    }
}

void MatrixMultGemmAvx2(
    const MatrixStruct *matA,
    const MatrixStruct *matB,
    MatrixHandleUnconst matM)
{
    int numRows = matM->row;
    int matRow;
    for (matRow = 0; matRow < numRows; matRow++) {
        MatrixVecMultAvx2(matRow, matA, matB, matM);
    }
}
#endif

#if defined(_AVX512) && defined(SUPPORT_AVX512)
void MatrixVecMultAvx512(
    int matRow,
    const MatrixStruct *matA,
    const MatrixStruct *matB,
    MatrixHandleUnconst matM)
{
    const float *dataA = matA->data;
    int aNumCols = matA->col;
    int aStride = matA->col;
    const float *dataB = matB->data;
    int bStride = matB->col;
    float *dataM = matM->data;
    int numCols = matM->col;

    int numColsCnt = numCols - 3;
    int anumColsCnt = aNumCols - 15;
    int bnumColsCnt = aNumCols - 7;
    int stride = numCols;

    int col, k;
    float tmp1, tmp2;
    float tmpArr1[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float *dataANew = dataA + matRow * aStride;
    float *dataMNew0 = dataM + matRow * stride;

    for (col = 0; col < numColsCnt; col = col + 4)
    {
        k = 0;
        const float *dataBNew1 = dataB + col * bStride;
        const float *dataBNew2 = dataBNew1 + bStride;
        const float *dataBNew3 = dataBNew2 + bStride;
        const float *dataBNew4 = dataBNew3 + bStride;
        if(k < anumColsCnt)
        {
            __m128 ftmp0t32x4 = _mm_setzero_ps();
            __m128 ftmp1t32x4 = _mm_setzero_ps();
            __m128 ftmp2t32x4 = _mm_setzero_ps();
            __m128 ftmp3t32x4 = _mm_setzero_ps();

            for (; k < anumColsCnt; k = k + 16)
            {
                __m512 fdataA32x16 = _mm512_setr_ps(dataANew[k], dataANew[k+2], dataANew[k+4], dataANew[k+6],
                dataANew[k+1], dataANew[k+3], dataANew[k+5], dataANew[k+7],
                dataANew[k+8], dataANew[k+10], dataANew[k+12], dataANew[k+14],
                dataANew[k+9], dataANew[k+11], dataANew[k+13], dataANew[k+15]);
                __m512 fdataB32x16 = _mm512_setr_ps(dataBNew1[k], dataBNew1[k+2], dataBNew1[k+4], dataBNew1[k+6],
                dataBNew1[k+1], dataBNew1[k+3], dataBNew1[k+5], dataBNew1[k+7],
                dataBNew1[k+8], dataBNew1[k+10], dataBNew1[k+12], dataBNew1[k+14],
                dataBNew1[k+9], dataBNew1[k+11], dataBNew1[k+13], dataBNew1[k+15]);
                __m512 result2 = _mm512_mul_ps(fdataA32x16, fdataB32x16);
                __m128 result3 = _mm512_extractf32x4_ps(result2,0);
                __m128 result4 = _mm512_extractf32x4_ps(result2,1);
                __m128 result5 = _mm512_extractf32x4_ps(result2,2);
                __m128 result6 = _mm512_extractf32x4_ps(result2,3);
                ftmp0t32x4 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_add_ps(ftmp0t32x4,result3),result4),result5),result6);

                fdataB32x16 = _mm512_setr_ps(dataBNew2[k], dataBNew2[k+2], dataBNew2[k+4], dataBNew2[k+6],
                dataBNew2[k+1], dataBNew2[k+3], dataBNew2[k+5], dataBNew2[k+7],
                dataBNew2[k+8], dataBNew2[k+10], dataBNew2[k+12], dataBNew2[k+14],
                dataBNew2[k+9], dataBNew2[k+11], dataBNew2[k+13], dataBNew2[k+15]);
                result2 = _mm512_mul_ps(fdataA32x16, fdataB32x16);
                result3 = _mm512_extractf32x4_ps(result2,0);
                result4 = _mm512_extractf32x4_ps(result2,1);
                result5 = _mm512_extractf32x4_ps(result2,2);
                result6 = _mm512_extractf32x4_ps(result2,3);
                ftmp1t32x4 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_add_ps(ftmp1t32x4,result3),result4),result5),result6);

                fdataB32x16 = _mm512_setr_ps(dataBNew3[k], dataBNew3[k+2], dataBNew3[k+4], dataBNew3[k+6],
                dataBNew3[k+1], dataBNew3[k+3], dataBNew3[k+5], dataBNew3[k+7],
                dataBNew3[k+8], dataBNew3[k+10], dataBNew3[k+12], dataBNew3[k+14],
                dataBNew3[k+9], dataBNew3[k+11], dataBNew3[k+13], dataBNew3[k+15]);
                result2 = _mm512_mul_ps(fdataA32x16, fdataB32x16);
                result3 = _mm512_extractf32x4_ps(result2,0);
                result4 = _mm512_extractf32x4_ps(result2,1);
                result5 = _mm512_extractf32x4_ps(result2,2);
                result6 = _mm512_extractf32x4_ps(result2,3);
                ftmp2t32x4 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_add_ps(ftmp2t32x4,result3),result4),result5),result6);

                fdataB32x16 = _mm512_setr_ps(dataBNew4[k], dataBNew4[k+2], dataBNew4[k+4], dataBNew4[k+6],
                dataBNew4[k+1], dataBNew4[k+3], dataBNew4[k+5], dataBNew4[k+7],
                dataBNew4[k+8], dataBNew4[k+10], dataBNew4[k+12], dataBNew4[k+14],
                dataBNew4[k+9], dataBNew4[k+11], dataBNew4[k+13], dataBNew4[k+15]);
                result2 = _mm512_mul_ps(fdataA32x16, fdataB32x16);
                result3 = _mm512_extractf32x4_ps(result2,0);
                result4 = _mm512_extractf32x4_ps(result2,1);
                result5 = _mm512_extractf32x4_ps(result2,2);
                result6 = _mm512_extractf32x4_ps(result2,3);
                ftmp3t32x4 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_add_ps(ftmp3t32x4,result3),result4),result5),result6);
            }

            if(k < bnumColsCnt)
            {
                for (; k < bnumColsCnt; k = k + 8)
                {
                    __m256 fdataA32x8 = _mm256_setr_ps(dataANew[k], dataANew[k+2], dataANew[k+4],dataANew[k+6],
                    dataANew[k+1], dataANew[k+3], dataANew[k+5], dataANew[k+7]);
                    __m256 fdataB32x8 = _mm256_setr_ps(dataBNew1[k], dataBNew1[k+2], dataBNew1[k+4], dataBNew1[k+6],
                    dataBNew1[k+1], dataBNew1[k+3], dataBNew1[k+5], dataBNew1[k+7]);
                    __m256 result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                    ftmp0t32x4 = _mm_add_ps(_mm_add_ps(ftmp0t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));

                    fdataB32x8 = _mm256_setr_ps(dataBNew2[k], dataBNew2[k+2], dataBNew2[k+4], dataBNew2[k+6],
                    dataBNeW2[k+1], dataBNew2[k+3], dataBNew2[k+5], dataBNew2[k+7]);
                    result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                    ftmp1t32x4 = _mm_add_ps(_mm_add_ps(ftmp1t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));

                    fdataB32x8 = _mm256_setr_ps(dataBNew3[k], dataBNew3[k+2], dataBNew3[k+4], dataBNew3[k+6],
                    dataBNeW3[k+1], dataBNew3[k+3], dataBNew3[k+5], dataBNew3[k+7]);
                    result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                    ftmp2t32x4 = _mm_add_ps(_mm_add_ps(ftmp2t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));

                    fdataB32x8 = _mm256_setr_ps(dataBNew4[k], dataBNew4[k+2], dataBNew4[k+4], dataBNew4[k+6],
                    dataBNeW4[k+1], dataBNew4[k+3], dataBNew4[k+5], dataBNew4[k+7]);
                    result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                    ftmp3t32x4 = _mm_add_ps(_mm_add_ps(ftmp3t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));
                }
            }

            dataMNew0[col] = (ftmp0t32x4[0] + ftmp0t32x4[1]) + (ftmp0t32x4[2] + ftmp0t32x4[3]);
            dataMNew0[col + 1] = (ftmp1t32x4[0] + ftmp1t32x4[1]) + (ftmp1t32x4[2] + ftmp1t32x4[3]);
            dataMNew0[col + 2] = (ftmp2t32x4[0] + ftmp2t32x4[1]) + (ftmp2t32x4[2] + ftmp2t32x4[3]);
            dataMNew0[col + 3] = (ftmp3t32x4[0] + ftmp3t32x4[1]) + (ftmp3t32x4[2] + ftmp3t32x4[3]);
        }
        else
        {
            if(k < bnumColsCnt)
            {
                __m128 ftmp0t32x4 = _mm_setzero_ps();
                __m128 ftmp1t32x4 = _mm_setzero_ps();
                __m128 ftmp2t32x4 = _mm_setzero_ps();
                __m128 ftmp3t32x4 = _mm_setzero_ps();

                for (; k < bnumColsCnt; k = k + 8)
                {
                    __m256 fdataA32x8 = _mm256_setr_ps(dataANew[k], dataANew[k+2], dataANew[k+4],dataANew[k+6],
                    dataANew[k+1], dataANew[k+3], dataANew[k+5], dataANew[k+7]);
                    __m256 fdataB32x8 = _mm256_setr_ps(dataBNew1[k], dataBNew1[k+2], dataBNew1[k+4], dataBNew1[k+6],
                    dataBNew1[k+1], dataBNew1[k+3], dataBNew1[k+5], dataBNew1[k+7]);
                    __m256 result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                    ftmp0t32x4 = _mm_add_ps(_mm_add_ps(ftmp0t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));

                    fdataB32x8 = _mm256_setr_ps(dataBNew2[k], dataBNew2[k+2], dataBNew2[k+4], dataBNew2[k+6],
                    dataBNeW2[k+1], dataBNew2[k+3], dataBNew2[k+5], dataBNew2[k+7]);
                    result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                    ftmp1t32x4 = _mm_add_ps(_mm_add_ps(ftmp1t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));

                    fdataB32x8 = _mm256_setr_ps(dataBNew3[k], dataBNew3[k+2], dataBNew3[k+4], dataBNew3[k+6],
                    dataBNeW3[k+1], dataBNew3[k+3], dataBNew3[k+5], dataBNew3[k+7]);
                    result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                    ftmp2t32x4 = _mm_add_ps(_mm_add_ps(ftmp2t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));

                    fdataB32x8 = _mm256_setr_ps(dataBNew4[k], dataBNew4[k+2], dataBNew4[k+4], dataBNew4[k+6],
                    dataBNeW4[k+1], dataBNew4[k+3], dataBNew4[k+5], dataBNew4[k+7]);
                    result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                    ftmp3t32x4 = _mm_add_ps(_mm_add_ps(ftmp3t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));
                }

                dataMNew0[col] = (ftmp0t32x4[0] + ftmp0t32x4[1]) + (ftmp0t32x4[2] + ftmp0t32x4[3]);
                dataMNew0[col + 1] = (ftmp1t32x4[0] + ftmp1t32x4[1]) + (ftmp1t32x4[2] + ftmp1t32x4[3]);
                dataMNew0[col + 2] = (ftmp2t32x4[0] + ftmp2t32x4[1]) + (ftmp2t32x4[2] + ftmp2t32x4[3]);
                dataMNew0[col + 3] = (ftmp3t32x4[0] + ftmp3t32x4[1]) + (ftmp3t32x4[2] + ftmp3t32x4[3]);
            }
        }

        if(k < aNumCols)
        {
            __m128 dataMVec = _mm_setzero_ps();
            for (; k < aNumCols; k++) {
                __m128 tmpVec1 = _mm_set1_ps(dataANew[k]);
                __m128 tmpVec2 = _mm_setr_ps(dataBNew1[k], dataBNew2[k], dataBNew3[k], dataBNew4[k]);
                dataMVec = _mm_add_ps(dataMVec, _mm_mul_ps(tmpVec1, tmpVec2));
            }
            _mm_store_ps(dataMNew0 + col, _mm_add_ps(_mm_load_ps(dataMNew0 + col), dataMVec));
        }
    }

    for (; col < numCols; col++)
    {
        k = 0;
        if(k < anumColsCnt)
        {
            tmp1 = 0, tmp2 = 0;
            __m128 ftmp0t32x4 = _mm_setzero_ps();

            for (; k < anumColsCnt; k = k + 16)
            {
                __m512 fdataA32x16 = _mm512_setr_ps(dataANew[k], dataANew[k+2], dataANew[k+4], dataANew[k+6],
                dataANew[k+1], dataANew[k+3], dataANew[k+5], dataANew[k+7],
                dataANew[k+8], dataANew[k+10], dataANew[k+12], dataANew[k+14],
                dataANew[k+9], dataANew[k+11], dataANew[k+13], dataANew[k+15]);
                __m512 fdataB32x16 = _mm512_setr_ps(dataBNew1[k], dataBNew1[k+2], dataBNew1[k+4], dataBNew1[k+6],
                dataBNew1[k+1], dataBNew1[k+3], dataBNew1[k+5], dataBNew1[k+7],
                dataBNew1[k+8], dataBNew1[k+10], dataBNew1[k+12], dataBNew1[k+14],
                dataBNew1[k+9], dataBNew1[k+11], dataBNew1[k+13], dataBNew1[k+15]);
                __m512 result2 = _mm512_mul_ps(fdataA32x16, fdataB32x16);
                __m128 result3 = _mm512_extractf32x4_ps(result2,0);
                __m128 result4 = _mm512_extractf32x4_ps(result2,1);
                __m128 result5 = _mm512_extractf32x4_ps(result2,2);
                __m128 result6 = _mm512_extractf32x4_ps(result2,3);
                ftmp0t32x4 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_add_ps(ftmp0t32x4,result3),result4),result5),result6);
            }

            if(k < bnumColsCnt)
            {
                for (; k < bnumColsCnt; k = k + 8) {
                    __m256 fdataA32x8 = _mm256_setr_ps(dataANew[k], dataANew[k+2], dataANew[k+4],dataANew[k+6],
                    dataANew[k+1], dataANew[k+3], dataANew[k+5], dataANew[k+7]);
                    __m256 fdataB32x8 = _mm256_setr_ps(dataBNew1[k], dataBNew1[k+2], dataBNew1[k+4], dataBNew1[k+6],
                    dataBNew1[k+1], dataBNew1[k+3], dataBNew1[k+5], dataBNew1[k+7]);
                    __m256 result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                    ftmp0t32x4 = _mm_add_ps(_mm_add_ps(ftmp0t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));
                }
            }
            tmp1 = ftmp0t32x4[0] + ftmp0t32x4[1];
            tmp2 = ftmp0t32x4[2] + ftmp0t32x4[3];
            dataMNew0[col] = tmp1 + tmp2;
        }
        else
        {
            if(k < bnumColsCnt)
            {
                tmp1 = 0, tmp2 = 0;
                __m128 ftmp0t32x4 = _mm_setzero_ps();

                for (; k < bnumColsCnt; k = k + 8)
                {
                    __m256 fdataA32x8 = _mm256_setr_ps(dataANew[k], dataANew[k+2], dataANew[k+4],dataANew[k+6],
                    dataANew[k+1], dataANew[k+3], dataANew[k+5], dataANew[k+7]);
                    __m256 fdataB32x8 = _mm256_setr_ps(dataBNew1[k], dataBNew1[k+2], dataBNew1[k+4], dataBNew1[k+6],
                    dataBNew1[k+1], dataBNew1[k+3], dataBNew1[k+5], dataBNew1[k+7]);
                    __m256 result1 = _mm256_mul_ps(fdataA32x8, fdataB32x8);
                    ftmp0t32x4 = _mm_add_ps(_mm_add_ps(ftmp0t32x4, _mm256_extractf128_ps(result1, 0)), _mm256_extractf128_ps(result1, 1));
                }
                tmp1 = ftmp0t32x4[0] + ftmp0t32x4[1];
                tmp2 = ftmp0t32x4[2] + ftmp0t32x4[3];
                dataMNew0[col] = tmp1 + tmp2;
            }
        }

        if(k < aNumCols)
        {
            float temp = 0.0;
            for (; k < aNumCols; k++) {
                temp += dataANew[k] * dataB[col * bStride + k];
            }
            dataMNew0[col] += temp;
        }
    }
}

void MatrixMultGemmAvx512(
    const MatrixStruct *matA,
    const MatrixStruct *matB,
    MatrixHandleUnconst matM)
{
    int numRows = matM->row;
    int matRow;
    for (matRow = 0; matRow < numRows; matRow++) {
        MatrixVecMultAvx512(matRow, matA, matB, matM);
    }
}
#endif

#if defined(_NEON) && defined(SUPPORT_NEON)
// Matrix vector mult function
void MatrixVecMultNeon(
    int matRow,
    const MatrixStruct *matA,
    const MatrixStruct *matB,
    MatrixHandleUnconst matM)
{
    const float *dataA = matA->data;
    int aNumCols = matA->col;
    int aStride = matA->col;
    const float *dataB = matB->data;
    int bStride = matB->col;
    float *dataM = matM->data;
    int numCols = matM->col;

    int numColsCnt = numCols - 3;
    int anumColsCnt = aNumCols - 7;
    int stride = numCols;

    float32x4_t fdataA32x4, fdataB32x4, ftmp0t32x4, ftmp1t32x4, ftmp2t32x4, ftmp3t32x4;
    float32x4_t fadd1, fadd2, tmpVec1, tmpVec2, dataMVec;
    float32x4x2_t faddVec1, faddVec2, faddVecTmp1, faddVecTmp2, fdataA32x4x2, fdataB32x4x2;

    int col, k;
    float tmp1, tmp2;
    float tmpArr1[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float *dataANew = dataA + matRow * aStride;
    float *dataMNew0 = dataM + matRow * stride;

    for (col = 0; col < numColsCnt; col = col + 4)
    {
        k = 0;
        const float *dataBNew1 = dataB + col * bStride;
        const float *dataBNew2 = dataBNew1 + bStride;
        const float *dataBNew3 = dataBNew2 + bStride;
        const float *dataBNew4 = dataBNew3 + bStride;
        if(k < anumColsCnt)
        {
            ftmp0t32x4 = vdupq_n_f32(0);
            ftmp1t32x4 = vdupq_n_f32(0);
            ftmp2t32x4 = vdupq_n_f32(0);
            ftmp3t32x4 = vdupq_n_f32(0);
            
            for (; k < anumColsCnt; k = k + 8) {
                fdataA32x4x2 = vld2q_f32(dataANew + k);
                fdataB32x4x2 = vld2q_f32(dataBNew1 + k);
                ftmp0t32x4 = vmlaq_f32(ftmp0t32x4, fdataA32x4x2.val[0], fdataB32x4x2.val[0]);
                ftmp0t32x4 = vmlaq_f32(ftmp0t32x4, fdataA32x4x2.val[1], fdataB32x4x2.val[1]);
                fdataB32x4x2 = vld2q_f32(dataBNew2 + k);
                ftmp1t32x4 = vmlaq_f32(ftmp1t32x4, fdataA32x4x2.val[0], fdataB32x4x2.val[0]);
                ftmp1t32x4 = vmlaq_f32(ftmp1t32x4, fdataA32x4x2.val[1], fdataB32x4x2.val[1]);
                fdataB32x4x2 = vld2q_f32(dataBNew3 + k);
                ftmp2t32x4 = vmlaq_f32(ftmp2t32x4, fdataA32x4x2.val[0], fdataB32x4x2.val[0]);
                ftmp2t32x4 = vmlaq_f32(ftmp2t32x4, fdataA32x4x2.val[1], fdataB32x4x2.val[1]);
                fdataB32x4x2 = vld2q_f32(dataBNew4 + k);
                ftmp3t32x4 = vmlaq_f32(ftmp3t32x4, fdataA32x4x2.val[0], fdataB32x4x2.val[0]);
                ftmp3t32x4 = vmlaq_f32(ftmp3t32x4, fdataA32x4x2.val[1], fdataB32x4x2.val[1]);
            }

            faddVec1 = vtrnq_f32(ftmp0t32x4, ftmp2t32x4);
            faddVec2 = vtrnq_f32(ftmp1t32x4, ftmp3t32x4);
            faddVecTmp1 = vzipq_f32(faddVec1.val[0], faddVec2.val[0]);
            faddVecTmp2 = vzipq_f32(faddVec1.val[1], faddVec2.val[1]);
            fadd1 = vaddq_f32(faddVecTmp1.val[0], faddVecTmp2.val[0]);
            fadd2 = vaddq_f32(faddVecTmp1.val[1], faddVecTmp2.val[1]);
            vst1q_f32(dataMNew0 + col, vaddq_f32(fadd1, fadd2));
        }

        if(k < aNumCols)
        {
            dataMVec = vdupq_n_f32(0);
            for (; k < aNumCols; k++) {
                tmpVec1 = vdupq_n_f32(dataANew[k]);
                tmpArr1[0] = dataBNew1[k];
                tmpArr1[1] = dataBNew2[k];
                tmpArr1[2] = dataBNew3[k];
                tmpArr1[3] = dataBNew4[k];
                tmpVec2 = vld1q_f32(tmpArr1);
                dataMVec = vaddq_f32(dataMVec, vmulq_f32(tmpVec1, tmpVec2));
            }
            vst1q_f32(dataMNew0 + col, vaddq_f32(vld1q_f32(dataMNew0 + col), dataMVec));
        }
    }

    for (; col < numCols; col++) 
    {
        k = 0;
        if(k < anumColsCnt)
        {
            tmp1 = 0, tmp2 = 0;
            ftmp0t32x4 = vdupq_n_f32(0);
            for (; k < anumColsCnt; k = k + 8) {
                fdataA32x4x2 = vld2q_f32(dataANew + k);
                fdataB32x4x2 = vld2q_f32(dataB + col * bStride + k);
                ftmp0t32x4 = vmlaq_f32(ftmp0t32x4, fdataA32x4x2.val[0], fdataB32x4x2.val[0]);
                ftmp0t32x4 = vmlaq_f32(ftmp0t32x4, fdataA32x4x2.val[1], fdataB32x4x2.val[1]);
            }
            tmp1 = vgetq_lane_f32(ftmp0t32x4, 0) + vgetq_lane_f32(ftmp0t32x4, 1);
            tmp2 = vgetq_lane_f32(ftmp0t32x4, 2) + vgetq_lane_f32(ftmp0t32x4, 3);
            dataMNew0[col] = tmp1 + tmp2;
        }

        if(k < aNumCols)
        {
            float temp = 0.0;
            for (; k < aNumCols; k++) {
                temp += dataANew[k] * dataB[col * bStride + k];
            }
            dataMNew0[col] += temp;
        }
    }
}

// GEMM matrix mult function
void MatrixMultGemmNeon(
    const MatrixStruct *matA,
    const MatrixStruct *matB,
    MatrixHandleUnconst matM)
{
    int numRows = matM->row;
    int matRow;
    for (matRow = 0; matRow < numRows; matRow++) {
        MatrixVecMultNeon(matRow, matA, matB, matM);
    }
}
#endif