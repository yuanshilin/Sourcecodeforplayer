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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "avs3_options.h"
#include "avs3_cnst_com.h"
#include "avs3_rom_com.h"
#include "avs3_prot_com.h"
#include "avs3_prot_enc.h"


/*
encode the LSF coefficients in the first stage
I/O params:
    const float *lsf                                (i) the LSF coefficients in the first stage
    const float *lsfCodebook                        (i) codebook name of the first stage
    const short lsfLen                              (i) LSP length in the first stage
    const short nbitsCodebook                       (i) the codebook length of the first stage
    short *index                                    (o) the index of nearest quanted LSF coefficients
    const short candidateNum                        (i) total candidates of quanted LSF coefficients
*/
static void SearchIndexStage1(
    const float *lsf, 
    const float *lsfCodebook, 
    const short lsfLen, 
    const short nbitsCodebook, 
    short *index, 
    const short candidateNum
)
{
    short i, j, k, m, startNum, flag, codebookRow;
    float tmp, minDist[MAX_CANDIDATE_NUM];

    codebookRow = nbitsCodebook;
    
    for (i = 0; i < MAX_CANDIDATE_NUM; i++) {
        minDist[i] = 1.0e30f;
        index[i] = 0;
    }

    for (i = 0; i < codebookRow; i++) {
        tmp = 0.0f;
        flag = 0;
        for (j = 0; j < lsfLen; j++) {
            tmp += (lsfCodebook[i * lsfLen + j] - lsf[j]) * (lsfCodebook[i * lsfLen + j] - lsf[j]);
        }
        for (k = 0; k < candidateNum; k++) {
            if (tmp < minDist[k]) {
                flag = 1;
                startNum = k;
                break;
            }
        }
        if (flag == 1) {
            for (m = candidateNum - 1; m > startNum; m--) {
                minDist[m] = minDist[m - 1];
                index[m] = index[m - 1];
            }
            minDist[startNum] = tmp;
            index[startNum] = i;
        }
    }

    return;
}


/*
encode the LSF coefficients in the second stage
I/O params:
    const float *lsfStage2                          (i) the LSF coefficients in the second stage
    const float *lsfStage2Codebook                  (i) codebook name of the second stage
    const short lsfLen                              (i) LSP length in the second stage
    const short nbitsStage2                         (i) the codebook length of the second stage
    short *index                                    (o) the index of nearest quanted LSF coefficients
*/
static float SearchIndexStage2(
    const float *lsfStage2, 
    const float *lsfStage2Codebook, 
    const short lsfLen, 
    const short nbitsStage2, 
    short *index
)
{
    short i, j, codebookRow;
    float minDist, tmp;

    codebookRow = nbitsStage2;
    minDist = 1.0e30f;

    for (i = 0; i < codebookRow; i++) {
        tmp = 0.0f;
        for (j = 0; j < lsfLen; j++) {
            tmp += (lsfStage2Codebook[i * lsfLen + j] - lsfStage2[j]) * (lsfStage2Codebook[i * lsfLen + j] - lsfStage2[j]);
        }
        if (tmp < minDist) {
            minDist = tmp;
            *index = i;
        }
    }

    return minDist;
}


