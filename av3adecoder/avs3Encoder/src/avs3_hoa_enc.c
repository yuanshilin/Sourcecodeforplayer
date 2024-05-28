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
#include <assert.h>
#include <math.h>
#include <float.h>
#include "avs3_debug.h"
#include "avs3_stat_com.h"
#include "avs3_stat_enc.h"
#include "avs3_prot_com.h"
#include "avs3_prot_enc.h"
#include "avs3_rom_com.h"

static float VecMean(const float* x, const short len)
{
    short i;
    float tmp = 0.f;

    if (x == NULL || len < 1)
    {
        return tmp;
    }

    for (i = 0; i < len; i++)
    {
        tmp += x[i];
    }

    tmp /= len;

    return tmp;
}

static short VecCompareWithZero(float* vec, const short len)
{
    assert(vec != NULL && len >= 1);

    float delta = 0.f;

    for (short i = 0; i < len; i++)
    {
        delta = (float)fabs(vec[i] - 0.f);

        if (delta > AVS3_EPSILON)
        {
            return AVS3_FALSE;
        }
    }

    return AVS3_TRUE;
}

static void VecArgSort(float* vec, const short len1, short* res, const short len2)
{
    assert(vec != NULL && len1 >= 1);
    assert(len1 >= len2);

    short i;
    short count = 0;

    while (count < len2)
    {
        float max_value = vec[0];
        short maxIdx = 0;

        for (i = 0; i < len1; i++)
        {
            if (max_value < vec[i])
            {
                max_value = vec[i];
                maxIdx = i;
            }
        }

        vec[maxIdx] = -AVS3_EPSILON;
        res[count] = maxIdx;

        count++;
    }

    return;
}

static void VecArgPointSort(HOA_Point* vecPoints, const short lenPoints, const short firstN)
{
    short i, j;
    float maxValue;
    short maxIdx = 0;
    short tmpIdx = 0;
    float tmpVal = 0.f;

    for (i = 0; i < firstN; i++)
    {
        maxIdx = i;
        maxValue = vecPoints[i].value;

        for (j = i + 1; j < lenPoints; j++)
        {
            if (maxValue < vecPoints[j].value)
            {
                maxIdx = j;
                maxValue = vecPoints[j].value;
            }
        }

        if (maxIdx == i)
        {
            continue;
        }

        tmpVal = vecPoints[i].value;
        tmpIdx = vecPoints[i].idx;

        vecPoints[i].value = vecPoints[maxIdx].value;
        vecPoints[i].idx = vecPoints[maxIdx].idx;

        vecPoints[maxIdx].value = tmpVal;
        vecPoints[maxIdx].idx = tmpIdx;
    }

    return;
}

static short VecArgAbsMax(float* vec, const short len)
{
    assert(vec != NULL && len >= 1);

    short maxIdx = 0;

    if (len == 1)
    {
        return maxIdx;
    }

    for (short i = 1; i < len; i++)
    {
        if ((float)fabs(vec[maxIdx]) < (float)fabs(vec[i]))
        {
            maxIdx = i;
        }
    }

    return maxIdx;
}


static void HoaSceneAnalysis(AVS3_HOA_ENC_DATA_HANDLE hEncHoa)
{
    short i, ch;
    short n = (HOA_LEN_TRANSFORM) / 192;
    float hoaSignalBufferSub[MAX_HOA_CHANNELS][(HOA_LEN_TRANSFORM) / 192];
    float matU[L_HOA_BASIS_ROWS][L_HOA_BASIS_ROWS];
    float matVt[L_HOA_BASIS_ROWS][L_HOA_BASIS_ROWS];
    float vecS[L_HOA_BASIS_ROWS];
    float vecTmp[(HOA_LEN_TRANSFORM) / 192 - 1];

    AVS3_HOA_CONFIG_DATA_HANDLE hHoaConfig = hEncHoa->hHoaConfig;
    for (ch = 0; ch < hHoaConfig->nTotalChansInput; ch++)
    {
        for (i = 0; i < n; i++)
        {
            hoaSignalBufferSub[ch][i] = hEncHoa->hoaSignalBuffer[ch][i * 192];

            if (fabs(hoaSignalBufferSub[ch][i]) < 3.0f)
            {
                hoaSignalBufferSub[ch][i] = 0.f;
            }
        }
    }

    for (i = 0; i < L_HOA_BASIS_ROWS; i++)
    {
        SetZero(matU[i], L_HOA_BASIS_ROWS);
        SetZero(matVt[i], L_HOA_BASIS_ROWS);
    }

    SetZero(vecS, L_HOA_BASIS_ROWS);

    for (ch = 0; ch < hHoaConfig->nTotalChansInput; ch++)
    {
        for (i = 0; i < n; i++)
        {
            matU[i][ch] = hoaSignalBufferSub[ch][i];
        }
    }

    Avs3HoaSVD(matU, n, n, vecS, matVt);

    if (vecS[0] == 0)
    {
        hEncHoa->numSource = 0;
    }
    else
    {
        for (i = 0; i < n - 1; i++)
        {
            vecTmp[i] = (vecS[i]) / (vecS[i + 1] + AVS3_EPSILON);
        }

        if (vecTmp[0] > 1.f || vecTmp[1] > 1.f)
        {
            hEncHoa->numSource = 8;
        }

        for (i = 0; i < n; i++)
        {
            if (vecTmp[i] >= 100.f)
            {
                hEncHoa->numSource = i + 1;
                break;
            }
        }
    }

    return;
}


static void HoaSignalMdctAnalysis(AVS3_HOA_ENC_DATA_HANDLE hEncHoa)
{
    short i, ch;
    float win[BLOCK_LEN_LONG];
    float* signalInput = NULL;

    AVS3_HOA_CONFIG_DATA_HANDLE hHoaConfig = hEncHoa->hHoaConfig;
    const short overlapSize = hHoaConfig->overlapSize;

    for (ch = 0; ch < hHoaConfig->nTotalChansInput; ch++)
    {
        signalInput = hEncHoa->hoaSignalBuffer[ch];

        for (short block = 0; block < N_BLOCK_HOA; block++)
        {
            SetZero(win, 2 * overlapSize);

            /* windowing left part */
            for (i = 0; i < overlapSize; i++)
            {
                win[i] = signalInput[i] * hHoaConfig->hoaWindow[i];
            }

            /* window right part */
            for (i = 0; i < overlapSize; i++)
            {
                win[i + overlapSize] = signalInput[i + overlapSize] * hHoaConfig->hoaWindow[overlapSize - i - 1];
            }

            MDCT(win, hEncHoa->origSpecturm[ch] + block * overlapSize, 2 * overlapSize);

            signalInput += overlapSize;
        }
    }

    return;
}


