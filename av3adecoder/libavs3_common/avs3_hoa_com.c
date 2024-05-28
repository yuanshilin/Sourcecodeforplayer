#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "avs3_prot_com.h"
#include "avs3_rom_com.h"
#include "avs3_cnst_com.h"

static float qsin(const short idx);

static float qcos(const short idx);

static void Avs3Hoa3v2(const float saz, const float caz, const float sel, const float cel, float *res);

void HoaSplitBytesGroup(AVS3_HOA_CONFIG_DATA_HANDLE hConfig, short* bytesChannels, short* groupRatio, short ratio[MAX_HOA_DMX_GROUPS][MAX_HOA_DMX_CHANNELS], const short totalBits)
{
    short ch, groupIdx;
    short sumRatio;
    short bits = 0;
    short nChans, offset;
    short byetsPerRange;
    short resBytesGroup;
    short tmp;
    short groupBytes[MAX_HOA_DMX_GROUPS];

    /* residual bits */
    const short resBits = totalBits % AVS3_BS_BYTE_SIZE;
    assert(resBits >= 0 && resBits < AVS3_BS_BYTE_SIZE);

    const short totalBytes = (short)floor((float)totalBits / AVS3_BS_BYTE_SIZE);
    short usedBytes = totalBytes;

    for (groupIdx = 0; groupIdx < hConfig->nTotalChanGroups - 1; groupIdx++) 
    {
        groupBytes[groupIdx] = (short)(totalBytes * (((float)groupRatio[groupIdx]) / HOA_BITS_RATIO_RANGE));

        usedBytes -= groupBytes[groupIdx];
    }

    groupBytes[hConfig->nTotalChanGroups - 1] = usedBytes;


    groupIdx = 0;
    while (groupIdx < hConfig->nTotalChanGroups)
    {
        sumRatio = 0;
        nChans = hConfig->groupChans[groupIdx];
        offset = hConfig->groupChOffset[groupIdx];

        for (ch = 0; ch < nChans; ch++)
        {
            sumRatio += ratio[groupIdx][ch];
        }

        byetsPerRange = (short)floor((float)groupBytes[groupIdx] / sumRatio);
        resBytesGroup = groupBytes[groupIdx] % sumRatio;

        /* first step bits split */
        for (ch = 0; ch < nChans; ch++)
        {
            bytesChannels[ch + offset] = byetsPerRange * ratio[groupIdx][ch];
        }

        /* tuning residual bytes inner bitrate */
        if (resBytesGroup >= nChans)
        {
            tmp = (short)floor((float)resBytesGroup / nChans);

            for (ch = 0; ch < nChans; ch++)
            {
                bytesChannels[ch + /*groupIdx **/ offset] += tmp;
            }

            if (resBytesGroup != nChans)
            {
                bytesChannels[0 + /*groupIdx **/ offset] += (resBytesGroup % nChans);
            }
        }
        else
        {
            bytesChannels[0 + /*groupIdx **/ offset] += resBytesGroup;
        }

        /* Total bits verification */
        for (ch = 0; ch < nChans; ch++)
        {
            bits += bytesChannels[ch + /*groupIdx **/ offset];
        }

        groupIdx++;
    }

    bits *= AVS3_BS_BYTE_SIZE;
    bits += resBits;

    assert(bits == totalBits);

    return;
}


float qsin(const short idx)
{
    if (idx <= L_SIN_TABLE_256)
    {
        return avs3_hoa_sin_table[idx];
    }
    else if (idx <= L_SIN_TABLE_512)
    {
        return avs3_hoa_sin_table[L_SIN_TABLE_512 - idx];
    }
    else if (idx <= L_SIN_TABLE_768)
    {
        return -avs3_hoa_sin_table[idx - L_SIN_TABLE_512];
    }
    else
    {
        return -avs3_hoa_sin_table[L_SIN_TABLE_1024 - idx];
    }
}

float qcos(const short idx)
{
    if (idx <= L_SIN_TABLE_256)
    {
        return avs3_hoa_sin_table[L_SIN_TABLE_256 - idx];
    }
    else if (idx <= L_SIN_TABLE_512)
    {
        return -avs3_hoa_sin_table[idx - L_SIN_TABLE_256];
    }
    else if (idx <= L_SIN_TABLE_768)
    {
        return -avs3_hoa_sin_table[L_SIN_TABLE_768 - idx];
    }
    else
    {
        return avs3_hoa_sin_table[idx - L_SIN_TABLE_768];
    }
}

