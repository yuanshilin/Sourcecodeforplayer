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
#include <math.h>
#include <assert.h>
#include "avs3_options.h"
#include "avs3_rom_com.h"
#include "avs3_stat_com.h"
#include "avs3_prot_com.h"
#include "avs3_prot_dec.h"

void ResetBitstream(AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    short i;

    for (i = 0; i < MAX_BS_BYTES; i++)
    {
        hBitstream->bitstream[i] = 0;
    }

    hBitstream->nextBitPos = 0;

    return;
}


short Avs3ParseBsFrameHeader(
    AVS3DecoderHandle hAvs3Dec,
    uint8_t *headerBs,
    int16_t isInitFrame,
    uint16_t *crcBs
)
{
    uint32_t nextBitPos = 0;

    // Sync word, 12 bits
    uint16_t syncWord;
    syncWord = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_SYNC_WORD);
    // Check sync word
    if (syncWord != SYNC_WORD_COMPAT) {
        return AVS3_FALSE;
    }

    // audio codec id, 4 bits
    uint16_t audioCodecId;
    audioCodecId = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_AUDIO_CODEC_ID);
    // Check audio codec id, should be 2 for HW branch
    if (audioCodecId != 2) {
        return AVS3_FALSE;
    }

    // anc data index, fixed to 0 in HW branch, 1 bit
    uint16_t ancDataIndex;
    ancDataIndex = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_ANC_DATA_INDEX);
    // Check anc data index
    if (ancDataIndex == 1) {
        return AVS3_FALSE;
    }

    // NN type, 3 bit
    // 0 for default main, 1 for default lc
    uint16_t nnTypeConfig;
    nnTypeConfig = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_NN_TYPE);

    // coding profile, 3 bit
    // 0 for mono/stereo/mc, 1 for channel + obj mix, 2 for hoa
    uint16_t codingProfile;
    codingProfile = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_CODING_PROFILE);

    // sampling rate index, 4 bit
    uint16_t samplingRateIdx;
    samplingRateIdx = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_SAMPLING_RATE_INDEX);

    // CRC first part
    uint16_t crcTmp;
    crcTmp = (uint16_t)GetNextIndice(headerBs, &nextBitPos, AVS3_BS_BYTE_SIZE);
    crcTmp = crcTmp << AVS3_BS_BYTE_SIZE;

    uint16_t channelNumIdx = 0;
    uint16_t numObjs = 0;
    uint16_t hoaOrder;
    uint16_t soundBedType;
    uint16_t bitrateIdxPerObj = 0;
    uint16_t bitrateIdxBedMc = 0;
    if (codingProfile == 0) {
        // channel number index
        // for mono/stereo/mc, 7 bits
        channelNumIdx = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_CHANNEL_NUMBER_INDEX);
    } else if (codingProfile == 1) {
        // sound bed type, 2bits
        soundBedType = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_SOUNDBED_TYPE);

        if (soundBedType == 0) {
            // for only objs
            // object number, 7 bits
            numObjs = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_NUM_OBJS);
            numObjs += 1;
            // bitrate index for each obj, 4 bits
            bitrateIdxPerObj = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_BITRATE_INDEX);
        }
        else if (soundBedType == 1) {
            // for MC+objs
            // channel number index, 7 bits
            channelNumIdx = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_CHANNEL_NUMBER_INDEX);
            // bitrate index for sound bed, 4 bits
            bitrateIdxBedMc = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_BITRATE_INDEX);

            // object number, 7 bits
            numObjs = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_NUM_OBJS);
            numObjs += 1;
            // bitrate index for each obj, 4 bits
            bitrateIdxPerObj = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_BITRATE_INDEX);
        }
    } else if (codingProfile == 2) {
        // for HOA, 4 bits
        hoaOrder = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_HOA_ORDER);
        hoaOrder += 1;
    }

    // resolution, i.e. bitDepth, 2 bits
    uint16_t resolution;
    resolution = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_RESOLUTION);

    // bitrate index, 4 bits
    uint16_t bitrateIdx = 0;
    if (codingProfile != 1) {
        bitrateIdx = (uint16_t)GetNextIndice(headerBs, &nextBitPos, NBITS_BITRATE_INDEX);
    }

    // second part of CRC, 8 bits
    crcTmp += (uint16_t)GetNextIndice(headerBs, &nextBitPos, AVS3_BS_BYTE_SIZE);

    // Config decoder
    // sampling frequency
    hAvs3Dec->outputFs = avs3SamplingRateTable[samplingRateIdx];
    hAvs3Dec->bitrateBedMc = 0;
    // frame length
    hAvs3Dec->frameLength = GetFrameLength(hAvs3Dec->outputFs);

    // bitdepth
    if (resolution == 0) {
        hAvs3Dec->bitDepth = 8;
    } else if (resolution == 1) {
        hAvs3Dec->bitDepth = 16;
    } else if (resolution == 2) {
        hAvs3Dec->bitDepth = 24;
    }

    // NN type config
    hAvs3Dec->nnTypeConfig = (NnTypeConfig)nnTypeConfig;

    // Codec format and bitrate
    if (codingProfile == 0) {
        // mono/stereo/mc
        hAvs3Dec->isMixedContent = 0;
        hAvs3Dec->channelNumConfig = (ChannelNumConfig)channelNumIdx;
        if (hAvs3Dec->channelNumConfig == CHANNEL_CONFIG_MONO) {
            // mono
            hAvs3Dec->avs3CodecFormat = AVS3_MONO_FORMAT;
            hAvs3Dec->avs3CodecCore = AVS3_MDCT_CORE;
            hAvs3Dec->numChansOutput = 1;
        } else if (hAvs3Dec->channelNumConfig == CHANNEL_CONFIG_STEREO) {
            // stereo
            hAvs3Dec->avs3CodecFormat = AVS3_STEREO_FORMAT;
            hAvs3Dec->avs3CodecCore = AVS3_MDCT_CORE;
            hAvs3Dec->numChansOutput = 2;
        } else if (hAvs3Dec->channelNumConfig <= CHANNEL_CONFIG_MC_7_1_4) {
            // mc
            hAvs3Dec->avs3CodecFormat = AVS3_MC_FORMAT;
            hAvs3Dec->avs3CodecCore = AVS3_MDCT_CORE;
            for (int16_t i = 0; i < AVS3_SIZE_MC_CONFIG_TABLE; i++) {
                if (hAvs3Dec->channelNumConfig == mcChannelConfigTable[i].channelNumConfig) {
                    hAvs3Dec->numChansOutput = mcChannelConfigTable[i].numChannels;
                }
            }
            hAvs3Dec->hasLfe = 1;
            if (hAvs3Dec->channelNumConfig == CHANNEL_CONFIG_MC_4_0) {
                hAvs3Dec->hasLfe = 0;
            }
        } else {
            return AVS3_FALSE;
        }
    } else if (codingProfile == 1) {
        // mix
        hAvs3Dec->isMixedContent = 1;

        // sound bed type
        hAvs3Dec->soundBedType = soundBedType;

        if (hAvs3Dec->soundBedType == 0) {
            // object number
            hAvs3Dec->numObjsOutput = numObjs;
            hAvs3Dec->numChansOutput = numObjs;

            if (numObjs == 1) {
                hAvs3Dec->avs3CodecFormat = AVS3_MONO_FORMAT;
                hAvs3Dec->avs3CodecCore = AVS3_MDCT_CORE;
            }
            else if (numObjs == 2) {
                hAvs3Dec->avs3CodecFormat = AVS3_STEREO_FORMAT;
                hAvs3Dec->avs3CodecCore = AVS3_MDCT_CORE;
            }
            else if (numObjs >= 3) {
                hAvs3Dec->avs3CodecFormat = AVS3_MC_FORMAT;
                hAvs3Dec->avs3CodecCore = AVS3_MDCT_CORE;
            }

            // channelNumConfig not used for pure objs
            hAvs3Dec->channelNumConfig = CHANNEL_CONFIG_UNKNOWN;

            // bitrate per obj
            hAvs3Dec->bitratePerObj = codecBitrateConfigTable[CHANNEL_CONFIG_MONO].bitrateTable[bitrateIdxPerObj];

            // total bitrate, only objs
            hAvs3Dec->totalBitrate = hAvs3Dec->numObjsOutput * hAvs3Dec->bitratePerObj;

            // for pure objs, lfe not exist
            hAvs3Dec->hasLfe = 0;
        }
        else if (hAvs3Dec->soundBedType == 1) {
            hAvs3Dec->avs3CodecFormat = AVS3_MC_FORMAT;
            hAvs3Dec->avs3CodecCore = AVS3_MDCT_CORE;

            // channelNumIdx for sound bed
            hAvs3Dec->channelNumConfig = (ChannelNumConfig)channelNumIdx;

            // sound bed bitrate
            hAvs3Dec->bitrateBedMc = codecBitrateConfigTable[hAvs3Dec->channelNumConfig].bitrateTable[bitrateIdxBedMc];

            // numChannels for sound bed
            for (int16_t i = 0; i < AVS3_SIZE_MC_CONFIG_TABLE; i++) {
                if (hAvs3Dec->channelNumConfig == mcChannelConfigTable[i].channelNumConfig) {
                    hAvs3Dec->numChansOutput = mcChannelConfigTable[i].numChannels;
                }
            }

            // object number
            hAvs3Dec->numObjsOutput = numObjs;

            // bitrate per obj
            hAvs3Dec->bitratePerObj = codecBitrateConfigTable[CHANNEL_CONFIG_MONO].bitrateTable[bitrateIdxPerObj];

            // add num chans and num objs to get total chans
            hAvs3Dec->numChansOutput += hAvs3Dec->numObjsOutput;

            // total bitrate, sound bed + objs
            hAvs3Dec->totalBitrate = hAvs3Dec->bitrateBedMc + hAvs3Dec->numObjsOutput * hAvs3Dec->bitratePerObj;

            // for sound bed + obj mix
            // if sound bed is stereo/MC4.0, no LFE, if sound bed is other mc configs, with lfe
            if (hAvs3Dec->channelNumConfig == CHANNEL_CONFIG_STEREO ||
                hAvs3Dec->channelNumConfig == CHANNEL_CONFIG_MC_4_0) {
                hAvs3Dec->hasLfe = 0;
            }
            else {
                hAvs3Dec->hasLfe = 1;
            }
        }
    } else if (codingProfile == 2) {
        // hoa
        hAvs3Dec->channelNumConfig = CHANNEL_CONFIG_UNKNOWN;
        if (hoaOrder == 1) {
            hAvs3Dec->channelNumConfig = CHANNEL_CONFIG_HOA_ORDER1;
        } else if (hoaOrder == 2) {
            hAvs3Dec->channelNumConfig = CHANNEL_CONFIG_HOA_ORDER2;
        } else if (hoaOrder == 3) {
            hAvs3Dec->channelNumConfig = CHANNEL_CONFIG_HOA_ORDER3;
        }

        hAvs3Dec->avs3CodecFormat = AVS3_HOA_FORMAT;
        hAvs3Dec->avs3CodecCore = AVS3_MDCT_CORE;
        hAvs3Dec->numChansOutput = (hoaOrder + 1) * (hoaOrder + 1);

        hAvs3Dec->isMixedContent = 0;
    }

    // total bitrate
    if (hAvs3Dec->isMixedContent == 0) {
        hAvs3Dec->totalBitrate = codecBitrateConfigTable[hAvs3Dec->channelNumConfig].bitrateTable[bitrateIdx];
    }

    // if not first frame
    if (isInitFrame == 0) {
        // copy crc bs
        *crcBs = crcTmp;

        // update bitrate
        hAvs3Dec->lastTotalBrate = hAvs3Dec->totalBitrate;
        hAvs3Dec->bitsPerFrame = (int32_t)(((float)hAvs3Dec->totalBitrate / (float)hAvs3Dec->outputFs) * hAvs3Dec->frameLength);

        // subtract frame bs header bits
        if (hAvs3Dec->isMixedContent == 0) {
            if (hAvs3Dec->avs3CodecFormat == AVS3_MONO_FORMAT) {
                hAvs3Dec->bitsPerFrame -= NBITS_FRAME_HEADER_MONO;
            }
            else if (hAvs3Dec->avs3CodecFormat == AVS3_STEREO_FORMAT) {
                hAvs3Dec->bitsPerFrame -= NBITS_FRAME_HEADER_STEREO;
            }
            else if (hAvs3Dec->avs3CodecFormat == AVS3_MC_FORMAT) {
                hAvs3Dec->bitsPerFrame -= NBITS_FRAME_HEADER_MC;
            }
            else if (hAvs3Dec->avs3CodecFormat == AVS3_HOA_FORMAT) {
                hAvs3Dec->bitsPerFrame -= NBITS_FRAME_HEADER_HOA;
            }
        }
        else {
            if (hAvs3Dec->soundBedType == 0) {
                hAvs3Dec->bitsPerFrame -= NBITS_FRAME_HEADER_MIX_SBT0;
            }
            else if (hAvs3Dec->soundBedType == 1) {
                hAvs3Dec->bitsPerFrame -= NBITS_FRAME_HEADER_MIX_SBT1;
            }
        }
    }

    return AVS3_TRUE;
}


