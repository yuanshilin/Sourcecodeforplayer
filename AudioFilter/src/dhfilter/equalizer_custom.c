//
//  equalizer.c
//  AudioPlayer
//
//  Created by develop on 2024/2/6.
//

#include "equalizer_custom.h"
#include "FilterEngine.h"
#include <math.h>

#define LIMITER_THRESHHOLD 0.99999f
#ifdef _ALIOS_
#define M_PI        3.14159265358979323846
#endif

MVoid Init_Filter(LPFILTERINFO pFilter)
{
    MDouble  omega = 2 * M_PI * pFilter->type.fl / pFilter->freq;
    MDouble  sine = sin(omega);
    MDouble  cosine = cos(omega);
    MDouble  alpha = sine / (2 * pFilter->q);
    MDouble  A2 = pow(10, pFilter->dbgain / 20);
    MDouble  A = sqrt(pow(10, pFilter->dbgain / 20));

    pFilter->b[0] = pFilter->b[1] = pFilter->b[2] = pFilter->a[0] = pFilter->a[1] = pFilter->a[2] = 0;

    switch (pFilter->type.type)
    {
        case Filter_Type_1ST_LP:
        case Filter_Type_1ST_HP:
            pFilter->b[0] = pFilter->type.fl * 2.0 * M_PI / pFilter->freq;
            pFilter->a[1] = 1 - pFilter->b[0];
            break;
        case Filter_Type_2ND_LP:
            pFilter->a[0] = 1 + alpha;
            pFilter->a[1] = (-2 * cosine) / pFilter->a[0];
            pFilter->a[2] = (1 - alpha) / pFilter->a[0];
            pFilter->b[0] = ((1 - cosine) / 2) / pFilter->a[0];
            pFilter->b[1] = (1 - cosine) / pFilter->a[0];
            pFilter->b[2] = ((1 - cosine) / 2) / pFilter->a[0];
            pFilter->a[0] = 1.0;
            break;
        case Filter_Type_2ND_HP:
            pFilter->a[0] = 1 + alpha;
            pFilter->a[1] = (-2 * cosine) / pFilter->a[0];
            pFilter->a[2] = (1 - alpha) / pFilter->a[0];
            pFilter->b[0] = ((1 + cosine) / 2) / pFilter->a[0];
            pFilter->b[1] = (-(1 + cosine)) / pFilter->a[0];
            pFilter->b[2] = ((1 + cosine) / 2) / pFilter->a[0];
            pFilter->a[0] = 1.0;
            break;
        case Filter_Type_2ND_BP:
            pFilter->a[0] = 1 + alpha;
            pFilter->a[1] = (-2 * cosine) / pFilter->a[0];
            pFilter->a[2] = (1 - alpha) / pFilter->a[0];
            pFilter->b[0] = (sine / 2) / pFilter->a[0];
            pFilter->b[1] = 0;
            pFilter->b[2] = (-sine / 2) / pFilter->a[0];
            pFilter->a[0] = 1.0;
            break;
        case Filter_Type_2ND_AP:
            pFilter->a[0] = 1 + alpha;
            pFilter->a[1] = (-2 * cosine) / pFilter->a[0];
            pFilter->a[2] = (1 - alpha) / pFilter->a[0];
            pFilter->b[0] = (1 - alpha) / pFilter->a[0];
            pFilter->b[1] = (-2 * cosine) / pFilter->a[0];
            pFilter->b[2] = 1.0;
            pFilter->a[0] = 1.0;
            break;
        case Filter_Type_2ND_PEAK:
            pFilter->a[0] = 1 + alpha / A;
            pFilter->a[1] = (-2 * cosine) / pFilter->a[0];
            pFilter->a[2] = (1 - alpha / A) / pFilter->a[0];
            pFilter->b[0] = (1 + alpha * A) / pFilter->a[0];
            pFilter->b[1] = (-2 * cosine) / pFilter->a[0];
            pFilter->b[2] = (1 - alpha * A) / pFilter->a[0];
            pFilter->a[0] = 1.0;
            break;
        case Filter_Type_2ND_NOTCH:
            pFilter->a[0] = 1 + alpha;
            pFilter->a[1] = (-2 * cosine) / pFilter->a[0];
            pFilter->a[2] = (1 - alpha) / pFilter->a[0];
            pFilter->b[0] = 1 / pFilter->a[0];
            pFilter->b[1] = (-2 * cosine) / pFilter->a[0];
            pFilter->b[2] = 1 / pFilter->a[0];
            pFilter->a[0] = 1.0;
            break;
        case Filter_Type_2ND_LS:
            pFilter->a[0] = (A + 1) + (A - 1) * cosine + 2 * sqrt(A) * alpha;
            pFilter->a[1] = (-2 * ((A - 1) + (A + 1) * cosine)) / pFilter->a[0];
            pFilter->a[2] = ((A + 1) + (A - 1) * cosine - 2 * sqrt(A) * alpha) / pFilter->a[0];
            pFilter->b[0] = A * ((A + 1) - (A - 1) * cosine + 2 * sqrt(A) * alpha) / pFilter->a[0];
            pFilter->b[1] = 2 * A * ((A - 1) - (A + 1) * cosine) / pFilter->a[0];
            pFilter->b[2] = A * ((A + 1) - (A - 1) * cosine - 2 * sqrt(A) * alpha) / pFilter->a[0];
            pFilter->a[0] = 1.0;
            break;
        case Filter_Type_2ND_HS:
            pFilter->a[0] = (A + 1) - (A - 1) * cosine + 2 * sqrt(A) * alpha;
            pFilter->a[1] = (2 * ((A - 1) - (A + 1) * cosine)) / pFilter->a[0];
            pFilter->a[2] = ((A + 1) - (A - 1) * cosine - 2 * sqrt(A) * alpha) / pFilter->a[0];
            pFilter->b[0] = A * ((A + 1) + (A - 1) * cosine + 2 * sqrt(A) * alpha) / pFilter->a[0];
            pFilter->b[1] = -2 * A * ((A - 1) + (A + 1) * cosine) / pFilter->a[0];
            pFilter->b[2] = A * ((A + 1) + (A - 1) * cosine - 2 * sqrt(A) * alpha) / pFilter->a[0];
            pFilter->a[0] = 1.0;
            break;
        case Filter_Type_Gain:
            pFilter->b[0] = A2;
            pFilter->a[0] = 1.0;
            break;
    }
    
    pFilter->x[0] = pFilter->x[1] = pFilter->x[2] = pFilter->y[0] = pFilter->y[1] = 0;
}