/*
encode the LSF coefficients in high bitrate with 46 bit CB
I/O params:
    const float *lsf                                (i) the LSF coefficients
    float *quantLsf                                 (o) quanted LSF coefficients
    short *lsfVqIndex                               (o) VQ index
    const short lpcOrder                            (i) LSF order
*/
static void LsfQuantEncHbr(
    const float *lsf, 
    float *quantLsf, 
    short *lsfVqIndex,
    const short lpcOrder
)
{
    short i, j;
    short vqIndex[LSF_CB_NUM_HBR], vqIndexTmp[LSF_CB_NUM_HBR];
    short candidateNum;
    short tmpIndex1[MAX_CANDIDATE_NUM], tmpIndex2[MAX_CANDIDATE_NUM];
    float tmpError, tmpDist, minDist;
    float resLsf[LPC_ORDER], resLsfStage2[MAX_CANDIDATE_NUM][LPC_ORDER];

    candidateNum = MAX_CANDIDATE_NUM;
    minDist = 1.0e30f;
    for (i = 0; i < lpcOrder; i++) {
        resLsf[i] = lsf[i] - mean_lsf[i];
    }

    /* LSF 1-9 quantization */
    for (i = 0; i < candidateNum; i++) {
        tmpIndex1[i] = 0;
    }

    SearchIndexStage1(resLsf, lsf_stage1_CB1_hbr, LSF_STAGE1_LEN1_HBR, LSF_STAGE1_CB1_SIZE_HBR, tmpIndex1, candidateNum);

    for (i = 0; i < candidateNum; i++) {

        tmpDist = 0.0f;
        for (j = 0; j < LSF_STAGE1_LEN1_HBR; j++) {
            resLsfStage2[i][j] = resLsf[j] - lsf_stage1_CB1_hbr[tmpIndex1[i] * LSF_STAGE1_LEN1_HBR + j];
        }

        tmpError = SearchIndexStage2(resLsfStage2[i], lsf_stage2_CB1_hbr, 
                                     LSF_STAGE2_LEN1_HBR, LSF_STAGE2_CB1_SIZE_HBR, &vqIndexTmp[2]);
        tmpDist += tmpError;
        tmpError = SearchIndexStage2(&resLsfStage2[i][LSF_STAGE2_LEN1_HBR], lsf_stage2_CB2_hbr, 
                                     LSF_STAGE2_LEN2_HBR, LSF_STAGE2_CB2_SIZE_HBR, &vqIndexTmp[3]);
        tmpDist += tmpError;
        tmpError = SearchIndexStage2(&resLsfStage2[i][LSF_STAGE2_LEN1_HBR + LSF_STAGE2_LEN2_HBR], lsf_stage2_CB3_hbr, 
                                     LSF_STAGE2_LEN3_HBR, LSF_STAGE2_CB3_SIZE_HBR, &vqIndexTmp[4]);
        tmpDist += tmpError;

        if (tmpDist < minDist) {
            minDist = tmpDist;
            vqIndex[0] = tmpIndex1[i];
            vqIndex[2] = vqIndexTmp[2];
            vqIndex[3] = vqIndexTmp[3];
            vqIndex[4] = vqIndexTmp[4];
        }
    }

    /* LSF 10-16 quantization */
    for (i = 0; i < candidateNum; i++) {
        tmpIndex2[i] = 0;
    }
    SearchIndexStage1(&resLsf[LSF_STAGE1_LEN1_HBR], lsf_stage1_CB2_hbr, LSF_STAGE1_LEN2_HBR, LSF_STAGE1_CB2_SIZE_HBR, tmpIndex2, candidateNum);

    minDist = 1.0e30f;
    for (i = 0; i < candidateNum; i++) {

        tmpDist = 0.0f;
        for (j = 0; j < LSF_STAGE1_LEN2_HBR; j++) {
            resLsfStage2[i][LSF_STAGE1_LEN1_HBR + j] = resLsf[LSF_STAGE1_LEN1_HBR + j] - lsf_stage1_CB2_hbr[tmpIndex2[i] * LSF_STAGE1_LEN2_HBR + j];
        }

        tmpError = SearchIndexStage2(&resLsfStage2[i][LSF_STAGE1_LEN1_HBR], lsf_stage2_CB4_hbr, 
                                     LSF_STAGE2_LEN4_HBR, LSF_STAGE2_CB4_SIZE_HBR, &vqIndexTmp[5]);
        tmpDist += tmpError;
        tmpError = SearchIndexStage2(&resLsfStage2[i][LSF_STAGE1_LEN1_HBR + LSF_STAGE2_LEN4_HBR], lsf_stage2_CB5_hbr, 
                                     LSF_STAGE2_LEN5_HBR, LSF_STAGE2_CB5_SIZE_HBR, &vqIndexTmp[6]);
        tmpDist += tmpError;

        if (tmpDist < minDist) {
            minDist = tmpDist;
            vqIndex[1] = tmpIndex2[i];
            vqIndex[5] = vqIndexTmp[5];
            vqIndex[6] = vqIndexTmp[6];
        }
    }

    /* Write the codebook indices into bitstream */
    for (i = 0; i < LSF_CB_NUM_HBR; i++) {
        lsfVqIndex[i] = vqIndex[i];
    }

    /* decode the lsf coefficients */
    LsfQuantDecHbr(vqIndex, quantLsf, LPC_ORDER);

    return;
}