static void HoaGetBasisCoefs(
    AVS3_HOA_ENC_DATA_HANDLE hEncHoa,
    float matBasisCoefs[L_HOA_BASIS_ROWS][MAX_HOA_BASIS],
    const short lenFrame
)
{
    short i, j, k, n, ch, cols, rows;
    AVS3_HOA_CONFIG_DATA_HANDLE hConfig = hEncHoa->hHoaConfig;

    short cntVote = 0;
    short firstStepProjMaxIdx = 0;
    short secondStepProjMaxIdx = 0;
    const short numPoints = (short)(lenFrame * 0.2f);
    const short numChansInput = hConfig->nTotalChansInput;

    float spectrum[MAX_HOA_CHANNELS][HOA_LEN_FRAME48k];
    float samples[L_HOA_BASIS_ROWS];
    float vecBasis[L_HOA_BASIS_ROWS];
    float projectBasis[L_FIRST_ORDER_HOA_BASIS];
    float * frameVote = hEncHoa->frameVote;
    short basisIdx[MAX_HOA_BASIS];
    short firstBasisIdx[L_SECOND_ORDER_MP_BASIS];
    float neighborsProject[L_SECOND_ORDER_MP_BASIS];
    short matAnglePair[L_SECOND_ORDER_MP_BASIS][2];
    float neighborsBasisCoeffs[L_SECOND_ORDER_MP_BASIS][MAX_HOA_CHANNELS];

    float firstBasisProject[L_FIRST_ORDER_HOA_BASIS];
    short anglePair[2];

    SetZero(samples, L_HOA_BASIS_ROWS);
    SetZero(vecBasis, L_HOA_BASIS_ROWS);
    SetZero(firstBasisProject, L_FIRST_ORDER_HOA_BASIS);
    SetZero(projectBasis, L_FIRST_ORDER_HOA_BASIS);
    SetShort(firstBasisIdx, 0, L_SECOND_ORDER_MP_BASIS);

    short reuseFlag;
    short idx[MAX_HOA_BASIS];
    short lastMaxIdx = 0;
    float lastBasisProject[MAX_HOA_BASIS];

    const short numProjBasis = numChansInput;

    for (ch = 0; ch < numChansInput; ch++)
    {
        Mvf2f(hEncHoa->origSpecturm[ch], spectrum[ch], lenFrame);

        MdctSpectrumInterleave(spectrum[ch], lenFrame, N_BLOCK_HOA);
    }

    /* Step1: get typical samples */
    for (i = 0; i < numPoints; i++)
    {
        for (j = 0; j < numChansInput; j++)
        {
            samples[j] += spectrum[j][i];
        }
    }

    /* Step2: get the best VL and its projection from last frame */
    for (i = 0; i < MAX_HOA_BASIS; i++)
    {
        idx[i] = hEncHoa->basisIdx[i];
    }

    for (i = 0; i < MAX_HOA_BASIS; i++)
    {
        MvShort2Short(avs3_hoa_fixed_angle_basis_matrix[idx[i]], anglePair, 2);
        GetSingleNeighborBasisCoeff(anglePair, vecBasis);

        lastBasisProject[i] = (float)Dotp(samples, vecBasis, numProjBasis);
    }

    lastMaxIdx = VecArgAbsMax(lastBasisProject, MAX_HOA_BASIS);

    /* Step3: get first step VL projection from this frame */
    for (cols = 0; cols < L_FIRST_ORDER_HOA_BASIS; cols++)
    {
        for (rows = 0; rows < numProjBasis; rows++)
        {
            vecBasis[rows] = avs3_hoa_first_step_sample_point_basis_matrix[rows][cols];
        }
        firstBasisProject[cols] = (float)Dotp(samples, vecBasis, numProjBasis);
    }
    firstStepProjMaxIdx = VecArgAbsMax(firstBasisProject, L_FIRST_ORDER_HOA_BASIS);

    /* Step4: first decision based on first step VL projection */
    if (fabs(firstBasisProject[firstStepProjMaxIdx]) > fabs(lastBasisProject[lastMaxIdx]))      // Need re-search to get accurate VL
    {
        reuseFlag = 0;
    }
    else
    {
        /* Step5: get the second step VL projection */
        MvShort2Short(avs3_hoa_sample_point_basis_matrix_second_step[firstStepProjMaxIdx], firstBasisIdx, L_SECOND_ORDER_MP_BASIS);
        for (k = 0; k < L_SECOND_ORDER_MP_BASIS; k++)
        {
            MvShort2Short(avs3_hoa_fixed_angle_basis_matrix[firstBasisIdx[k]], matAnglePair[k], 2);
        }
        GetNeighborBasisCoeff(matAnglePair, neighborsBasisCoeffs);

        for (cols = 0; cols < L_SECOND_ORDER_MP_BASIS; cols++)
        {
            for (rows = 0; rows < numProjBasis; rows++)
            {
                vecBasis[rows] = neighborsBasisCoeffs[cols][rows];
            }
            neighborsProject[cols] = (float)Dotp(samples, vecBasis, numProjBasis);
        }
        secondStepProjMaxIdx = VecArgAbsMax(neighborsProject, L_SECOND_ORDER_MP_BASIS);
        /* Step6: second decision based on second step VL projection */
        if (fabs(neighborsProject[secondStepProjMaxIdx]) > fabs(lastBasisProject[lastMaxIdx]))      // Need re-search to get accurate VL 
        {
            reuseFlag = 0;
        }
        else
        {
            reuseFlag = 1;
        }
    }

    if (reuseFlag == 0)
    {
        /* pre-processing: sort spectrum points */
        for (i = 0; i < lenFrame; i++)
        {
            for (j = 0; j < numChansInput; j++)
            {
                samples[j] = spectrum[j][i];
            }

            hEncHoa->points[i].idx = i;
            hEncHoa->points[i].value = VLinalgNorm(samples, numProjBasis);
        }

        float w1 = 0;
        float w2 = 0;
        float w3;
        HOA_Point pointsLo[HOA_LEN_FRAME48k];

        for (k = 1; k < HOA_MAX_SFB; k++)
        {
            for (i = avs3_mp_vl_sfb[k - 1]; i < avs3_mp_vl_sfb[k]; i++)
            {
                pointsLo[i - avs3_mp_vl_sfb[k - 1]] = hEncHoa->points[i];
            }

            VecArgPointSort(pointsLo, avs3_mp_vl_sfb[k] - avs3_mp_vl_sfb[k - 1], (avs3_mp_vl_sfb[k] - avs3_mp_vl_sfb[k - 1]) / 2);

            for (i = 0; i < (avs3_mp_vl_sfb[k] - avs3_mp_vl_sfb[k - 1]) / 2; i++)
            {
                w1 += pointsLo[i].value;
                w2 += pointsLo[i + (avs3_mp_vl_sfb[k] - avs3_mp_vl_sfb[k - 1]) / 2].value;
            }
            w3 = w1 / (w1 + w2);

            for (i = avs3_mp_vl_sfb[k - 1] + (avs3_mp_vl_sfb[k] - avs3_mp_vl_sfb[k - 1]) / 2; i < avs3_mp_vl_sfb[k]; i++)
            {
                pointsLo[i - avs3_mp_vl_sfb[k - 1]].value = 0;
            }

            for (i = avs3_mp_vl_sfb[k - 1]; i < avs3_mp_vl_sfb[k]; i++)
            {
                hEncHoa->points[i].idx = pointsLo[i - avs3_mp_vl_sfb[k - 1]].idx;
                hEncHoa->points[i].value = w3 * pointsLo[i - avs3_mp_vl_sfb[k - 1]].value;
            }
        }

        VecArgPointSort(hEncHoa->points, lenFrame, numPoints);

        for (i = 0; i < numPoints; i++)
        {
            short point_idx = hEncHoa->points[i].idx;

            for (j = 0; j < numChansInput; j++)
            {
                samples[j] = spectrum[j][point_idx];
            }

            if (VecCompareWithZero(samples, numChansInput) == AVS3_TRUE)
            {
                continue;
            }

            cntVote = 0;

            while (cntVote < min(hEncHoa->numVote, hEncHoa->numSource))
            {
                float tmp = 0.f;

                /* get first step VL coefficients. */
                for (cols = 0; cols < L_FIRST_ORDER_HOA_BASIS; cols++)
                {
                    for (rows = 0; rows < numProjBasis; rows++)
                    {
                        vecBasis[rows] = avs3_hoa_first_step_sample_point_basis_matrix[rows][cols];
                    }

                    firstBasisProject[cols] = (float)Dotp(samples, vecBasis, numProjBasis);
                }

                /* find first step max projection index. */
                firstStepProjMaxIdx = VecArgAbsMax(firstBasisProject, L_FIRST_ORDER_HOA_BASIS);

                /* find out the neighbors basis index. */
                MvShort2Short(avs3_hoa_sample_point_basis_matrix_second_step[firstStepProjMaxIdx], firstBasisIdx, L_SECOND_ORDER_MP_BASIS);

                /* covert basis to azimuth and elevation angles pair. */
                for (k = 0; k < L_SECOND_ORDER_MP_BASIS; k++)
                {
                    MvShort2Short(avs3_hoa_fixed_angle_basis_matrix[firstBasisIdx[k]], matAnglePair[k], 2);
                }

                /* compute VL coefficients. */
                GetNeighborBasisCoeff(matAnglePair, neighborsBasisCoeffs);

                for (cols = 0; cols < L_SECOND_ORDER_MP_BASIS; cols++)
                {
                    for (rows = 0; rows < numProjBasis; rows++)
                    {
                        vecBasis[rows] = neighborsBasisCoeffs[cols][rows];
                    }

                    neighborsProject[cols] = (float)Dotp(samples, vecBasis, numProjBasis);
                }

                secondStepProjMaxIdx = VecArgAbsMax(neighborsProject, L_SECOND_ORDER_MP_BASIS);

                frameVote[firstBasisIdx[secondStepProjMaxIdx]] += (float)fabs(neighborsProject[secondStepProjMaxIdx]);
                tmp = VLinalgNorm(neighborsBasisCoeffs[secondStepProjMaxIdx], numProjBasis) + AVS3_EPSILON;

                for (n = 0; n < numProjBasis; n++)
                {
                    samples[n] -= neighborsBasisCoeffs[secondStepProjMaxIdx][n] / tmp * neighborsProject[secondStepProjMaxIdx];
                }

                cntVote++;
            }
        }

        if (VecCompareWithZero(frameVote, L_HOA_BASIS_COLS) != AVS3_TRUE)
        {
            float tmp_frame_vote[L_HOA_BASIS_COLS];
            Mvf2f(frameVote, tmp_frame_vote, L_HOA_BASIS_COLS);
            VecArgSort(tmp_frame_vote, L_HOA_BASIS_COLS, basisIdx, hEncHoa->numVote);
        }
        else
        {
            for (i = 0; i < hEncHoa->numVote; i++)
            {
                basisIdx[i] = L_HOA_BASIS_COLS - i - 1;
            }
        }

        MvShort2Short(basisIdx, hEncHoa->basisIdx, hEncHoa->numVote);

        if (hEncHoa->numSource > 6)
        {
            SetZero(frameVote, L_HOA_BASIS_COLS);
        }
        else if (hEncHoa->numSource > 0 && hEncHoa->numSource < 7)
        {
            for (i = 0; i < L_HOA_BASIS_COLS; i++)
            {
                frameVote[i] *= hEncHoa->numSource * 0.12f;
            }
        }
        else
        {
            for (i = 0; i < L_HOA_BASIS_COLS; i++)
            {
                frameVote[i] *= 0.2f;// 0.5f;
            }
        }


        float tempFrameVote[MAX_HOA_BASIS];
        for (i = 0; i < hEncHoa->numVote; i++)
        {
            tempFrameVote[i] = frameVote[basisIdx[i]];
        }
        SetFloat(frameVote, 0.f, L_HOA_BASIS_COLS);

        for (i = 0; i < hEncHoa->numVote; i++)
        {
            frameVote[basisIdx[i]] = tempFrameVote[i];
        }

        /* sort VL basis */
        SortS(hEncHoa->basisIdx, hEncHoa->numVote);
    }

    for (k = 0; k < hEncHoa->numVote; k++)
    {
        short idx = hEncHoa->basisIdx[k];

        MvShort2Short(avs3_hoa_fixed_angle_basis_matrix[idx], anglePair, 2);

        GetSingleNeighborBasisCoeff(anglePair, vecBasis);

        for (i = 0; i < numProjBasis; i++)
        {
            matBasisCoefs[i][k] = vecBasis[i];
        }
    }

    return;
}