// short ReadBitstream(AVS3DecoderHandle hAvs3Dec, FILE* fBitstream) 
// {
//     short bytesPerFrame = 0;

//     uint8_t* bitstream = hAvs3Dec->hBitstream->bitstream;

// #ifdef CRC_CHECK
//     uint16_t crcBs = 0, crcResult = 0;          // crc info from BS and calculated at decoder
// #endif

//     if (fBitstream == NULL) 
//     {
//         return AVS3_FALSE;
//     }

// #ifdef BS_HEADER_COMPAT
//     /* Read frame header info */
//     Avs3ParseBsFrameHeader(hAvs3Dec, fBitstream, 0, &crcBs);
// #endif

//     bytesPerFrame = (uint32_t)(ceil((float)hAvs3Dec->bitsPerFrame / 8));

//     /* frame payload */
//     fread(bitstream, sizeof(uint8_t), bytesPerFrame, fBitstream);

// #ifdef CRC_CHECK
//     /* CRC check */
//     crcResult = Crc16(bitstream, bytesPerFrame);
//     if (crcResult != crcBs) {
//         return AVS3_FALSE;
//     }
// #endif

//     return AVS3_TRUE;
// }

uint16_t GetNextIndice(uint8_t *bitstream, uint32_t *nextBitPos, int16_t numBits)
{
    uint16_t value;
    uint32_t byteIndex;
    uint16_t bitIndex;
    uint8_t mask;

    byteIndex = (*nextBitPos) >> 3;
    bitIndex = (*nextBitPos) & 0x7;
    mask = 1 << (7 - bitIndex);

    value = 0;
    for (int16_t i = 0; i < numBits; i++) {

        value <<= 1;
        if ((bitstream[byteIndex] & mask) != 0) {
            value += 1;
        }

        mask >>= 1;
        if (mask == 0) {
            byteIndex += 1;
            bitIndex = 0x7;
            mask = 0x80;
        }
    }

    *nextBitPos += numBits;

    return value;
}


