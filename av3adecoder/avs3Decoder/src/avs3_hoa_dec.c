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
#include <math.h>
#include <assert.h>
#include "avs3_stat_dec.h"
#include "avs3_prot_dec.h"
#include "avs3_prot_com.h"
#include "avs3_rom_com.h"

static void Avs3HoaDecoderReconfig(AVS3DecoderHandle hAvs3Dec, short* nchans, short* totalBits)
{
    hAvs3Dec->hDecHoa->hHoaConfig->totalBitrate = hAvs3Dec->totalBitrate;

    HoaBitrateConfigTable(hAvs3Dec->hDecHoa->hHoaConfig);

    *nchans = hAvs3Dec->hDecHoa->hHoaConfig->nTotalChansTransport;

    hAvs3Dec->numChansOutput = hAvs3Dec->hDecHoa->hHoaConfig->nTotalChansTransport;

    hAvs3Dec->hDecHoa->numVL = hAvs3Dec->hDecHoa->hHoaConfig->spatialAnalysis ? hAvs3Dec->hDecHoa->hHoaConfig->nTotalForeChans : 0;

    *totalBits = (short)hAvs3Dec->bitsPerFrame;

    return;
}


static void InverseSubBandMS(float x0[], float x1[], const short startLines, const short stopLine)
{
    short i;
    float tmpValue;
    float const c = (float)(sqrt(2.f) / 2.f);

    for (i = startLines; i < stopLine; i++)
    {
        tmpValue = x0[i];
        x0[i] = (x0[i] + x1[i]) * c;
        x1[i] = (tmpValue - x1[i]) * c;
    }

    return;
}


static void IndexToChannel(const short pairIdx, short* ch1, short* ch2, const short nChannels)
{
    short i, j;
    short tmpIdx = 0;

    for (j = 1; j < nChannels; j++)
    {
        for (i = 0; i < j; i++)
        {
            if (tmpIdx == pairIdx)
            {
                *ch1 = i;
                *ch2 = j;

                return;
            }
            else
            {
                tmpIdx++;
            }
        }
    }

    return;
}

static void Avs3HoaInverseDMX(AVS3DecoderHandle hAvs3Dec)
{
    short i, ch, ch1, ch2, groupIdx;
    AVS3_HOA_DEC_DATA_HANDLE hDecHoa = hAvs3Dec->hDecHoa;
    short nChans;
    float qratio = 0.f;
    short groupChOffset = 0;
    const short nGroups = hDecHoa->hHoaConfig->nTotalChanGroups;
    const short lenFrame = hDecHoa->hHoaConfig->frameLength;

    nChans = hDecHoa->hHoaConfig->nTotalChansTransport;

    groupIdx = 0;
    while (groupIdx < nGroups)
    {
        for (i = 0; i < hDecHoa->pairIdx[groupIdx]; i++)
        {
            groupChOffset = hDecHoa->hHoaConfig->groupChOffset[groupIdx];

            IndexToChannel(hDecHoa->chIdx[groupIdx][i], &ch1, &ch2, hDecHoa->hHoaConfig->groupChans[groupIdx]);

            ch1 += groupChOffset;
            ch2 += groupChOffset;
            for (short sfb = 0; sfb < N_SFB_HOA_LBR - 1; sfb++) {

                if (hDecHoa->sfbMask[groupIdx][i][sfb]) {
                    InverseSubBandMS(hAvs3Dec->hDecCore[ch1]->origSpectrum, hAvs3Dec->hDecCore[ch2]->origSpectrum,
                        hoa_sfb_table_low_bitrate[sfb], hoa_sfb_table_low_bitrate[sfb + 1]);
                }
            }
        }

        groupIdx++;
    }

    for (ch = 0; ch < nChans; ch++)
    {
        if (hDecHoa->groupILD[ch] != MC_ILD_CBLEN)
        {
            qratio = mcIldCodebook[hDecHoa->groupILD[ch]];

            for (i = 0; i < lenFrame; i++)
            {
                hAvs3Dec->hDecCore[ch]->origSpectrum[i] *= qratio;
            }
        }
    }

    return;
}