static void HoaComputeParams(AVS3_HOA_ENC_DATA_HANDLE hEncHoa,
    const float matBasisCoefs[L_HOA_BASIS_ROWS][MAX_HOA_BASIS],
    float invMatCoefs[MAX_HOA_BASIS][L_HOA_BASIS_ROWS]
)
{
    short i, j, k;
    short row, cols;

    const short m = hEncHoa->hHoaConfig->nTotalChansInput;
    const short n = hEncHoa->numVote;

    float matU[L_HOA_BASIS_ROWS][L_HOA_BASIS_ROWS];
    float matVt[L_HOA_BASIS_ROWS][L_HOA_BASIS_ROWS];
    float vecS[L_HOA_BASIS_ROWS];
    float vecTmp[L_HOA_BASIS_ROWS];

    for (i = 0; i < m; i++)
    {
        SetZero(matU[i], L_HOA_BASIS_ROWS);
        SetZero(matVt[i], L_HOA_BASIS_ROWS);
    }

    SetZero(vecS, L_HOA_BASIS_ROWS);

    for (row = 0; row < m; row++)
    {
        for (cols = 0; cols < n; cols++)
        {
            matU[row][cols] = matBasisCoefs[row][cols];
        }
    }

    Avs3HoaSVD(matU, m, n, vecS, matVt);

    for (i = 0; i < n; i++)
    {
        short mask = AVS3_FALSE;

        if ((vecS[0] * HOA_RCOND) <= vecS[i])
        {
            mask = AVS3_TRUE;
            vecS[i] = 1.f / (vecS[i] + AVS3_EPSILON);
        }

        for (j = 0; j < n; j++)
        {
            if (mask)
            {
                matVt[j][i] = matVt[j][i] * vecS[i];
            }
            else
            {
                matVt[j][i] = 0.f;
            }
        }
    }

    for (i = 0; i < n; i++)
    {
        for (j = 0; j < m; j++)
        {
            for (k = 0; k < n; k++)
            {
                vecTmp[k] = matU[j][k];
            }

            invMatCoefs[i][j] = Dotp(vecTmp, matVt[i], n);
        }
    }

    return;
}