void DecodeStereoSideBits(AVS3DecoderHandle hAvs3Dec, AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    AVS3_STEREO_DEC_HANDLE hDecStereo = hAvs3Dec->hDecStereo;

    if (hDecStereo->useMcr == 0) {
        // MS stereo params
        /* ms flag */
        hDecStereo->isMS = (short)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_MS_FLAG);

        if (hDecStereo->isMS) {
            hDecStereo->ILD = (short)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_ENERGY_BALENCE);
        }

        /* bit split ratio*/
        hDecStereo->bitsRatio = (short)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SPLIT_STEREO);
    }
    else {
        // MCR stereo params
        MCR_CONFIG_HANDLE mcrConfig = &hDecStereo->mcrConfig;
        MCR_DATA_HANDLE mcrData = &hDecStereo->mcrData;

        // isShortWin flag for left channel
        int16_t isShortWin = (hAvs3Dec->hDecCore[0]->transformType == ONLY_SHORT_WINDOW);

        // MCR vq indices
        // for short frame, 8 bits for each vq index
        // for long/transition frame, 9 bits for each vq index
        for (int16_t i = 0; i < mcrConfig->vqVecNum[isShortWin]; i++) {
            mcrData->vqIdx[0][i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, mcrConfig->vqNumBits[isShortWin]);
            mcrData->vqIdx[1][i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, mcrConfig->vqNumBits[isShortWin]);
        }
    }

    return;
}