static void Avs3Hoa3v2(const float saz, const float caz, const float sel, const float cel, float *res)
{

    const float r00 = (float)sqrt(1.f / 4.f / AVS3_PI);
    const float r01 = (float)sqrt(3.f / 4.f / AVS3_PI);
    const float r04 = (float)sqrt(5.f / 16.f / AVS3_PI);
    const float r05 = (float)sqrt(15.f / 4.f / AVS3_PI);
    const float r07 = (float)sqrt(15.f / 16.f / AVS3_PI);
    const float r09 = (float)sqrt(7.f / 16.f / AVS3_PI);
    const float r10 = (float)sqrt(21.f / 32.f / AVS3_PI);
    const float r12 = (float)sqrt(105.f / 16.f / AVS3_PI);
    const float r14 = (float)sqrt(35.f / 32.f / AVS3_PI);

    float t;

    float saz2 = saz * saz;
    float sel2 = sel * sel;
    float caz2 = caz * caz;
    float cel2 = cel * cel;
    float scaz = saz * caz;

    res[0] = r00; // (0, +0)
    res[2] = r01 * sel; // (1, +0)
    t = r01 * cel;
    res[1] = t * saz; // (1, -1)
    res[3] = t * caz; // (1, +1)
    res[6] = r04 * (3.f * sel2 - 1.f); // (2, +0)
    t = r05 * cel * sel;
    res[5] = t * saz; // (2, -1)
    res[7] = t * caz; // (2, +1)
    t = r07 * cel2;
    res[4] = t * 2.f * scaz; // (2, -2)
    res[8] = t * (2.f * caz2 - 1.f); // (2, +2)
    res[12] = r09 * (5.f * sel2 * sel - 3.f * sel); // (3, +0)
    t = r10 * cel * (5.f * sel2 - 1.f);
    res[11] = t * saz; // (3, -1)
    res[13] = t * caz; // (3, +1)
    t = r12 * cel2 * sel;
    res[10] = t * 2.f * scaz; // (3, -2)
    res[14] = t * (2.f * caz2 - 1.f); // (3, +2)
    t = r14 * cel2 * cel;
    res[9] = t * (3.f * saz - 4.f * saz2 * saz); // (3, -3)
    res[15] = t * (4.f * caz2 * caz - 3.f * caz); // (3, +3)

    return;
}


static void GetGroupConfiguration(AVS3_HOA_CONFIG_DATA_HANDLE hConfig, const HOA_GROUP_CONFIG* cfg) {

    short i, bitrateIdx;
    hConfig->nTotalChansTransport = 0;

    SetShort(hConfig->groupChOffset, 0, MAX_HOA_DMX_GROUPS + 1);

    for (bitrateIdx = 0; bitrateIdx < HOA_SIZE_BITRATE_TABLE; bitrateIdx++)
    {
        if (hConfig->totalBitrate == cfg[bitrateIdx].bitrate)
        {
            hConfig->spatialAnalysis = cfg[bitrateIdx].spatialAnalysis;

            /* total number of channel groups */
            hConfig->nTotalChanGroups = cfg[bitrateIdx].nGroups;

            /* bitrate channel configuration */
            MvShort2Short(cfg[bitrateIdx].dmxGroup, hConfig->groupChans, hConfig->nTotalChanGroups);

            /* bitrate bits ratio */
            Mvf2f(cfg[bitrateIdx].bitsRatio, hConfig->groupBitsRatio, hConfig->nTotalChanGroups);

            /* core band width */
            MvShort2Short(cfg[bitrateIdx].coreBwidth, hConfig->groupCodeLines, MAX_HOA_DMX_GROUPS);

            /* recursion for get channel bitrate offset */
            for (i = 1; i < hConfig->nTotalChanGroups + 1; i++)
            {
                hConfig->groupChOffset[i] = hConfig->groupChans[i - 1] + hConfig->groupChOffset[i - 1];
            }

            for (i = 0; i < hConfig->nTotalChanGroups; i++)
            {
                hConfig->nTotalChansTransport += hConfig->groupChans[i];

                hConfig->groupIndexBits[i] = AVS3_MAX(1, (short)(floor((log(hConfig->groupChans[i] * (hConfig->groupChans[i] - 1) / 2 - 1) / log(2.f))) + 1));
            }

            /* BWE configures */
            MvShort2Short(cfg[bitrateIdx].groupBwe, hConfig->groupBwe, MAX_HOA_DMX_GROUPS);

            hConfig->maxDirecChanBitsRatio = cfg[bitrateIdx].maxBitsRatio;

            hConfig->nTotalForeChans = hConfig->groupChans[0];
            hConfig->nTotalResChans = hConfig->nTotalChansTransport - hConfig->groupChans[0];

            break;
        }
    }

    if (bitrateIdx >= HOA_SIZE_BITRATE_TABLE) {
		LOGD(" Unsupported bitrate!\n");
        exit(-1);
    }

    return;
}