static void HoaSignalDecomposition(
    AVS3_HOA_ENC_DATA_HANDLE hEncHoa,
    float specOutput[MAX_HOA_CHANNELS][HOA_LEN_FRAME48k],
    const float matBasisCoefs[L_HOA_BASIS_ROWS][MAX_HOA_BASIS],
    const float invMatCoefs[MAX_HOA_BASIS][L_HOA_BASIS_ROWS],
    const short lenFrame
)
{
    short row, cols, k;
    AVS3_HOA_CONFIG_DATA_HANDLE hConfig;
    float recoverySpectrum[MAX_HOA_CHANNELS][HOA_LEN_FRAME48k];
    float resSpectrum[MAX_HOA_CHANNELS][HOA_LEN_FRAME48k];
    float signalMatBasis[HOA_LEN_FRAME48k][MAX_HOA_BASIS];
    float samples[L_HOA_BASIS_ROWS];
    float tmp1[L_HOA_BASIS_ROWS];
    float tmp2[L_HOA_BASIS_ROWS];
    short nTotalTransportChans;
    short numChansInput;
    short numBasis;
    float origNrg[MAX_HOA_CHANNELS];
    float recoveryNrg[MAX_HOA_CHANNELS];

    hConfig = hEncHoa->hHoaConfig;
    nTotalTransportChans = hConfig->nTotalForeChans + hConfig->nTotalResChans;
    numChansInput = hConfig->nTotalChansInput;
    numBasis = hEncHoa->numVote;

    SetZero(samples, L_HOA_BASIS_ROWS);
    SetZero(tmp1, L_HOA_BASIS_ROWS);
    SetZero(tmp2, L_HOA_BASIS_ROWS);

    for (cols = 0; cols < lenFrame; cols++)
    {
        for (row = 0; row < numChansInput; row++)
        {
            samples[row] = hEncHoa->origSpecturm[row][cols];
        }

        for (k = 0; k < numBasis; k++)
        {
            signalMatBasis[cols][k] = (float)Dotp(samples, invMatCoefs[k], numChansInput);
        }
    }

    /* foreground signals. */
    for (row = 0; row < numBasis; row++)
    {
        for (cols = 0; cols < lenFrame; cols++)
        {
            specOutput[row][cols] = signalMatBasis[cols][row];
        }
    }

    /* recovery signals. */
    for (row = 0; row < lenFrame; row++)
    {
        for (cols = 0; cols < numBasis; cols++)
        {
            tmp1[cols] = signalMatBasis[row][cols];
        }

        for (k = 0; k < numChansInput; k++)
        {
            for (cols = 0; cols < numBasis; cols++)
            {
                tmp2[cols] = matBasisCoefs[k][cols];
            }

            recoverySpectrum[k][row] = (float)Dotp(tmp1, tmp2, numBasis);
        }
    }

    /* residual/background signals. */
    for (row = 0; row < numChansInput; row++)
    {
        for (cols = 0; cols < lenFrame; cols++)
        {
            resSpectrum[row][cols] = hEncHoa->origSpecturm[row][cols] - recoverySpectrum[row][cols];
        }
    }

    for (row = 0; row < nTotalTransportChans; row++)
    {
        for (cols = 0; cols < lenFrame; cols++)
        {
            specOutput[row + numBasis][cols] = resSpectrum[row][cols];
        }
    }

    for (row = 0; row < hEncHoa->hHoaConfig->nTotalChansInput; row++)
    {
        recoveryNrg[row] = AVS3_MAX(VLinalgNorm(recoverySpectrum[row], lenFrame), AVS3_EPSILON);
        origNrg[row] = AVS3_MAX(VLinalgNorm(hEncHoa->origSpecturm[row], lenFrame), AVS3_EPSILON);
    }

    hEncHoa->direcSignalNrgRatio = SumFloat(recoveryNrg, hEncHoa->hHoaConfig->nTotalChansInput) / SumFloat(origNrg, hEncHoa->hHoaConfig->nTotalChansInput);

    if (hEncHoa->direcSignalNrgRatio > 1.f)
    {
        hEncHoa->direcSignalNrgRatio = 0.99f;
    }

    return;
}


static void HoaVLPostFiltetr(
    AVS3_HOA_ENC_DATA_HANDLE hEncHoa,
    float specOutput[MAX_HOA_CHANNELS][HOA_LEN_FRAME48k],
    float matBasisCoefs[L_HOA_BASIS_ROWS][MAX_HOA_BASIS],
    float invMatCoefs[MAX_HOA_BASIS][L_HOA_BASIS_ROWS],
    const short lenFrame
)
{
    float nrgRatio = hEncHoa->direcSignalNrgRatio;
    float vecBasis[L_HOA_BASIS_ROWS];
    short anglePair[2];
    short k;
    short i;

    hEncHoa->nrgRatioMax = (nrgRatio + hEncHoa->nrgRatioMax) / 2;

    if (nrgRatio < 0.55f)
    {
        /* Update virtual loudspeaker and transport signal */
        SetZero(hEncHoa->frameVote, L_HOA_BASIS_COLS);

        if (nrgRatio < 0.45f)
        {
            hEncHoa->basisIdx[0] = 100;
            hEncHoa->basisIdx[1] = 200;
        }
        else
        {
            if (nrgRatio > hEncHoa->nrgRatioMax / 2)
            {
                hEncHoa->nrgRatioMax = 0;
                //hEncHoa->nrgRatioMax = nrgRatio;
                hEncHoa->basisIdxMax[0] = hEncHoa->basisIdx[0];
                hEncHoa->basisIdxMax[1] = hEncHoa->basisIdx[1];
            }
            hEncHoa->basisIdx[0] = hEncHoa->basisIdxMax[0];
            hEncHoa->basisIdx[1] = hEncHoa->basisIdxMax[1];
        }

        for (k = 0; k < hEncHoa->numVote; k++)
        {
            short idx = hEncHoa->basisIdx[k];

            MvShort2Short(avs3_hoa_fixed_angle_basis_matrix[idx], anglePair, 2);

            GetSingleNeighborBasisCoeff(anglePair, vecBasis);

            for (i = 0; i < hEncHoa->hHoaConfig->nTotalChansInput; i++)
            {
                matBasisCoefs[i][k] = vecBasis[i];
            }
        }

        /* HOA signal decomposition */
        HoaComputeParams(hEncHoa, matBasisCoefs, invMatCoefs);

        /* Original signals to transport signals */
        HoaSignalDecomposition(hEncHoa, specOutput, matBasisCoefs, invMatCoefs, lenFrame);
    }

    return;
}