void DecodeMcSideBits(
    AVS3_MC_DEC_HANDLE hDecMc, 
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream, 
    short chBitRatios[MAX_CHANNELS]
)
{
    short i, pair;
    AVS3_MC_PAIR_DATA_HANDLE hPair;
    short channelPairIndex;

    hDecMc->hasSilFlag = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_HASSILFLAG);
    if (hDecMc->hasSilFlag) {
        for (i = 0; i < hDecMc->channelNum + hDecMc->objNum; i++) {
            if ((hDecMc->lfeExist) && (i == hDecMc->lfeChIdx)) {
                hDecMc->silFlag[i] = 0;
                continue;
            }
            hDecMc->silFlag[i] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_SILFLAG);
        }
    }
    else {
        /* set to default value */
        for (i = 0; i < hDecMc->channelNum + hDecMc->objNum; i++) {
            hDecMc->silFlag[i] = 0;
        }
    }

    /* pairing cnt */
    hDecMc->pairCnt = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, PAIR_NUM_DATA_BITS);

    for (i = 0; i < MAX_CHANNELS; i++) {
        hDecMc->mcIld[i] = MC_ILD_CBLEN;
    }

    for (pair = 0; pair < hDecMc->pairCnt; pair++)
    {
        hPair = &(hDecMc->hPair[pair]);

        /*get channel pair index from BS*/
        channelPairIndex = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, hDecMc->bitsPairIndex);
        Index2PairMapping(hPair, channelPairIndex, hDecMc->channelNum + hDecMc->objNum);

        hDecMc->mcIld[hPair->ch1] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, MC_EB_BITS);
        hDecMc->mcIld[hPair->ch2] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, MC_EB_BITS);
    }

    int j = 0;
    for (i = 0; i < hDecMc->channelNum + hDecMc->objNum; i++) {
        if ((hDecMc->lfeExist) && (i == hDecMc->lfeChIdx)) {
            continue;
        }
        if (hDecMc->silFlag[i] == 1) {
            continue;
        }
        chBitRatios[j] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_MC_RATIO);
        j++;
    }

    return;
}


