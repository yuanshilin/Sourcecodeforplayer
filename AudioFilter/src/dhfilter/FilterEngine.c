//
//  FilterEngine.c
//  AudioPlayer
//
//  Created by develop on 2024/3/1.
//

#include "FilterEngine.h"
#include "FilterLog.h"
#include "cJSON.h"
#include "equalizer_custom.h"
#include "delay_processor.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#define PORT 37686
#define TRUE    1
#define FALSE   0

#define RECV_BUFFER_SIZE 10*1024

typedef struct FilterEngine
{
    AudioParam  aParam;
    void*       pFilters[MAX_FILTER_SIZE];
    void*       tempbuffer;
    void*       pDelayProcessor;
    short*      pDelayOut;
    int         filterCount;
    pthread_mutex_t* pFilterMutex;
    
    int         bDebug;
    int         socketFD;
    
}FilterEngine, *LPFilterEngine;

static int HandleReceivedBuffer(FilterEngine* fEngine, char* buffer)
{
    cJSON* root = cJSON_Parse(buffer);
    if (!root)
    {
        return -1;
    }
    cJSON* filters = cJSON_GetObjectItem(root, "filters");
    if (!filters)
    {
        cJSON_Delete(root);
        return -1;
    }
    ResetFilter(fEngine);
    int arraySize = cJSON_GetArraySize(filters);
    for (int i = 0; i < arraySize; i++)
    {
        cJSON* item = cJSON_GetArrayItem(filters, i);
        if (!item)
        {
            continue;
        }
        cJSON* type = cJSON_GetObjectItem(item, "type");
        cJSON* freq = cJSON_GetObjectItem(item, "freq");
        cJSON* qFactor = cJSON_GetObjectItem(item, "Qfactor");
        if (!type) {
            continue;
        }
        if (type->valueint == Filter_Type_Delay) {
            int channels = fEngine->aParam.channels;
            float *channel_delays_ms = (float*)calloc(channels, sizeof(float));
            char key[10] = "";
            for (int i = 0; i < channels; i++) {
                sprintf(key, "channel%d", i+1);
                cJSON *channel = cJSON_GetObjectItem(item, key);
                if (channel) {
                    channel_delays_ms[i] = channel->valuedouble;
                } else {
                    channel_delays_ms[i] = 0;
                }
            }
            AddChannelDelays(fEngine, channel_delays_ms);
            continue;
        }
        if (type->valueint != Filter_Type_Gain && (!freq || !qFactor))
        {
            continue;
        }
        cJSON* gain = cJSON_GetObjectItem(item, "gain");
        cJSON* channels = cJSON_GetObjectItem(item, "channels");
        EqulizerParam param = {0};
        param.type = (Filter_Type)type->valueint;
        if (freq)
            param.centre_freq = freq->valueint;
        if (qFactor)
            param.quality_factor = qFactor->valuedouble;
        param.enabled_channel_bit = 0xFFFF;
        if (gain)
        {
            param.dbgain = gain->valuedouble;
        }
        if (channels)
        {
            param.enabled_channel_bit = channels->valueint;
        }
        LOGI("HandleReceivedBuffer type: %d, freq: %d, Qfactor: %f, channels: 0x%x, gain: %f\r\n", param.type, param.centre_freq, param.quality_factor, param.enabled_channel_bit, param.dbgain);

        AddFilter(fEngine, &param);
         
    }
    
    cJSON_Delete(root);
    return 0;
}
static void* ThreadRecvProcess(void* pData)
{
    FilterEngine* fEngine = (FilterEngine*)pData;

    fEngine->bDebug = TRUE;

    struct sockaddr_in address_client;
    struct timeval timeout;
    int addr_client_len = sizeof(address_client);
    int new_socket = 0;
    fd_set readfds;
    int max_sd, activity;
    LOGI("socket fd: %d\r\n", fEngine->socketFD);
    while (fEngine->bDebug)
    {
        LOGD("\n+++++++ Waiting for new connection ++++++++\n\n");
        // 清空读文件描述符集合
        FD_ZERO(&readfds);

        // 将服务器套接字加入集合
        FD_SET(fEngine->socketFD, &readfds);

        max_sd = fEngine->socketFD;
        // 使用select等待活动
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0) {
            LOGD("select activity: %d\r\n", activity);
            break;
        }
        if (FD_ISSET(fEngine->socketFD, &readfds))
        {
            new_socket = accept(fEngine->socketFD, (struct sockaddr *)&address_client, (socklen_t*)&addr_client_len);
            LOGI("new socket fd: %d\r\n", new_socket);
            if (new_socket < 0)
            {
                LOGE("In accept error, %d\r\n", errno);
                break;
            }

            uint8_t buffer[RECV_BUFFER_SIZE] = {0};
            uint8_t *temp = NULL;
            int targetLength = 0;
            int stringLength = 0;
            int bAppend = 0;
            do {
                memset(buffer, 0, RECV_BUFFER_SIZE);
                long valread = read( new_socket , buffer, RECV_BUFFER_SIZE);
                LOGD("valread: %ld, fEngine->bDebug: %d\n", valread, fEngine->bDebug);
                if (valread > 0) {
                    if (buffer[0] == 0x24 && bAppend == 0) {
                        targetLength = (buffer[2] << 8) + buffer[3];
                        LOGD("target length: %d, 0x%x, 0x%x\n", targetLength, buffer[2], buffer[3]);
                        if (valread == targetLength + 4) {
                            if (fEngine->bDebug) {
                                int ret = HandleReceivedBuffer(fEngine, (char*)buffer + 4);
                                LOGI("HandleReceivedBuffer ret: %d\n", ret);
                            }
                            break;
                        } else if (valread < targetLength + 4) {
                            temp = malloc(targetLength+1);
                            memset(temp, 0, targetLength+1);
                            memcpy(temp, buffer+4, valread-4);
                            stringLength += valread - 4;
                            bAppend = 1;
                        }
                    } else if (bAppend == 1) {
                        memcpy(temp + stringLength, buffer, valread);
                        stringLength += valread;
                        if (stringLength == targetLength) {
                            if (fEngine->bDebug) {
                                int ret = HandleReceivedBuffer(fEngine, (char*)temp);
                                LOGI("HandleReceivedBuffer ret: %d\n", ret);
                            }
                            break;
                        } else if (stringLength > targetLength) {
                            break;
                        } else {
                            continue;
                        }
                    }
                } else
                    break;
            } while (1);
            if (temp) {
                free(temp);
                temp = NULL;
            }
            close(new_socket);
        } 
    }
    LOGI("thread end\r\n");
    return 0;
}

