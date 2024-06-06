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

void Init_Filter(LPFILTERINFO pFilter)
{
    double  omega = 2 * M_PI * pFilter->type.fl / pFilter->freq;
    double  sine = sin(omega);
    double  cosine = cos(omega);
    double  alpha = sine / (2 * pFilter->q);
    double  A2 = pow(10, pFilter->dbgain / 20);
    double  A = sqrt(pow(10, pFilter->dbgain / 20));

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

float Filter(LPFILTERINFO pFilter, float in)
{
    float f = in;
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

uint32_t FilterAudioData(LPFILTERINFO pFilter, int8_t* inData, uint32_t inLen, int8_t* outData)
{
    uint32_t ret = 0;
    if (pFilter == NULL || inData == NULL || outData == NULL)
        return 2;
    switch (pFilter->sampleSize) {
        case 16:
        {
            int16_t* src = (int16_t*)inData;
            int16_t* dst = (int16_t*)outData;
            uint32_t srcLen = inLen * sizeof(int8_t) / sizeof(int16_t);
            uint32_t blocks = srcLen / pFilter->channels;
            for (int i = 0; i < blocks; i++) {
                for (int j = 0; j < pFilter->channels; j++) {
                    uint16_t b = pFilter->enabled_channel_bit & (1 << j);
                    if (b) {
                        int16_t temp = src[i*pFilter->channels + j];
                        float temp_f = (float)temp / 32768.0f;
                        float dst_f = Filter(pFilter, temp_f);
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
            int32_t* src = (int32_t*)inData;
            int32_t* dst = (int32_t*)outData;
            uint32_t srcLen = inLen * sizeof(int8_t) / sizeof(int32_t);
            uint32_t blocks = srcLen / pFilter->channels;
            for (int i = 0; i < blocks; i++) {
                for (int j = 0; j < pFilter->channels; j++) {
                    uint16_t b = pFilter->enabled_channel_bit & (1 << j);
                    if (b) {
                        int32_t temp = src[i*pFilter->channels + j];
                        float temp_f = (float)temp / 4294967295.0f;
                        float dst_f = Filter(pFilter, temp_f);
                        dst[i*pFilter->channels + j] = dst_f * 4294967295;
                    } else {
                        dst[i*pFilter->channels + j] = src[i*pFilter->channels + j];
                    }

                }
            }
        }
        default:
            ret = 4;
            break;
    }
    return ret;
}

float Limiter(LPFILTERINFO pFilter, float inData)
{
    float out = inData;
    if (out > LIMITER_THRESHHOLD) {
        out = LIMITER_THRESHHOLD;
    } else if (out < -LIMITER_THRESHHOLD) {
        out = -LIMITER_THRESHHOLD;
    }
    return out;
}