void DecodeHoaSideBits(AVS3_HOA_DEC_DATA_HANDLE hDecHoa, AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream) 
{
    short i, groupIdx;
    short nTotalChans;
    short groupChOffset;

    nTotalChans = hDecHoa->hHoaConfig->nTotalChansTransport;

    hDecHoa->sceneType = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, 4);

    hDecHoa->hHoaConfig->spatialAnalysis = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, 1);
    
    if (hDecHoa->hHoaConfig->spatialAnalysis)
    {
        hDecHoa->numVL = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, 4);
        assert(hDecHoa->numVL > 0);
    }

    for (i = 0; i < hDecHoa->numVL; i++)
    {
        hDecHoa->basisIdx[i] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, HOA_BASIS_BITS);
    }

    /* read bitstream by groups */
    for (groupIdx = 0; groupIdx < hDecHoa->hHoaConfig->nTotalChanGroups; groupIdx++)
    {
        groupChOffset = hDecHoa->hHoaConfig->groupChOffset[groupIdx];

        /* total channel pair in group */
        hDecHoa->pairIdx[groupIdx] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, 4);

        assert(hDecHoa->pairIdx[groupIdx] >= 0);

        if (hDecHoa->pairIdx[groupIdx] > 0)
        {
            /* channels index */
            for (i = 0; i < hDecHoa->pairIdx[groupIdx]; i++)
            {
                hDecHoa->chIdx[groupIdx][i] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, hDecHoa->hHoaConfig->groupIndexBits[groupIdx]);

                hDecHoa->dmxMode[groupIdx][i] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, 1) + 1;
                assert(hDecHoa->dmxMode[groupIdx][i] == DMX_FULL_MS || hDecHoa->dmxMode[groupIdx][i] == DMX_SFB_MS);

                if (hDecHoa->dmxMode[groupIdx][i] == DMX_SFB_MS) {

                    for (short sfb = 0; sfb < N_SFB_HOA_LBR - 1; sfb++)
                    {
                        hDecHoa->sfbMask[groupIdx][i][sfb] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, 1);
                        assert(hDecHoa->sfbMask[groupIdx][i][sfb] == 0 || hDecHoa->sfbMask[groupIdx][i][sfb] == 1);
                    }
                }
                else
                {
                    SetShort(hDecHoa->sfbMask[groupIdx][i], 1, N_SFB_HOA_LBR - 1);
                }
            }

            /* group ILD */
            for (i = 0; i < hDecHoa->hHoaConfig->groupChans[groupIdx]; i++)
            {
                hDecHoa->groupILD[i + groupChOffset] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, HOA_ILD_BITS);
            }
        }
        else
        {
            hDecHoa->pairIdx[groupIdx] = 0;

            for (i = 0; i < hDecHoa->hHoaConfig->groupChans[groupIdx]; i++)
            {
                hDecHoa->groupILD[i + groupChOffset] = MC_ILD_CBLEN;
            }
        }

        /* group bits */
        hDecHoa->groupBitsRatio[groupIdx] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, 4);

        /* Bits ratio */
        for (i = 0; i < hDecHoa->hHoaConfig->groupChans[groupIdx]; i++)
        {
            hDecHoa->bitsRatio[groupIdx][i] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, 4);
        }
    }

  

    return;
}


