//
//  equalizer.h
//  AudioPlayer
//
//  Created by develop on 2024/2/6.
//

#ifndef equalizer_h
#define equalizer_h

#include <stdio.h>
#include <math.h>
#include <stdint.h>

typedef struct
{
    uint16_t type;
    float fl;
}FILTERTYPE;  // 滤波器类型

typedef struct
{
    FILTERTYPE type;
    uint8_t sampleSize;
    uint8_t channels;
    uint16_t enabled_channel_bit;
    uint32_t freq; // 采样率
    float dbgain; // 增益
    float q;  // 品质因子
    float amp; // 幅值
    float a[3]; // a0,a1,a2
    float b[3]; // b0,b1,b2
    float x[3]; // x(n),x(n-1),x(n-2)
    float y[2]; // y(n-1),y(n-2)
}FILTERINFO, *LPFILTERINFO;

void Init_Filter(LPFILTERINFO pFilter);
uint32_t FilterAudioData(LPFILTERINFO pFilter, int8_t* inData, uint32_t inLen, int8_t* outData);
float Limiter(LPFILTERINFO pFilter, float inData);

#endif /* equalizer_h */