static LPFILTERINFO InitFilter(AudioParam *aParam, EqulizerParam* param)
{
    LPFILTERINFO filter = (LPFILTERINFO)malloc(sizeof(FILTERINFO));
    memset(filter, 0, sizeof(FILTERINFO));
    // 信号的增益不予改变，所以不去计算dbgain
    filter->type.type = param->type; // 二阶带通滤波器
    filter->freq = aParam->freq;
    filter->type.fl = param->centre_freq; // 中心频率为1Hz
    filter->q = param->quality_factor; // 品质因子为1，对中心频率信号滤波效果最佳
    filter->dbgain = param->dbgain;
    filter->sampleSize = aParam->format & 0xFF;
    filter->channels = aParam->channels;
    filter->enabled_channel_bit = param->enabled_channel_bit;
    Init_Filter(filter);
    return filter;
}

static void ReleaseDelayProcessor(LPFilterEngine fEngine)
{
    if (fEngine == NULL) {
        return;
    }
    if (fEngine->pDelayProcessor != NULL) {
        destroy_delay_processor(fEngine->pDelayProcessor);
        fEngine->pDelayProcessor = NULL;
    }
    if (fEngine->pDelayOut != NULL) {
        free(fEngine->pDelayOut);
        fEngine->pDelayOut = NULL;
    }
}