static void HoaCoreDec(AVS3_HOA_DEC_DATA_HANDLE hEncHoa, float output[MAX_HOA_CHANNELS][HOA_LEN_FRAME48k], const short lenFrame)
{
    short i, j;
    short ch, row, cols, k;

    AVS3_HOA_CONFIG_DATA_HANDLE hConfig = hEncHoa->hHoaConfig;
    const short nPreChans = hEncHoa->numVL;
    const short nChansOutput = hConfig->nTotalChansInput;
    const short nTotalResChans = hConfig->nTotalResChans;

    float matSignalBasis[HOA_LEN_FRAME48k + HOA_LEN_FRAME48k][MAX_HOA_BASIS];
    float matBasisCoefs[L_HOA_BASIS_ROWS][MAX_HOA_BASIS];
    float recoverySignal[MAX_HOA_CHANNELS][HOA_LEN_FRAME48k];

    float samples[L_HOA_BASIS_ROWS];
    float tmp1[L_HOA_BASIS_ROWS];
    float tmp2[L_HOA_BASIS_ROWS];

    short anglePair[2];

    SetZero(samples, L_HOA_BASIS_ROWS);
    SetZero(tmp1, L_HOA_BASIS_ROWS);
    SetZero(tmp2, L_HOA_BASIS_ROWS);

    /* initialization. */
    for (ch = 0; ch < nChansOutput; ch++)
    {
        SetFloat(recoverySignal[ch], 0.f, lenFrame);
    }

    /* get speaker basis index. */
    for (i = 0; i < nPreChans; i++)
    {
        short idx = hEncHoa->delayBasisIdx[0][i];

        MvShort2Short(avs3_hoa_fixed_angle_basis_matrix[idx], anglePair, 2);

        GetSingleNeighborBasisCoeff(anglePair, tmp1);

        for (j = 0; j < nChansOutput; j++)
        {
            matBasisCoefs[j][i] = tmp1[j];
        }
    }

    for (row = 0; row < nPreChans; row++)
    {
        for (cols = 0; cols < lenFrame; cols++)
        {
            matSignalBasis[cols][row] = output[row][cols];
        }
    }

    /* recovery signals. */
    for (row = 0; row < lenFrame; row++)
    {
        for (cols = 0; cols < nPreChans; cols++)
        {
            tmp1[cols] = matSignalBasis[row][cols];
        }

        for (k = 0; k < nChansOutput; k++)
        {
            for (cols = 0; cols < nPreChans; cols++)
            {
                tmp2[cols] = matBasisCoefs[k][cols];
            }

            recoverySignal[k][row] = (float)Dotp(tmp1, tmp2, nPreChans);
        }
    }

    /* get final result signals. */
    for (row = 0; row < nTotalResChans; row++)
    {
        for (cols = 0; cols < lenFrame; cols++)
        {
            recoverySignal[row][cols] += output[row + nPreChans][cols];
        }
    }

    for (row = 0; row < nChansOutput; row++)
    {
        Mvf2f(recoverySignal[row], output[row], lenFrame);
    }

    return;
}

static void HoaPostSynthesisFilter(AVS3DecoderHandle hAvs3Dec, float output[MAX_HOA_CHANNELS][FRAME_LEN], const short frameLength)
{
    short i, j, ch;
    short subFrame;
    float synthBuffer[HOA_OVERLAP_SIZE];
    float win[BLOCK_LEN_LONG];
    float* signal = NULL;

    AVS3_HOA_DEC_DATA_HANDLE hDecHoa = hAvs3Dec->hDecHoa;
    AVS3_HOA_CONFIG_DATA_HANDLE hHoaConfig = hDecHoa->hHoaConfig;
    const short overlapSize = hHoaConfig->overlapSize;
    const short synthChannelsOutput = hHoaConfig->nTotalChansInput;

    hAvs3Dec->numChansOutput = hHoaConfig->nTotalChansInput;

    /* MDCT */
    for (ch = 0; ch < hHoaConfig->nTotalChansTransport; ch++)
    {
        signal = hDecHoa->decSignalInput[ch] - overlapSize;
        for (subFrame = 0; subFrame < N_BLOCK_HOA; subFrame++)
        {
            /* windowing left part */
            for (i = 0; i < overlapSize; i++)
            {
                win[i] = signal[i] * hHoaConfig->hoaWindow[i];
            }

            /* window right part */
            for (i = 0; i < overlapSize; i++)
            {
                win[i + overlapSize] = signal[i + overlapSize] * hHoaConfig->hoaWindow[overlapSize - i - 1];
            }

            MDCT(win, hDecHoa->decSpecturm[ch] + subFrame * overlapSize, 2 * overlapSize);

            signal += overlapSize;
        }
    }

    if (hHoaConfig->spatialAnalysis) 
    {
        /* Transport signals to recovery signals */
        HoaCoreDec(hDecHoa, hDecHoa->decSpecturm, frameLength);
    }

    /* HOA signal Synthesis*/
    for (ch = 0; ch < synthChannelsOutput; ch++)
    {
        SetZero(output[ch], frameLength);

        Mvf2f(hDecHoa->decSynthBuffer[ch], synthBuffer, overlapSize);

        for (subFrame = 0; subFrame < N_BLOCK_HOA; subFrame++)
        {
            SetZero(win, BLOCK_LEN_LONG);

            Mvf2f(hDecHoa->decSpecturm[ch] + subFrame * overlapSize, win, overlapSize);

            IMDCT(win, 2 * overlapSize);

            /* windowing left part */
            for (i = 0; i < overlapSize; i++)
            {
                win[i] *= hHoaConfig->hoaWindow[i];
            }

            /* windowing right part */
            for (i = 0; i < overlapSize; i++)
            {
                win[i + overlapSize] *= hHoaConfig->hoaWindow[overlapSize - i - 1];
            }

            Vadd(win, synthBuffer, win, overlapSize);

            Mvf2f(win + overlapSize, synthBuffer, overlapSize);

            Mvf2f(win, output[ch] + subFrame * overlapSize, overlapSize);
        }

        Mvf2f(synthBuffer, hDecHoa->decSynthBuffer[ch], overlapSize);
    }

    /* update */
    for (i = 0; i < HOA_DELAY_BASIS - 1; i++)
    {
        for (j = 0; j < hDecHoa->numVL; j++)
        {
            hDecHoa->delayBasisIdx[i][j] = hDecHoa->delayBasisIdx[i + 1][j];
        }
    }

    MvShort2Short(hDecHoa->basisIdx, hDecHoa->delayBasisIdx[HOA_DELAY_BASIS - 1], hDecHoa->numVL);

    return;
}