static void HoaSynthCurrentFrame(
    AVS3_HOA_ENC_DATA_HANDLE hEncHoa,
    float spec[MAX_HOA_CHANNELS][HOA_LEN_FRAME48k],
    float output[MAX_CHANNELS][MAX_FRAME_LEN],
    const short lenFrame
)
{
    short i, ch;
    float wtdaSignal[BLOCK_LEN_LONG];
    float synthBuffer[HOA_OVERLAP_SIZE];

    AVS3_HOA_CONFIG_DATA_HANDLE hHoaConfig = hEncHoa->hHoaConfig;
    const short overlapSize = hHoaConfig->overlapSize;

    short nSynthChans = hHoaConfig->nTotalForeChans + hHoaConfig->nTotalResChans;

    for (ch = 0; ch < nSynthChans; ch++)
    {
        SetZero(wtdaSignal, 2 * overlapSize);

        Mvf2f(hEncHoa->synthBuffer[ch], synthBuffer, overlapSize);

        for (short block = 0; block < N_BLOCK_HOA; block++)
        {
            Mvf2f(spec[ch] + block * overlapSize, wtdaSignal, overlapSize);

            IMDCT(wtdaSignal, 2 * overlapSize);

            /* windowing left part */
            for (i = 0; i < overlapSize; i++)
            {
                wtdaSignal[i] *= hHoaConfig->hoaWindow[i];
            }

            /* windowing right part */
            for (i = 0; i < overlapSize; i++)
            {
                wtdaSignal[i + overlapSize] *= hHoaConfig->hoaWindow[overlapSize - i - 1];
            }

            Vadd(wtdaSignal, synthBuffer, wtdaSignal, overlapSize);

            Mvf2f(wtdaSignal + overlapSize, synthBuffer, overlapSize);

            Mvf2f(wtdaSignal, output[ch] + block * overlapSize, overlapSize);
        }

        Mvf2f(synthBuffer, hEncHoa->synthBuffer[ch], overlapSize);

        Mvf2f(synthBuffer, output[ch] + lenFrame, overlapSize);
    }
}

static short CheckHoaVLBasis(AVS3_HOA_ENC_DATA_HANDLE hEncHoa)
{
    short i;

    if (hEncHoa->hHoaConfig->spatialAnalysis)
    {
        for (i = 0; i < hEncHoa->numVote; i++)
        {
            if (hEncHoa->basisIdx[i] != hEncHoa->lastBasisIdx[i])
            {
                return AVS3_FALSE;
            }
        }
    }
    else
    {
        return AVS3_FALSE;
    }

    return AVS3_TRUE;
}


static void SubBandMS(float left[], float right[], const short startLines, const short endLines)
{
    short i;
    float tmp;
    const float factor = (float)(sqrt(2.f) / 2.f);

    for (i = startLines; i < endLines; i++)
    {
        tmp = left[i];
        left[i] = (left[i] + right[i]) * factor;
        right[i] = (tmp - right[i]) * factor;
    }

    return;
}


static short ChanToIndex(const short chIdx1, const short chIdx2, const short nChans)
{
    short ch1, ch2;
    short pairIdx;

    pairIdx = 0;

    for (ch2 = 1; ch2 < nChans; ch2++)
    {
        for (ch1 = 0; ch1 < ch2; ch1++)
        {
            if ((ch1 == chIdx1) && (ch2 == chIdx2))
            {
                return pairIdx;
            }
            else
            {
                pairIdx++;
            }
        }
    }

    return -1;
}

static float FindMaxCorrelation(float xCorrMatrix[MAX_HOA_DMX_CHANNELS][MAX_HOA_DMX_CHANNELS], short *maxCh1, short *maxCh2,
    const short nChans
)
{
    short ch1, ch2;
    float maxVal = 0.f;

    for (ch1 = 0; ch1 < nChans; ch1++)
    {
        for (ch2 = ch1 + 1; ch2 < nChans; ch2++)
        {
            if (maxVal < xCorrMatrix[ch1][ch2])
            {
                maxVal = xCorrMatrix[ch1][ch2];

                *maxCh1 = ch1;
                *maxCh2 = ch2;
            }
        }
    }

    return maxVal;
}


static short GetMSDecision(AVS3_HOA_ENC_DATA_HANDLE hEncHoa, const short ch1, const short ch2, short mask[])
{
    short sfb;
    short len;
    float* right = hEncHoa->coreSpectrum[ch1];
    float* left = hEncHoa->coreSpectrum[ch2];
    float nrgRight;
    float nrgLeft;
    float nrgX;
    float sfbRatio;
    short cutoffBandsFlag = 0;

    SetShort(mask, 0, N_SFB_HOA_LBR - 1);

    for (sfb = 0; sfb < N_SFB_HOA_LBR - 1; sfb++)
    {
        len = hoa_sfb_table_low_bitrate[sfb + 1] - hoa_sfb_table_low_bitrate[sfb];

        nrgRight = Dotp(right, right, len) + AVS3_EPSILON;
        nrgLeft = Dotp(left, left, len) + AVS3_EPSILON;
        nrgX = Dotp(right, left, len);

        float xCorr = nrgX / (float)sqrt(nrgRight*nrgLeft);

        sfbRatio = 10.f* (float)log10(AVS3_MAX(nrgRight, nrgLeft) / AVS3_MIN(nrgRight, nrgRight));

        if (hEncHoa->hHoaConfig->spatialAnalysis)
        {
            if ((float)fabs(xCorr) > 0.325f)
            {
                if (sfbRatio < 2.f && sfbRatio < CUTOFF_BANDS) {

                    mask[sfb] = 1;
                }
                else if (sfbRatio < 5.f && sfbRatio >= CUTOFF_BANDS)
                {
                    mask[sfb] = 1;
                }
            }
        }
        else
        {
            if ((float)fabs(xCorr) > 0.325f && sfbRatio < 5.f)
            {
                mask[sfb] = 1;
            }
        }

        right += len;
        left += len;
    }

    short cnt = 0;
    for (sfb = 0; sfb < N_SFB_HOA_LBR - 1; sfb++)
    {
        if (sfb < CUTOFF_BANDS && !mask[sfb])
        {
            cutoffBandsFlag = 1;
        }

        if (mask[sfb])
        {
            cnt++;
        }
    }
    if ((cnt <= ((N_SFB_HOA_LBR - 1) / 3) + 3) || cutoffBandsFlag)
    {
        SetShort(mask, 0, N_SFB_HOA_LBR - 1);

        return DMX_MONO;
    }
    else
    {
        if ((cnt >= (N_SFB_HOA_LBR - (N_SFB_HOA_LBR - 1) / 3) - 3)) {

            SetShort(mask, 1, N_SFB_HOA_LBR - 1);

            return DMX_FULL_MS;
        }
        else {
            return DMX_SFB_MS;
        }
    }
}


/*
HOA mode ild param scalar quantization
*/
static int16_t HoaIldQuantization(
    float ild,
    float *ildQ)
{
    float dist;
    float distMin = FLT_MAX;
    int16_t idx = 0;

    int16_t range;
    int16_t idxTmp;
    float ildTmp;

    range = 1 << (HOA_ILD_BITS - 1);

    if (ild <= 1.0f) {
        idxTmp = AVS3_MIN(AVS3_MAX((int16_t)(ild * range + 0.5f), 1), range - 1);
        ildTmp = (float)idxTmp / range;
    }
    else {
        idxTmp = AVS3_MIN(AVS3_MAX((int16_t)(1.0f / ild * range + 0.5f), 1), range - 1);
        ildTmp = (float)range / idxTmp;
    }

    for (int16_t i = 0; i < MC_ILD_CBLEN; i++) {
        dist = ildTmp - mcIldCodebook[i];
        dist *= dist;

        if (dist < distMin) {
            distMin = dist;
            idx = i;
        }
    }

    *ildQ = mcInvIldCodebook[idx];

    return idx;
}


