//
//  FilterEngine.h
//  AudioPlayer
//
//  Created by develop on 2024/3/1.
//

#ifndef FilterEngine_h
#define FilterEngine_h

#include <stdint.h>

#define MAX_FILTER_SIZE 128

typedef enum
{
    Filter_Type_BYPASS = 0, // 不滤波
    Filter_Type_Gain,       // 音量调节
    Filter_Type_1ST_LP,     // 1阶低通
    Filter_Type_1ST_HP,     // 1阶高通
    Filter_Type_2ND_LP,     // 2阶低通
    Filter_Type_2ND_HP,     // 2阶高通
    Filter_Type_2ND_BP,     // 2阶带通
    Filter_Type_2ND_AP,     // 2阶全通
    Filter_Type_2ND_PEAK,   // 2阶峰值滤波
    Filter_Type_2ND_NOTCH,  // 2阶陷波滤波
    Filter_Type_2ND_LS,     // 2阶低切
    Filter_Type_2ND_HS,     // 2阶高切
    Filter_Type_Delay       // 延时，最多支持200ms
}Filter_Type;

typedef struct AudioParam
{
    int freq;           //  采样率
    uint16_t format;    //  数据格式
    uint16_t samples;   //  一块音频数据的采样点数
    uint8_t  channels;  //  声道数
}AudioParam;

typedef struct EqulizerParam
{
    Filter_Type type;                   // 滤波器类型
    uint32_t    centre_freq;            // 滤波中心频率
    float       dbgain;                 // 增益
    float       quality_factor;         // 品质因子
    uint16_t    enabled_channel_bit;    /*  需要滤波的声道位置，最多支持16声道，
                                            例如0b1010101011100011，
                                            从低位到高位依次表示第1、2、3...16位的声道是否需要滤波，
                                            值为1表示该声道需要滤波，值为0表示该声道不需要滤波，
                                            超过声道数的位不做处理，如声道数为6，则只需处理最低6位的值 */
}EqulizerParam;

void CreateFilterEngine(void** pEngine);
void DestroyFilterEngine(void* pEngine);
void StartFilterEngine(void* pEngine, AudioParam* aParam, const char* configFile);
void StopFilterEngine(void* pEngine);
void AddChannelDelays(void* pEngine, const float* channel_delays_ms);
void AddFilter(void* pEngine, EqulizerParam* eqParam);
void ResetFilter(void* pEngine);
 
int8_t* FilterAudio(void* pEngine, int8_t* inData, uint32_t inLen);

void StartDebug(void* pEngine);
void StopDebug(void* pEngine);


#endif /* FilterEngine_h */