/*
encode the LSF coefficients in low bitrate with 36 bit CB
I/O params:
const float *lsf                                (i) the LSF coefficients
float *quantLsf                                 (o) quanted LSF coefficients
short *lsfVqIndex                               (o) VQ index
const short lpcOrder                            (i) LSF order
*/
static void LsfQuantEncLbr(
    const float *lsf,
    float *quantLsf,
    short *lsfVqIndex,
    const short lpcOrder
)
{
    short i, j;
    short vqIndex[LSF_CB_NUM_LBR], vqIndexTmp[LSF_CB_NUM_LBR];
    short candidateNum;
    short tmpIndex1[MAX_CANDIDATE_NUM], tmpIndex2[MAX_CANDIDATE_NUM];
    float tmpError, tmpDist, minDist;
    float resLsf[LPC_ORDER], resLsfStage2[MAX_CANDIDATE_NUM][LPC_ORDER];

    candidateNum = MAX_CANDIDATE_NUM;
    minDist = 1.0e30f;
    for (i = 0; i < lpcOrder; i++) {
        resLsf[i] = lsf[i] - mean_lsf[i];
    }

    /* LSF 1-9 quantization */
    for (i = 0; i < candidateNum; i++) {
        tmpIndex1[i] = 0;
    }

    SearchIndexStage1(resLsf, lsf_stage1_CB1_lbr, LSF_STAGE1_LEN1_LBR, LSF_STAGE1_CB1_SIZE_LBR, tmpIndex1, candidateNum);

    for (i = 0; i < candidateNum; i++) {

        tmpDist = 0.0f;
        for (j = 0; j < LSF_STAGE1_LEN1_LBR; j++) {
            resLsfStage2[i][j] = resLsf[j] - lsf_stage1_CB1_lbr[tmpIndex1[i] * LSF_STAGE1_LEN1_LBR + j];
        }

        tmpError = SearchIndexStage2(resLsfStage2[i], lsf_stage2_CB1_lbr, 
                                     LSF_STAGE2_LEN1_LBR, LSF_STAGE2_CB1_SIZE_LBR, &vqIndexTmp[2]);
        tmpDist += tmpError;
        tmpError = SearchIndexStage2(&resLsfStage2[i][LSF_STAGE2_LEN1_LBR], lsf_stage2_CB2_lbr, 
                                     LSF_STAGE2_LEN2_LBR, LSF_STAGE2_CB2_SIZE_LBR, &vqIndexTmp[3]);
        tmpDist += tmpError;

        if (tmpDist < minDist) {
            minDist = tmpDist;
            vqIndex[0] = tmpIndex1[i];
            vqIndex[2] = vqIndexTmp[2];
            vqIndex[3] = vqIndexTmp[3];
        }
    }

    /* LSF 10-16 quantization */
    for (i = 0; i < candidateNum; i++) {
        tmpIndex2[i] = 0;
    }
    SearchIndexStage1(&resLsf[LSF_STAGE1_LEN1_LBR], lsf_stage1_CB2_lbr, LSF_STAGE1_LEN2_LBR, LSF_STAGE1_CB2_SIZE_LBR, tmpIndex2, candidateNum);

    minDist = 1.0e30f;
    for (i = 0; i < candidateNum; i++) {

        tmpDist = 0.0f;
        for (j = 0; j < LSF_STAGE1_LEN2_LBR; j++) {
            resLsfStage2[i][LSF_STAGE1_LEN1_LBR + j] = resLsf[LSF_STAGE1_LEN1_LBR + j] - lsf_stage1_CB2_lbr[tmpIndex2[i] * LSF_STAGE1_LEN2_LBR + j];
        }

        tmpError = SearchIndexStage2(&resLsfStage2[i][LSF_STAGE1_LEN1_LBR], lsf_stage2_CB3_lbr, 
                                     LSF_STAGE2_LEN3_LBR, LSF_STAGE2_CB3_SIZE_LBR, &vqIndexTmp[4]);
        tmpDist += tmpError;

        if (tmpDist < minDist) {
            minDist = tmpDist;
            vqIndex[1] = tmpIndex2[i];
            vqIndex[4] = vqIndexTmp[4];
        }
    }

    /* Write the codebook indices into bitstream */
    for (i = 0; i < LSF_CB_NUM_LBR; i++) {
        lsfVqIndex[i] = vqIndex[i];
    }

    /* decode the lsf coefficients */
    LsfQuantDecLbr(vqIndex, quantLsf, LPC_ORDER);

    return;
}


/*
choose the LSF quantization function
I/O params:
const float *lsf                                (i) the LSF coefficients
float *quantLsf                                 (o) quanted LSF coefficients
short *lsfVqIndex                               (o) VQ index
const short lpcOrder                            (i) LSF order
const short lsfLbrFlag                          (i) low bits (36) or high bits
*/
void LsfQuantEnc(
    const float *lsf,
    float *quantLsf,
    short *lsfVqIndex,
    const short lpcOrder,
    const short lsfLbrFlag
)
{
    if (lsfLbrFlag == 0) {
        LsfQuantEncHbr(lsf, quantLsf, lsfVqIndex, lpcOrder);
    }
    else if (lsfLbrFlag == 1) {
        LsfQuantEncLbr(lsf, quantLsf, lsfVqIndex, lpcOrder);
    }

    return;
}