static void DecodeFdShapingSideBits(AVS3_DEC_CORE_HANDLE hDecCore, AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    // decode VQ indices
    if (hDecCore->lsfLbrFlag == 0) {
        // high bitrate LSF VQ indices
        hDecCore->lsfVqIndex[0] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, LSF_STAGE1_CB1_NBITS_HBR);
        hDecCore->lsfVqIndex[1] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, LSF_STAGE1_CB2_NBITS_HBR);
        hDecCore->lsfVqIndex[2] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, LSF_STAGE2_CB1_NBITS_HBR);
        hDecCore->lsfVqIndex[3] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, LSF_STAGE2_CB2_NBITS_HBR);
        hDecCore->lsfVqIndex[4] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, LSF_STAGE2_CB3_NBITS_HBR);
        hDecCore->lsfVqIndex[5] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, LSF_STAGE2_CB4_NBITS_HBR);
        hDecCore->lsfVqIndex[6] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, LSF_STAGE2_CB5_NBITS_HBR);
    }
    else if (hDecCore->lsfLbrFlag == 1) {
        // low bitrate LSF VQ indices
        hDecCore->lsfVqIndex[0] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, LSF_STAGE1_CB1_NBITS_LBR);
        hDecCore->lsfVqIndex[1] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, LSF_STAGE1_CB2_NBITS_LBR);
        hDecCore->lsfVqIndex[2] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, LSF_STAGE2_CB1_NBITS_LBR);
        hDecCore->lsfVqIndex[3] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, LSF_STAGE2_CB2_NBITS_LBR);
        hDecCore->lsfVqIndex[4] = GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, LSF_STAGE2_CB3_NBITS_LBR);
    }

    return;
}


static void DecodeTnsSideBits(AVS3_DEC_CORE_HANDLE hDecCore, AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t found;
    int16_t nBits;
    uint16_t code;
    TnsBsParam *tnsBsParam;

    // loop over filters
    for (int16_t i = 0; i < TNS_MAX_FILTER_NUM; i++) {

        // get handle
        tnsBsParam = &(hDecCore->tnsData.bsParam[i]);

        // get enable flag
        tnsBsParam->enable = (short)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, TNS_NBITS_ENABLE);

        // enabled, read order and huffman codes
        if (tnsBsParam->enable == 1) {

            // get order
            tnsBsParam->order = (short)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, TNS_NBITS_ORDER);
            tnsBsParam->order += 1;

            // get huffman codes
            for (int16_t j = 0; j < tnsBsParam->order; j++) {

                found = 0;
                nBits = 0;
                code = 0;

                while (found == 0) {

                    // read 1 bit from bitstream
                    code = (code << 1) + GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, 1);
                    nBits++;

                    for (int16_t k = 0; k < N_TNS_COEFF_CODES; k++) {
                        // code and nbits same as table item, found
                        if (code == tnsCodingTable[j][k].code &&
                            nBits == tnsCodingTable[j][k].nBits) {

                            tnsBsParam->parcorHuffCode[j] = code;
                            tnsBsParam->parcorNbits[j] = nBits;

                            found = 1;
                            break;
                        }
                    }
                }
            }
        }
    }

    return;
}


static void DecodeBweSideBits(AVS3_DEC_CORE_HANDLE hDecCore, AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t i;
    int16_t flag;
    BweConfigHandle bweConfig = &hDecCore->bweConfig;
    BweDecDataHandle bweDecData = &hDecCore->bweDecData;

    // read sfb envelope
    for (i = 0; i < bweConfig->numSfb; i++) {
        bweDecData->sfbEnvQIdx[i] = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_BWE_ENV);
    }

    // read whitening level
    for (i = 0; i < bweConfig->numTiles; i++) {
        flag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_BWE_WHITEN_ONOFF);
        if (flag == 0) {
            bweDecData->whiteningLevel[i] = BWE_WHITENING_OFF;
        }
        else {
            flag = (int16_t)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_BWE_WHITEN_MIDHIGH);
            if (flag == 0) {
                bweDecData->whiteningLevel[i] = BWE_WHITENING_MID;
            }
            else {
                bweDecData->whiteningLevel[i] = BWE_WHITENING_HIGH;
            }
        }
    }

    return;
}