void ReadFromJsonFile(LPFilterEngine fEngine, const char* filepath)
{
    FILE* fp = fopen(filepath, "r");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long length = ftell(fp);
        char* buffer = malloc(length + 1);
        memset(buffer, 0, length + 1);
        
        fseek(fp, 0, SEEK_SET);
        size_t read = fread(buffer, 1, length, fp);
        if (read == length) {
            HandleReceivedBuffer(fEngine, buffer);
        }
        
        fclose(fp);
    }
}

void CreateFilterEngine(void** pEngine)
{
    LPFilterEngine engine = (LPFilterEngine)malloc(sizeof(FilterEngine));
    memset(engine, 0, sizeof(FilterEngine));
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
#ifndef _ALIOS_
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#else
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#endif
    engine->pFilterMutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(engine->pFilterMutex, &attr);
    pthread_mutexattr_destroy(&attr);

    *pEngine = engine;
}

void DestroyFilterEngine(void* pEngine)
{
    if (pEngine) {
        LPFilterEngine fEngine = (LPFilterEngine)pEngine;

        pthread_mutex_destroy(fEngine->pFilterMutex);
        free(fEngine->pFilterMutex);
        free(fEngine);
    }
}

void StartFilterEngine(void* pEngine, AudioParam* aParam, const char* configFile)
{
    if (pEngine == NULL) {
        return;
    }
    LOGI("StartFilterEngine %d, %d, %d\r\n", aParam->freq, aParam->channels, aParam->samples);
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    memcpy(&fEngine->aParam, aParam, sizeof(AudioParam));
    
    int tempLen = aParam->samples * aParam->channels * 10;
    if (fEngine->tempbuffer == NULL)
        fEngine->tempbuffer = malloc(tempLen);
    memset(fEngine->tempbuffer, 0, tempLen);
    
    if (configFile != NULL) {
        LOGI("config: %s\r\n", configFile);
        ReadFromJsonFile(fEngine, configFile);
    }
}