MFloat Filter(LPFILTERINFO pFilter, MFloat in)
{
    MFloat f = in;
    if (pFilter->type.type == Filter_Type_1ST_LP || pFilter->type.type == Filter_Type_1ST_HP)
    {
        f = pFilter->b[0] * in + pFilter->a[1] * pFilter->y[0];
        pFilter->y[0] = f;
    }
    else if (pFilter->type.type)
    {
        pFilter->x[2] = pFilter->x[1];
        pFilter->x[1] = pFilter->x[0];
        pFilter->x[0] = in;
        f = pFilter->b[0] * pFilter->x[0] + pFilter->b[1] * pFilter->x[1] + pFilter->b[2] * pFilter->x[2] - pFilter->a[1] * pFilter->y[0] - pFilter->a[2] * pFilter->y[1];
        pFilter->y[1] = pFilter->y[0];
        pFilter->y[0] = f;
    }
    
    f = Limiter(pFilter, f);
    return f;
}

MUInt32 FilterAudioData(LPFILTERINFO pFilter, MInt8* inData, MUInt32 inLen, MInt8* outData)
{
    MUInt32 ret = 0;
    if (pFilter == NULL || inData == NULL || outData == NULL)
        return 2;
    switch (pFilter->sampleSize) {
        case 16:
        {
            MInt16* src = (MInt16*)inData;
            MInt16* dst = (MInt16*)outData;
            MUInt32 srcLen = inLen * sizeof(MInt8) / sizeof(MInt16);
            MUInt32 blocks = srcLen / pFilter->channels;
            for (MUInt32 i = 0; i < blocks; i++) {
                for (MUInt32 j = 0; j < pFilter->channels; j++) {
                    MUInt16 b = pFilter->enabled_channel_bit & (1 << j);
                    if (b) {
                        MInt16 temp = src[i*pFilter->channels + j];
                        MFloat temp_f = (MFloat)temp / 32768.0f;
                        MFloat dst_f = Filter(pFilter, temp_f);
                        dst[i*pFilter->channels + j] = dst_f * 32768;
                    } else {
                        dst[i*pFilter->channels + j] = src[i*pFilter->channels + j];
                    }

                }
            }
        }
            break;
        case 32:
        {
            MInt32* src = (MInt32*)inData;
            MInt32* dst = (MInt32*)outData;
            MUInt32 srcLen = inLen * sizeof(MInt8) / sizeof(MInt32);
            MUInt32 blocks = srcLen / pFilter->channels;
            for (MUInt32 i = 0; i < blocks; i++) {
                for (MUInt32 j = 0; j < pFilter->channels; j++) {
                    MUInt16 b = pFilter->enabled_channel_bit & (1 << j);
                    if (b) {
                        MInt32 temp = src[i*pFilter->channels + j];
                        MFloat temp_f = (MFloat)temp / 4294967295.0f;
                        MFloat dst_f = Filter(pFilter, temp_f);
                        dst[i*pFilter->channels + j] = dst_f * 4294967295;
                    } else {
                        dst[i*pFilter->channels + j] = src[i*pFilter->channels + j];
                    }

                }
            }
        }
            break;
        default:
            ret = 4;
            break;
    }
    return ret;
}

MFloat Limiter(LPFILTERINFO pFilter, MFloat inData)
{
    if (pFilter == NULL)
        return inData;
    MFloat out = inData;
    if (out > LIMITER_THRESHHOLD) {
        out = LIMITER_THRESHHOLD;
    } else if (out < -LIMITER_THRESHHOLD) {
        out = -LIMITER_THRESHHOLD;
    }
    return out;
}