void DecodeCoreSideBits(AVS3_DEC_CORE_HANDLE hDecCore, AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    // decode transform type
    hDecCore->transformType = (short)GetNextIndice(hBitstream->bitstream, &hBitstream->nextBitPos, NBITS_TRANSFORM_TYPE);

    // decode Fd spectrum shaping info
    DecodeFdShapingSideBits(hDecCore, hBitstream);

    // decode TNS info
    DecodeTnsSideBits(hDecCore, hBitstream);

    // decode BWE info
    if (hDecCore->bwePresent == 1) {
        DecodeBweSideBits(hDecCore, hBitstream);
    }

    return;
}


void DecodeGroupBits(AVS3_DEC_CORE_HANDLE hDecCore, AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream)
{
    int16_t j;

    if (hDecCore->transformType == ONLY_SHORT_WINDOW) {
        // Short window
        // get number groups
        hDecCore->numGroups = GetNextIndice(hBitstream->bitstream, &(hBitstream->nextBitPos), 1) + 1;

        // get group indicator
        if (hDecCore->numGroups == N_GROUP_SHORT_WIN) {
            for (j = 0; j < N_BLOCK_SHORT; j++) {
                hDecCore->groupIndicator[j] = GetNextIndice(hBitstream->bitstream, &(hBitstream->nextBitPos), 1);
            }
        }
        else {
            SetShort(hDecCore->groupIndicator, 0, N_BLOCK_SHORT);
        }
    }
    else {
        // Long window
        // Set number groups to one, reset group indicator to all-zero
        hDecCore->numGroups = 1;
        SetShort(hDecCore->groupIndicator, 0, N_BLOCK_SHORT);
    }

    return;
}


void DecodeQcBits(
    AVS3_DEC_CORE_HANDLE hDecCore,
    NnTypeConfig nnTypeConfig,
    AVS3_BSTEREAM_DATA_DEC_HANDLE hBitstream,
    const short channelBytes)
{
    NeuralQcData *neuralQcData = &hDecCore->neuralQcData;
    uint32_t* nextBitPos = &(hBitstream->nextBitPos);

    // init QC data structure
    InitNeuralQcData(neuralQcData);

    // read side info
    if (nnTypeConfig == NN_TYPE_DEFAULT_MAIN) {
        neuralQcData->isFeatAmplified = GetNextIndice(hBitstream->bitstream, nextBitPos, NBITS_IS_FEAT_AMPLIFIED);
        neuralQcData->scaleQIdx = GetNextIndice(hBitstream->bitstream, nextBitPos, NBITS_FEATURE_SCALE);
    }
    else if (nnTypeConfig == NN_TYPE_DEFAULT_LC) {
        neuralQcData->scaleQIdx = GetNextIndice(hBitstream->bitstream, nextBitPos, NBITS_FEATURE_SCALE_LC);
    }

    if (hDecCore->numGroups == 1) {
        neuralQcData->nfParamQIdx[0] = GetNextIndice(hBitstream->bitstream, nextBitPos, NBITS_NF_PARAM);
    }
    else {
        neuralQcData->nfParamQIdx[0] = GetNextIndice(hBitstream->bitstream, nextBitPos, NBITS_NF_PARAM);
        neuralQcData->nfParamQIdx[1] = GetNextIndice(hBitstream->bitstream, nextBitPos, NBITS_NF_PARAM);
    }

    neuralQcData->contextNumBytes = GetNextIndice(hBitstream->bitstream, nextBitPos, NBITS_CONTEXT_NUM_BYTES);

    // determine base num bytes
    neuralQcData->baseNumBytes = channelBytes - neuralQcData->contextNumBytes;

    // read context bitstream
    for (int j = 0; j < neuralQcData->contextNumBytes; j++) {
        neuralQcData->contextBitstream[j] = (uint8_t)GetNextIndice(hBitstream->bitstream, nextBitPos, 8);
    }

    // read base bitstream
    for (int j = 0; j < neuralQcData->baseNumBytes; j++) {
        neuralQcData->baseBitstream[j] = (uint8_t)GetNextIndice(hBitstream->bitstream, nextBitPos, 8);
    }

    return;
}