static void GetFoaConfiguration(AVS3_HOA_CONFIG_DATA_HANDLE hConfig)
{
    short i;

    hConfig->nTotalChanGroups = 1;
    hConfig->nTotalChansTransport = 4;
    hConfig->spatialAnalysis = 0;
    hConfig->innerFormat = AVS3_INNER_FOA_FORMAT;
    hConfig->nTotalForeChans = 0;
    hConfig->nTotalResChans = 0;

    SetShort(hConfig->groupChans, 0, MAX_HOA_DMX_GROUPS);
    SetShort(hConfig->groupChOffset, 0, MAX_HOA_DMX_GROUPS + 1);

    if (hConfig->totalBitrate <= HOA_BITRATE_256K) {

        SetShort(hConfig->groupBwe, 1, hConfig->nTotalChanGroups);
    }
    else {
        SetShort(hConfig->groupBwe, 0, hConfig->nTotalChanGroups);
    }

    hConfig->groupChans[0] = hConfig->nTotalChansTransport;

    for (i = 0; i < hConfig->nTotalChanGroups; i++)
    {
        hConfig->groupIndexBits[i] = AVS3_MAX(1, (short)(floor((log(hConfig->groupChans[i] * (hConfig->groupChans[i] - 1) / 2 - 1) / log(2.f))) + 1));
        hConfig->groupCodeLines[i] = hConfig->frameLength;
    }

    return;
}

static void GetHoa2Configuration(AVS3_HOA_CONFIG_DATA_HANDLE hConfig)
{
    hConfig->innerFormat = AVS3_INNER_HOA2_FORMAT;

    GetGroupConfiguration(hConfig, avs3_hoa2_group_config);

    return;
}


static void GetHoa3Configuration(AVS3_HOA_CONFIG_DATA_HANDLE hConfig)
{
    hConfig->innerFormat = AVS3_INNER_HOA3_FORMAT;
    GetGroupConfiguration(hConfig, avs3_hoa3_group_config);
    return;
}


void HoaBitrateConfigTable(AVS3_HOA_CONFIG_DATA_HANDLE hConfig)
{
    switch (hConfig->order)
    {
    case 1:
        GetFoaConfiguration(hConfig);
        break;
    case 2:
        GetHoa2Configuration(hConfig);
        break;
    case 3:
        GetHoa3Configuration(hConfig);
        break;
    default:
        assert(!"Not support more than 4th HOA.\n");
        break;
    }

    return;
}

void Avs3HoaInitConfig(AVS3_HOA_CONFIG_DATA_HANDLE hConfig, const short numChansInput, const short lenFrame, const short bwidth, const long totalBitrate)
{
    /* HOA basic configuration. */
    hConfig->nTotalChansInput = numChansInput;
    hConfig->totalBitrate = totalBitrate;

    SetShort(hConfig->groupCodeLines, 0, MAX_HOA_DMX_GROUPS);
    SetZero(hConfig->groupBitsRatio, MAX_HOA_DMX_GROUPS);

    /* initial configure. */
    switch (hConfig->nTotalChansInput)
    {
    case 4: /* FOA. */
        hConfig->order = 1;
        break;
    case 9:
        hConfig->order = 2;
        break;
    case 16:
        hConfig->order = 3;
        break;
    default:
        assert(!"Not support more than 4 order HOA!\n");
        break;
    }

    hConfig->frameLength = lenFrame;
    HoaBitrateConfigTable(hConfig);

    hConfig->overlapSize = hConfig->frameLength / 2;
    GetMdctWindow(hConfig->hoaWindow, hConfig->overlapSize);

    return;
}

void GetNeighborBasisCoeff(const short anglePair[L_SECOND_ORDER_MP_BASIS][2], float basisCoeffs[L_SECOND_ORDER_MP_BASIS][MAX_HOA_CHANNELS])
{
    short i;

    short az, ele;

    float sinAz, cosAz, sinEle, cosEle;
    float vecBasis[L_HOA_BASIS_ROWS];

    for (i = 0; i < L_SECOND_ORDER_MP_BASIS; i++)
    {
        az = anglePair[i][0];
        ele = anglePair[i][1];

        sinAz = qsin(az);
        cosAz = qcos(az);

        sinEle = qsin(ele);
        cosEle = qcos(ele);

        Avs3Hoa3v2(sinAz, cosAz, sinEle, cosEle, vecBasis);

        Mvf2f(vecBasis, basisCoeffs[i], L_HOA_BASIS_ROWS);
    }

    return;
}

void GetSingleNeighborBasisCoeff(const short anglePair[2], float basisCoeffs[L_HOA_BASIS_ROWS])
{
    short az, ele;
    float sinAz, cosAz, sinEle, cosEle;
    float vecBasis[L_HOA_BASIS_ROWS];

    az = anglePair[0];
    ele = anglePair[1];

    sinAz = qsin(az);
    cosAz = qcos(az);

    sinEle = qsin(ele);
    cosEle = qcos(ele);

    Avs3Hoa3v2(sinAz, cosAz, sinEle, cosEle, vecBasis);

    Mvf2f(vecBasis, basisCoeffs, L_HOA_BASIS_ROWS);

    return;
}