static void ApplyHoaGroupILD(AVS3_HOA_ENC_DATA_HANDLE hEncHoa, float energy[MAX_HOA_DMX_CHANNELS], short chIdxMap[MAX_HOA_DMX_CHANNELS], const short nChans)
{
    short i, ch;
    float meanNrg = 0.f;
    float ratio, quntRatio;
    const short lenFrame = hEncHoa->hHoaConfig->frameLength;

    meanNrg = AVS3_MAX(VecMean(energy, nChans), AVS3_EPSILON);

    for (ch = 0; ch < nChans; ch++)
    {
        ratio = energy[ch] / meanNrg;
        hEncHoa->groupILD[chIdxMap[ch]] = HoaIldQuantization(ratio, &quntRatio);

        for (i = 0; i < lenFrame; i++)
        {
            hEncHoa->coreSpectrum[chIdxMap[ch]][i] *= quntRatio;
        }
    }

    return;
}


static void GetDMXChannelPair(
    AVS3_HOA_ENC_DATA_HANDLE hEncHoa,
    short chIdxMap[MAX_HOA_DMX_CHANNELS],
    short pairFlag[MAX_HOA_DMX_CHANNELS],
    const short groupIdx,
    const short nChans
)
{
    short row, ch1, ch2;
    float tmp1, tmp2;
    const short lenFrame = hEncHoa->hHoaConfig->groupCodeLines[groupIdx];
    const short nPair = nChans / 2;
    short pairIdx = 0;
    short cntPair = 0;
    float maxCorr = 0.f;
    float corrMatrix[MAX_HOA_DMX_CHANNELS][MAX_HOA_DMX_CHANNELS];

    short dmxMode;
    short mask[N_SFB_HOA_LBR];

    for (ch1 = 0; ch1 < nChans; ch1++)
    {
        for (ch2 = 0; ch2 < nChans; ch2++)
        {
            corrMatrix[ch1][ch2] = 0.f;
        }
    }

    for (ch1 = 0; ch1 < nChans; ch1++)
    {
        tmp1 = VLinalgNorm(hEncHoa->coreSpectrum[chIdxMap[ch1]], lenFrame);
        for (ch2 = ch1; ch2 < nChans; ch2++)
        {
            tmp2 = VLinalgNorm(hEncHoa->coreSpectrum[chIdxMap[ch2]], lenFrame);

            corrMatrix[ch1][ch2] = Dotp(hEncHoa->coreSpectrum[chIdxMap[ch1]], hEncHoa->coreSpectrum[chIdxMap[ch2]], lenFrame);

            corrMatrix[ch1][ch2] /= tmp1 * tmp2;

            if (((float)fabs(corrMatrix[ch1][ch2]) < 0.325f) || (tmp1 * tmp2 < 0.f))
            {
                corrMatrix[ch1][ch2] = AVS3_EPSILON;
            }

            corrMatrix[ch1][ch2] = (float)fabs(corrMatrix[ch1][ch2]);
        }
    }

    cntPair = 0;
    for (pairIdx = 0; pairIdx < nPair; pairIdx++)
    {
        float maxCorr = FindMaxCorrelation(corrMatrix, &ch1, &ch2, nChans);

        if (hEncHoa->transformType[chIdxMap[ch1]] != hEncHoa->transformType[chIdxMap[ch2]] || maxCorr < 0.325f)
        {
            /* clear row */
            SetZero(corrMatrix[ch1], nChans);
            SetZero(corrMatrix[ch2], nChans);

            /* update matrix */
            for (row = 0; row < nChans; row++)
            {
                corrMatrix[row][ch1] = 0.f;
                corrMatrix[row][ch2] = 0.f;
            }

            pairFlag[chIdxMap[ch1]] = 0;
            pairFlag[chIdxMap[ch2]] = 0;

            continue;
        }

        if ((groupIdx == 0 && maxCorr > 0.325f) || (groupIdx != 0 && maxCorr > 0.4725f))
        {
            hEncHoa->dmxInfo[groupIdx][cntPair].ch1 = chIdxMap[ch1];
            hEncHoa->dmxInfo[groupIdx][cntPair].ch2 = chIdxMap[ch2];
            hEncHoa->dmxInfo[groupIdx][cntPair].chIdx = ChanToIndex(ch1, ch2, nChans);

            dmxMode = GetMSDecision(hEncHoa, chIdxMap[ch1], chIdxMap[ch2], mask);

            if (dmxMode == DMX_MONO)
            {
                /* clear row */
                SetZero(corrMatrix[ch1], nChans);
                SetZero(corrMatrix[ch2], nChans);

                /* update matrix */
                for (row = 0; row < nChans; row++)
                {
                    corrMatrix[row][ch1] = 0.f;
                    corrMatrix[row][ch2] = 0.f;
                }

                pairFlag[chIdxMap[ch1]] = 0;
                pairFlag[chIdxMap[ch2]] = 0;
                hEncHoa->dmxInfo[groupIdx][cntPair].ms = DMX_MONO;

                continue;
            }

            assert(dmxMode == DMX_FULL_MS || dmxMode == DMX_SFB_MS);

            hEncHoa->dmxInfo[groupIdx][cntPair].ms = dmxMode;
            MvShort2Short(mask, hEncHoa->dmxInfo[groupIdx][cntPair].sfbMask, N_SFB_HOA_LBR - 1);
            assert(hEncHoa->dmxInfo[groupIdx][cntPair].chIdx >= 0);

            pairFlag[chIdxMap[ch1]] = 1;
            pairFlag[chIdxMap[ch2]] = 1;

            /* clear row */
            SetZero(corrMatrix[ch1], nChans);
            SetZero(corrMatrix[ch2], nChans);

            /* update matrix */
            for (row = 0; row < nChans; row++)
            {
                corrMatrix[row][ch1] = 0.f;
                corrMatrix[row][ch2] = 0.f;
            }

            cntPair++;
        }
        else
        {
            pairFlag[chIdxMap[ch1]] = 0;
            pairFlag[chIdxMap[ch2]] = 0;

            break;
        }
    }

    hEncHoa->pairIdx[groupIdx] = cntPair;

    return;
}