void StopFilterEngine(void* pEngine)
{
    if (pEngine == NULL) {
        return;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    LOGI("StopFilterEngine In bDebug: %d\r\n", fEngine->bDebug);

    if (fEngine->bDebug == TRUE) {
        StopDebug(fEngine);
    }
    
    ReleaseDelayProcessor(fEngine);
    
    ResetFilter(fEngine);
    if (fEngine->tempbuffer) {
        free(fEngine->tempbuffer);
        fEngine->tempbuffer = NULL;
    }
}

void AddChannelDelays(void* pEngine, const float* channel_delays_ms)
{
    if (pEngine == NULL) {
        return;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    
    ReleaseDelayProcessor(fEngine);
    
    fEngine->pDelayProcessor = create_delay_processor();
    int ret = initialize_delay_processor(fEngine->pDelayProcessor, fEngine->aParam.freq, fEngine->aParam.channels, channel_delays_ms);
    LOGI("AddChannelDelays init delay processor freq: %d, channels: %d, delay: %f, ret: %d\r\n", fEngine->aParam.freq, fEngine->aParam.channels, channel_delays_ms[0], ret);
    if (ret != 0) {
        LOGE("Failed to initialize delay processor. ret: %d\n", ret);
        destroy_delay_processor(fEngine->pDelayProcessor);
        fEngine->pDelayProcessor = NULL;
    } else {
        int delayOutSize = fEngine->aParam.channels * fEngine->aParam.samples;
        fEngine->pDelayOut = (short*)calloc(delayOutSize, sizeof(short));
        memset(fEngine->pDelayOut, 0, delayOutSize * sizeof(short));
    }
}

void AddFilter(void* pEngine, EqulizerParam* eqParam)
{
    if (pEngine == NULL) {
        return;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    
    pthread_mutex_lock(fEngine->pFilterMutex);
    LPFILTERINFO filter = InitFilter(&fEngine->aParam, eqParam);
    fEngine->pFilters[fEngine->filterCount++] = filter;
    LOGD("AddFilter type: %d, count: %d\r\n", filter->type.type, fEngine->filterCount);
    pthread_mutex_unlock(fEngine->pFilterMutex);
}

void ResetFilter(void* pEngine)
{
    if (pEngine == NULL) {
        return;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;

    pthread_mutex_lock(fEngine->pFilterMutex);
    for (int i = 0; i < fEngine->filterCount; i++) {
        LPFILTERINFO filter = fEngine->pFilters[i];
        free(filter);
        fEngine->pFilters[i] = NULL;
    }
    fEngine->filterCount = 0;
    pthread_mutex_unlock(fEngine->pFilterMutex);
}

int8_t* FilterAudio(void* pEngine, int8_t* inData, uint32_t inLen)
{
    if (pEngine == NULL) {
        return inData;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;

    int8_t* outData = NULL;
    if (fEngine->filterCount > 0) {
        pthread_mutex_lock(fEngine->pFilterMutex);
        int8_t* pIn = inData;
        for (int i = 0; i < fEngine->filterCount; i++) {
            LPFILTERINFO pFilter = fEngine->pFilters[i];
            int ret = FilterAudioData(pFilter, pIn, inLen, fEngine->tempbuffer);
            if (ret != 0) {
                LOGE("FilterAudioData ret: %d\r\n", ret);
            }
            pIn = fEngine->tempbuffer;
        }
        outData = fEngine->tempbuffer;
        pthread_mutex_unlock(fEngine->pFilterMutex);
    } else {
        outData = inData;
    }
    if (fEngine->pDelayProcessor) {
        short input_data[fEngine->aParam.channels][fEngine->aParam.samples];
        short output_data[fEngine->aParam.channels][fEngine->aParam.samples];
        short *inbuf = (short*)outData;
        for (int ch = 0;ch < fEngine->aParam.channels; ch++) {
            for (int i = 0; i < fEngine->aParam.samples; i++) {
                input_data[ch][i] = inbuf[fEngine->aParam.channels *i + ch];
            }
        }

        int ret = process_audio(fEngine->pDelayProcessor, input_data, output_data, fEngine->aParam.samples);
        if (ret != 0) {
            LOGE("Failed to process audio. ret: %d\r\n", ret);
            ReleaseDelayProcessor(fEngine);
        } else {
            for (int ch = 0;ch < fEngine->aParam.channels; ch++) {
                for (int i = 0; i < fEngine->aParam.samples; i++) {
                    fEngine->pDelayOut[fEngine->aParam.channels *i + ch] = output_data[ch][i];
                }
            }
            outData = (int8_t*)fEngine->pDelayOut;
        }
    }
    return outData;
}

void StartDebug(void* pEngine)
{
    if (pEngine == NULL) {
        return;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons( PORT );

    fEngine->socketFD = socket(AF_INET, SOCK_STREAM, 0);
    LOGI("StartDebug socket fd : %d\r\n", fEngine->socketFD);
    if (fEngine->socketFD == 0) {
        LOGE("StartDebug() create socket error, %d(%s)\n", errno, strerror(errno));
        return;
    }
    int reuse = 1;
    setsockopt(fEngine->socketFD, SOL_SOCKET, SO_REUSEPORT, (const void *)&reuse, sizeof(int));
    setsockopt(fEngine->socketFD, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(int));

    if (bind(fEngine->socketFD, (struct sockaddr *)&address, sizeof(address))<0)
    {
        LOGE("StartDebug() bind socket error, %d(%s)\n", errno, strerror(errno));
        close(fEngine->socketFD);
        return;
    }
    if (listen(fEngine->socketFD, 10) < 0)
    {
        LOGE("StartDebug() listen socket error, %d(%s)\n", errno, strerror(errno));
        close(fEngine->socketFD);
        return;
    }
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, ThreadRecvProcess, fEngine);
    if (ret == 0) {
        pthread_detach(thread);
    }

}

void StopDebug(void* pEngine)
{
    if (pEngine == NULL) {
        return;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    LOGI("StopDebug In bDebug: %d, fEngine->socketFD: %d\r\n", fEngine->bDebug, fEngine->socketFD);

    fEngine->bDebug = FALSE;
    if (fEngine->socketFD > 0) {
        close(fEngine->socketFD);
    }
}