void Avs3HoaDec(AVS3DecoderHandle hAvs3Dec, float synth[MAX_CHANNELS][FRAME_LEN]) 
{
    short ch;
    short nChans = 0;
    short totalBits = 0;
    short availableBits = 0;
    short channelBytes[MAX_CHANNELS] = {0};
    AVS3_DEC_CORE_HANDLE hDecCore = NULL;
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream = hAvs3Dec->hBitstream;
    const short frameLength = hAvs3Dec->frameLength;

    int16_t numGroups[MAX_CHANNELS];

    Avs3HoaDecoderReconfig(hAvs3Dec, &nChans, &totalBits);

    // decode core side info
    for (ch = 0; ch < nChans; ch++) 
    {
        DecodeCoreSideBits(hAvs3Dec->hDecCore[ch], hBitstream);
    }

    // grouping info
    for (ch = 0; ch < nChans; ch++) {
        DecodeGroupBits(hAvs3Dec->hDecCore[ch], hBitstream);
        numGroups[ch] = hAvs3Dec->hDecCore[ch]->numGroups;
    }

    // decode mode side info
    DecodeHoaSideBits(hAvs3Dec->hDecHoa, hBitstream);

    /* Bits verification */
    availableBits = (short)GetAvailableBits(hAvs3Dec->bitsPerFrame, hBitstream->nextBitPos, numGroups, nChans, hAvs3Dec->nnTypeConfig);

    HoaSplitBytesGroup(hAvs3Dec->hDecHoa->hHoaConfig, channelBytes, hAvs3Dec->hDecHoa->groupBitsRatio, hAvs3Dec->hDecHoa->bitsRatio, availableBits);

    // decode QC bits
    for (ch = 0; ch < nChans; ch++) 
    {
        DecodeQcBits(hAvs3Dec->hDecCore[ch], hAvs3Dec->nnTypeConfig, hBitstream, channelBytes[ch]);
    }

    // inverse QC for all channels
    Avs3InverseQC(hAvs3Dec, nChans);

    /* Inverse DMX */
    Avs3HoaInverseDMX(hAvs3Dec);

    // inverse MDCT and OLA
    for (ch = 0; ch < nChans; ch++)
    {
        hDecCore = hAvs3Dec->hDecCore[ch];

        // post synthesis, including bwe, tns, fd shaping, degrouping and inv MDCT
        Avs3PostSynthesis(hDecCore, hAvs3Dec->hDecHoa->decSignalInput[ch], 0);
    }

    /* HOA synthesis */
    HoaPostSynthesisFilter(hAvs3Dec, synth, frameLength);

    /* update */
    for (ch = 0; ch < hAvs3Dec->numChansOutput; ch++)
    {
        Mvf2f(hAvs3Dec->hDecHoa->decHoaDelayBuffer[ch] + frameLength, hAvs3Dec->hDecHoa->decHoaDelayBuffer[ch], frameLength);
    }

    return;
}