static void ApplyHoaGroupDMX(AVS3_HOA_ENC_DATA_HANDLE hEncHoa, const short groupIdx)
{
    short i, ch, pair;
    short chIdxMap[MAX_HOA_DMX_CHANNELS];
    float nrg[MAX_HOA_DMX_CHANNELS];
    const short nChans = hEncHoa->hHoaConfig->groupChans[groupIdx];
    short* pairFlag = hEncHoa->pairFlag;
    const short lenFrame = hEncHoa->hHoaConfig->frameLength;

    SetZero(nrg, MAX_HOA_DMX_CHANNELS);

    /* init */
    for (ch = 0; ch < nChans; ch++)
    {
        chIdxMap[ch] = ch + hEncHoa->hHoaConfig->groupChOffset[groupIdx];

        nrg[ch] = VLinalgNorm(hEncHoa->coreSpectrum[chIdxMap[ch]], lenFrame);
    }

    /* ILD */
    ApplyHoaGroupILD(hEncHoa, nrg, chIdxMap, nChans);

    if (!CheckHoaVLBasis(hEncHoa))
    {
        for (pair = 0; pair < MAX_HOA_DMX_CHANNELS / 2; pair++)
        {
            SetShort(hEncHoa->dmxInfo[groupIdx][pair].sfbMask, 0, N_SFB_HOA_LBR);
        }
        SetShort(pairFlag + hEncHoa->hHoaConfig->groupChOffset[groupIdx], 0, nChans);

        /* Get channel pair */
        GetDMXChannelPair(hEncHoa, chIdxMap, pairFlag, groupIdx, nChans);
    }

    for (pair = 0; pair < hEncHoa->pairIdx[groupIdx]; pair++)
    {
        short ch1 = hEncHoa->dmxInfo[groupIdx][pair].ch1;
        short ch2 = hEncHoa->dmxInfo[groupIdx][pair].ch2;

        if (hEncHoa->dmxInfo[groupIdx][pair].ms != DMX_MONO)
        {
            for (short sfb = 0; sfb < N_SFB_HOA_LBR - 1; sfb++)
            {
                if (hEncHoa->dmxInfo[groupIdx][pair].sfbMask[sfb])
                {
                    SubBandMS(hEncHoa->coreSpectrum[ch1], hEncHoa->coreSpectrum[ch2], hoa_sfb_table_low_bitrate[sfb],
                        hoa_sfb_table_low_bitrate[sfb + 1]);
                }
            }
        }
    }

    /* recovery Mono channel */
    for (ch = 0; ch < nChans; ch++)
    {
        float qratio;

        if (!pairFlag[chIdxMap[ch]] || hEncHoa->pairIdx[groupIdx] == 0)
        {
            qratio = mcIldCodebook[hEncHoa->groupILD[chIdxMap[ch]]];

            for (i = 0; i < hEncHoa->hHoaConfig->frameLength; i++)
            {
                hEncHoa->coreSpectrum[chIdxMap[ch]][i] *= qratio;
            }

            hEncHoa->groupILD[chIdxMap[ch]] = MC_ILD_CBLEN;
        }
    }

    return;
}

static void GetForeGroupEnergyRatio(AVS3_HOA_ENC_DATA_HANDLE hEncHoa, const short nTotalChans)
{
    short ch, group;
    short offset;
    float nrg[MAX_HOA_CHANNELS];
    float totalEnergy;
    float nrgRatio[MAX_HOA_DMX_GROUPS];
    float directionalNrgRatio;
    float tmp = 0.f;

    const short lenFrame = hEncHoa->hHoaConfig->frameLength;
    AVS3_HOA_CONFIG_DATA_HANDLE hHoaConfig = hEncHoa->hHoaConfig;

    if (!hHoaConfig->spatialAnalysis)
    {
        for (group = 0; group < hHoaConfig->nTotalChanGroups; group++)
        {
            hEncHoa->groupBitsRatio[group] = AVS3_MIN(HOA_BITS_RATIO_RANGE - 1, AVS3_MAX(1, (unsigned short)(HOA_BITS_RATIO_RANGE * hHoaConfig->groupBitsRatio[group] + 0.5f)));
        }

        return;
    }

    /* init */
    for (ch = 0; ch < nTotalChans; ch++)
    {
        nrg[ch] = VLinalgNorm(hEncHoa->coreSpectrum[ch], lenFrame);
    }

    totalEnergy = AVS3_MAX(SumFloat(nrg, nTotalChans), AVS3_EPSILON);

    directionalNrgRatio = SumFloat(nrg, hHoaConfig->nTotalForeChans) / totalEnergy;

    /* shuai@2021/5/11£ºtuning for directional signal */
    if (directionalNrgRatio >= 0.9375f
        || ((hEncHoa->numSource <= 2 && hEncHoa->direcSignalNrgRatio > 0.875f))
        )
    {
        /* high strong directional signal */
        nrgRatio[0] = 0.375f * directionalNrgRatio + 0.625f * hHoaConfig->maxDirecChanBitsRatio;

        nrgRatio[0] = AVS3_MIN(nrgRatio[0], hHoaConfig->maxDirecChanBitsRatio + 0.2f * nrgRatio[0]);

        /* residual channels */
        tmp = (1.f - nrgRatio[0]) / hHoaConfig->nTotalResChans;

        for (group = 1; group < hHoaConfig->nTotalChanGroups; group++)
        {
            nrgRatio[group] = tmp * hHoaConfig->groupChans[group];
        }
    }
    else if ((directionalNrgRatio < 0.9375f && directionalNrgRatio >= 0.75f)
        || (hEncHoa->numSource <= 2 || hEncHoa->direcSignalNrgRatio >= 0.6875f)
        )
    {
        /* middle strong directional signal */
        nrgRatio[0] = 0.4f * directionalNrgRatio + 0.6f * hHoaConfig->maxDirecChanBitsRatio;

        nrgRatio[0] = AVS3_MIN(nrgRatio[0], hHoaConfig->maxDirecChanBitsRatio + 0.1f * nrgRatio[0]);

        /* residual channels */
        tmp = (1.f - nrgRatio[0]) / hHoaConfig->nTotalResChans;

        for (group = 1; group < hHoaConfig->nTotalChanGroups; group++)
        {
            nrgRatio[group] = tmp * hHoaConfig->groupChans[group];
        }
    }
    else
    {
        /* no directional signal */
        for (group = 0; group < hHoaConfig->nTotalChanGroups; group++)
        {
            offset = hHoaConfig->groupChOffset[group];
            nrgRatio[group] = SumFloat(nrg + offset, hHoaConfig->groupChans[group]) / totalEnergy;

            if (nrgRatio[group] < hHoaConfig->groupBitsRatio[group])
            {
                nrgRatio[group] = hHoaConfig->groupBitsRatio[group];
            }
            else
            {
                nrgRatio[group] = 0.9f * hHoaConfig->groupBitsRatio[group] + 0.1f * nrgRatio[group];
            }
        }
    }

    for (group = 0; group < hHoaConfig->nTotalChanGroups; group++)
    {
        hEncHoa->groupBitsRatio[group] = AVS3_MIN(HOA_BITS_RATIO_RANGE - 1, AVS3_MAX(1, (unsigned short)(HOA_BITS_RATIO_RANGE * nrgRatio[group] + 0.5f)));
    }

    return;
}

static void FindGroupBitsRatio(AVS3_HOA_ENC_DATA_HANDLE hEncHoa, const short groupIdx)
{
    short ch;
    float tmpRatio;
    float nrg[MAX_HOA_DMX_CHANNELS];
    float totalNrg = 0.f;
    const short chanOffset = hEncHoa->hHoaConfig->groupChOffset[groupIdx];
    const short nChans = hEncHoa->hHoaConfig->groupChans[groupIdx];
    const short lenFrame = hEncHoa->hHoaConfig->groupCodeLines[groupIdx];

    for (ch = 0; ch < nChans; ch++)
    {
        nrg[ch] = VLinalgNorm(hEncHoa->coreSpectrum[ch + chanOffset], lenFrame);

        totalNrg += AVS3_MAX(nrg[ch], AVS3_EPSILON);
    }

    totalNrg = 1.f / totalNrg;

    for (ch = 0; ch < nChans; ch++)
    {
        tmpRatio = nrg[ch] * totalNrg;
        hEncHoa->bitsRatio[groupIdx][ch] = AVS3_MIN(HOA_BITS_RATIO_RANGE - 1, AVS3_MAX(1, (unsigned short)(HOA_BITS_RATIO_RANGE * tmpRatio + 0.5f)));
    }

    return;
}

static void HoaTransportChannelsDMX(AVS3_HOA_ENC_DATA_HANDLE hEncHoa)
{
    short groupIdx;
    const short nTotalChanGroups = hEncHoa->hHoaConfig->nTotalChanGroups;
    const short nChans = hEncHoa->hHoaConfig->nTotalChansTransport;

    /* get group energy minRatio */
    GetForeGroupEnergyRatio(hEncHoa, nChans);

    groupIdx = 0;
    while (groupIdx < nTotalChanGroups)
    {
        /* DMX channel groups */
        ApplyHoaGroupDMX(hEncHoa, groupIdx);

        /* group bits split */
        FindGroupBitsRatio(hEncHoa, groupIdx);

        groupIdx++;
    }

    return;
}


void Avs3HOAReconfig(AVS3EncoderHandle stAvs3, short* nChans)
{
    short ch;
    AVS3_HOA_ENC_DATA_HANDLE hEncHoa = stAvs3->hEncHoa;
    const short lenFrame = stAvs3->hEncHoa->hHoaConfig->frameLength;
    const short overlapSize = stAvs3->hEncHoa->hHoaConfig->overlapSize;

    /* Update input signalInput */
    for (ch = 0; ch < stAvs3->hEncHoa->hHoaConfig->nTotalChansInput; ch++)
    {
        Mvf2f(stAvs3->hEncHoa->hoaSignalBuffer[ch] + lenFrame, stAvs3->hEncHoa->hoaSignalBuffer[ch], overlapSize);
    }

    *nChans = hEncHoa->hHoaConfig->nTotalChansTransport;
}

static void UpdateHoaEncoder(AVS3_HOA_ENC_DATA_HANDLE hEncHoa)
{

    short ch, group, offset;
    AVS3_HOA_CONFIG_DATA_HANDLE hHoaConfig = hEncHoa->hHoaConfig;

    for (group = 0; group < hHoaConfig->nTotalChanGroups; group++)
    {
        offset = hHoaConfig->groupChOffset[group];

        for (ch = 0; ch < hHoaConfig->groupChans[group]; ch++)
        {
            if (!hHoaConfig->groupBwe[group])
            {
                SetZero(hEncHoa->coreSpectrum[ch + offset] + hEncHoa->hHoaConfig->groupCodeLines[group],
                    hEncHoa->hHoaConfig->frameLength - hEncHoa->hHoaConfig->groupCodeLines[group]);
            }
        }
    }

    /* update */
    MvShort2Short(hEncHoa->pairIdx, hEncHoa->lastPairIdx, MAX_HOA_DMX_GROUPS);

    MvShort2Short(hEncHoa->basisIdx, hEncHoa->lastBasisIdx, hEncHoa->numVote);

    return;
}


void Avs3HoaCoreEncoder(AVS3EncoderHandle stAvs3, short* channelBytes)
{
    short ch;
    AVS3_HOA_ENC_DATA_HANDLE hEncHoa = stAvs3->hEncHoa;
    AVS3_HOA_CONFIG_DATA_HANDLE hConfig = hEncHoa->hHoaConfig;
    const short nChans = hConfig->nTotalChansTransport;
    const short lenFrame = hConfig->frameLength;
    short availableBits = 0;

    int16_t numGroups[MAX_CHANNELS];         // num groups for each channel

    /* initial */
    for (ch = 0; ch < nChans; ch++)
    {
        hEncHoa->coreSpectrum[ch] = stAvs3->hEncCore[ch]->origSpectrum;
        hEncHoa->transformType[ch] = stAvs3->hEncCore[ch]->transformType;
    }

    /* HOA transport channels DMX (beta version complexity is n^2: will replace by graph algorithm later, complexity will be decrese to n*lgn.)*/
    HoaTransportChannelsDMX(hEncHoa);

    // grouping for short window
    for (int16_t i = 0; i < nChans; i++) {

        AVS3_ENC_CORE_HANDLE hEncCore = stAvs3->hEncCore[i];
        SpectrumGroupingEnc(hEncCore->origSpectrum, hEncCore->frameLength, hEncCore->transformType,
            hEncCore->groupIndicator, &hEncCore->numGroups);

        numGroups[i] = hEncCore->numGroups;
    }
    // write grouping bitstream
    WriteGroupBitstream(stAvs3, nChans, stAvs3->bitstream, &stAvs3->totalSideBits);

    /* write HOA side bits */
    WriteHoaBitstream(stAvs3);

    /* get available bits */
    availableBits = (short)GetAvailableBits(stAvs3->bitsPerFrame, stAvs3->totalSideBits, numGroups, nChans, stAvs3->nnTypeConfig);

    /* bits split */
    HoaSplitBytesGroup(hConfig, channelBytes, hEncHoa->groupBitsRatio, hEncHoa->bitsRatio, availableBits);

    /* update */
    UpdateHoaEncoder(hEncHoa);

    return;
}

void Avs3HOAEncoder(AVS3EncoderHandle stAvs3, float data[MAX_CHANNELS][MAX_FRAME_LEN], const short lenFrame)
{
    short i, ch;
    float matBasisCoefs[L_HOA_BASIS_ROWS][MAX_HOA_BASIS];
    float invMatCoefs[MAX_HOA_BASIS][L_HOA_BASIS_ROWS];
    const short offset = stAvs3->initFrame ? 0 : stAvs3->lookaheadSamples;
    const short delay = stAvs3->initFrame ? stAvs3->lookaheadSamples : 0;
    float hoaSpec[MAX_HOA_CHANNELS][HOA_LEN_FRAME48k];

    AVS3_HOA_ENC_DATA_HANDLE hEncHoa = stAvs3->hEncHoa;

    /* Init input the same as core encoder */
    for (ch = 0; ch < stAvs3->numChansInput; ch++)
    {
        Mvf2f(data[ch], hEncHoa->hoaSignalBuffer[ch] + offset, lenFrame + delay);
    }

    for (i = 0; i < hEncHoa->numVote; i++)
    {
        SetZero(invMatCoefs[i], L_HOA_BASIS_ROWS);
    }

    for (i = 0; i < L_HOA_BASIS_ROWS; i++)
    {
        SetZero(matBasisCoefs[i], MAX_HOA_BASIS);
    }

    /* HOA Scene analysis */
    HoaSceneAnalysis(hEncHoa);

    /* TF transform */
    HoaSignalMdctAnalysis(hEncHoa);

    if (hEncHoa->hHoaConfig->spatialAnalysis)
    {
        /* Get virtual loudspeaker basis */
        HoaGetBasisCoefs(hEncHoa, matBasisCoefs, lenFrame);

        /* HOA signal decomposition */
        HoaComputeParams(hEncHoa, matBasisCoefs, invMatCoefs);

        /* Original signals to transport signals */
        HoaSignalDecomposition(hEncHoa, hoaSpec, matBasisCoefs, invMatCoefs, lenFrame);

        /* VL post filter */
        HoaVLPostFiltetr(hEncHoa, hoaSpec, matBasisCoefs, invMatCoefs, lenFrame);
    }
    else
    {
        for (ch = 0; ch < stAvs3->numChansInput; ch++)
        {
            Mvf2f(hEncHoa->origSpecturm[ch], hoaSpec[ch], lenFrame);
        }
    }

    /* Synthesis current frame */
    HoaSynthCurrentFrame(hEncHoa, hoaSpec, data, lenFrame);

    return;
}