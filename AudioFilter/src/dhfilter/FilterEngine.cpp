//
//  FilterEngine.c
//  AudioPlayer
//
//  Created by develop on 2024/3/1.
//

#include "FilterEngine.h"

#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include <map>
#include <string>

#include "FilterLog.h"
#include "cJSON.h"
#include "delay_processor.h"
#include "equalizer_custom.h"

#define PORT 37686
#define TRUE 1
#define FALSE 0

#define RECV_BUFFER_SIZE 10 * 1024
typedef std::map<std::string, LPFILTERINFO> FilterMap;

typedef struct FilterEngine {
    AudioParam aParam;
    FilterMap* pFilters;
    MInt8* tempbuffer;
    MVoid* pDelayProcessor;
    MInt16* pDelayOut;
    
    pthread_mutex_t* pFilterMutex;
    MPChar configFilePath;
    
    MInt32 bDebug;
    MInt32 socketFD;
    
} FilterEngine, *LPFilterEngine;

static MInt32 HandleReceivedBuffer(FilterEngine* fEngine, MInt8* buffer) {
    cJSON* root = cJSON_Parse((MPCChar)buffer);
    if (!root) {
        return -1;
    }
    cJSON* filters = cJSON_GetObjectItem(root, "filters");
    if (!filters) {
        cJSON_Delete(root);
        return -1;
    }
    
    MInt32 arraySize = cJSON_GetArraySize(filters);
    for (MInt32 i = 0; i < arraySize; i++) {
        cJSON* item = cJSON_GetArrayItem(filters, i);
        if (!item) {
            continue;
        }
        cJSON* type = cJSON_GetObjectItem(item, "type");
        cJSON* freq = cJSON_GetObjectItem(item, "freq");
        cJSON* qFactor = cJSON_GetObjectItem(item, "Qfactor");
        if (!type) {
            continue;
        }
        if (type->valueint == Filter_Type_Delay) {
            MInt32 channels = fEngine->aParam.channels;
            MFloat* channel_delays_ms = (MFloat*)calloc(channels, sizeof(MFloat));
            MInt8 key[10] = "";
            for (MInt32 i = 0; i < channels; i++) {
                snprintf((MPChar)key, 10, "channel%d", i + 1);
                cJSON* channel = cJSON_GetObjectItem(item, (const char* const)key);
                if (channel) {
                    channel_delays_ms[i] = channel->valuedouble;
                } else {
                    channel_delays_ms[i] = 0;
                }
            }
            AddChannelDelays(fEngine, channel_delays_ms);
            free(channel_delays_ms);
            continue;
        }
        if (type->valueint != Filter_Type_Gain && (!freq || !qFactor)) {
            continue;
        }
        cJSON* gain = cJSON_GetObjectItem(item, "gain");
        cJSON* channels = cJSON_GetObjectItem(item, "channels");
        EqulizerParam param;
        memset(&param, 0, sizeof(EqulizerParam));
        param.type = (Filter_Type)type->valueint;
        if (freq) param.centre_freq = freq->valueint;
        if (qFactor) param.quality_factor = qFactor->valuedouble;
        param.enabled_channel_bit = 0xFFFF;
        if (gain) {
            param.dbgain = gain->valuedouble;
        }
        if (channels) {
            param.enabled_channel_bit = channels->valueint;
        }
        LOGI(
             "HandleReceivedBuffer type: %d, freq: %d, Qfactor: %f, channels: 0x%x, "
             "gain: %f\r\n",
             param.type, param.centre_freq, param.quality_factor,
             param.enabled_channel_bit, param.dbgain);
        
        AddFilter(fEngine, &param);
    }
    
    cJSON_Delete(root);
    return 0;
}
static MVoid* ThreadRecvProcess(MVoid* pData) {
    FilterEngine* fEngine = (FilterEngine*)pData;
    
    fEngine->bDebug = TRUE;
    
    struct sockaddr_in address_client;
    struct timeval timeout;
    MInt32 addr_client_len = sizeof(address_client);
    MInt32 new_socket = 0;
    fd_set readfds;
    MInt32 max_sd, activity;
    LOGI("socket fd: %d\r\n", fEngine->socketFD);
    while (fEngine->bDebug) {
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
        if (FD_ISSET(fEngine->socketFD, &readfds)) {
            new_socket = accept(fEngine->socketFD, (struct sockaddr*)&address_client,
                                (socklen_t*)&addr_client_len);
            LOGI("new socket fd: %d\r\n", new_socket);
            if (new_socket < 0) {
                LOGE("In accept error, %d\r\n", errno);
                break;
            }
            
            MUInt8 buffer[RECV_BUFFER_SIZE] = {0};
            MUInt8* temp = NULL;
            MInt32 targetLength = 0;
            MInt32 stringLength = 0;
            MInt32 bAppend = 0;
            do {
                memset(buffer, 0, RECV_BUFFER_SIZE);
                MInt64 valread = read(new_socket, buffer, RECV_BUFFER_SIZE);
                LOGD("valread: %lld, fEngine->bDebug: %d\n", (long long)valread,
                     fEngine->bDebug);
                if (valread > 0) {
                    if (buffer[0] == 0x24 && bAppend == 0) {
                        targetLength = (buffer[2] << 8) + buffer[3];
                        LOGD("target length: %d, 0x%x, 0x%x\n", targetLength, buffer[2],
                             buffer[3]);
                        if (valread == targetLength + 4) {
                            if (fEngine->bDebug) {
                                ResetFilter(fEngine);
                                MInt32 ret = HandleReceivedBuffer(fEngine, (MInt8*)buffer + 4);
                                LOGI("HandleReceivedBuffer ret: %d\n", ret);
                            }
                            break;
                        } else if (valread < targetLength + 4) {
                            temp = (MUInt8*)malloc(targetLength + 1);
                            memset(temp, 0, targetLength + 1);
                            memcpy(temp, buffer + 4, valread - 4);
                            stringLength += valread - 4;
                            bAppend = 1;
                        }
                    } else if (bAppend == 1) {
                        memcpy(temp + stringLength, buffer, valread);
                        stringLength += valread;
                        if (stringLength == targetLength) {
                            if (fEngine->bDebug) {
                                ResetFilter(fEngine);
                                MInt32 ret = HandleReceivedBuffer(fEngine, (MInt8*)temp);
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
    if (fEngine->socketFD > 0) {
        LOGI("close socket: %d\r\n", fEngine->socketFD);
        close(fEngine->socketFD);
        fEngine->socketFD = 0;
    }
    return 0;
}

static LPFILTERINFO InitFilter(AudioParam* aParam, EqulizerParam* param) {
    LPFILTERINFO filter = (LPFILTERINFO)malloc(sizeof(FILTERINFO));
    memset(filter, 0, sizeof(FILTERINFO));
    // 信号的增益不予改变，所以不去计算dbgain
    filter->type.type = param->type;  // 二阶带通滤波器
    filter->freq = aParam->freq;
    filter->type.fl = param->centre_freq;  // 中心频率为1Hz
    filter->q = param->quality_factor;  // 品质因子为1，对中心频率信号滤波效果最佳
    filter->dbgain = param->dbgain;
    filter->sampleSize = aParam->bitDepth & 0xFF;
    filter->channels = aParam->channels;
    filter->enabled_channel_bit = param->enabled_channel_bit;
    Init_Filter(filter);
    return filter;
}

static MVoid UpdateFilter(LPFILTERINFO filter, EqulizerParam* param) {
    filter->type.type = param->type;       // 二阶带通滤波器
    filter->type.fl = param->centre_freq;  // 中心频率为1Hz
    filter->q = param->quality_factor;  // 品质因子为1，对中心频率信号滤波效果最佳
    filter->dbgain = param->dbgain;
    filter->enabled_channel_bit = param->enabled_channel_bit;
    Init_Filter(filter);
}

static MVoid ReleaseDelayProcessor(LPFilterEngine fEngine) {
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

MVoid ReadFromJsonFile(LPFilterEngine fEngine, MPCChar filepath) {
    FILE* fp = fopen(filepath, "r");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        MInt64 length = ftell(fp);
        MInt8* buffer = (MInt8*)malloc(length + 1);
        memset(buffer, 0, length + 1);
        
        fseek(fp, 0, SEEK_SET);
        MInt64 read = fread(buffer, 1, length, fp);
        LOGI("ReadFromJsonFile length: %lld\r\n", read);
        if (read == length) {
            HandleReceivedBuffer(fEngine, buffer);
        }
        free(buffer);
        fclose(fp);
    }
}

MVoid CreateFilterEngine(MVoid** pEngine) {
    LPFilterEngine engine = (LPFilterEngine)malloc(sizeof(FilterEngine));
    memset(engine, 0, sizeof(FilterEngine));
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
#ifndef _ALIOS_
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#else
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#endif
    engine->pFilterMutex = new pthread_mutex_t;
    pthread_mutex_init(engine->pFilterMutex, &attr);
    pthread_mutexattr_destroy(&attr);
    
    engine->pFilters = new FilterMap();
    
    *pEngine = engine;
}

MVoid DestroyFilterEngine(MVoid* pEngine) {
    if (pEngine) {
        LPFilterEngine fEngine = (LPFilterEngine)pEngine;
        
        if (fEngine->pFilters != nullptr) {
            delete fEngine->pFilters;
            fEngine->pFilters = nullptr;
        }
        if (fEngine->pFilterMutex != nullptr) {
            pthread_mutex_destroy(fEngine->pFilterMutex);
            delete fEngine->pFilterMutex;
            fEngine->pFilterMutex = nullptr;
        }
        free(fEngine);
        fEngine = nullptr;
    }
}

MVoid StartFilterEngine(MVoid* pEngine, AudioParam* aParam,
                        MPCChar configFile) {
    if (pEngine == NULL) {
        return;
    }
    LOGI("StartFilterEngine %d, %d, %d\r\n", aParam->freq, aParam->channels,
         aParam->samples);
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    memcpy(&fEngine->aParam, aParam, sizeof(AudioParam));
    
    MInt32 tempLen = aParam->samples * aParam->channels * 10;
    if (fEngine->tempbuffer == NULL)
        fEngine->tempbuffer = (MInt8*)malloc(tempLen);
    memset(fEngine->tempbuffer, 0, tempLen);
    
    if (configFile != NULL) {
        MUInt64 len = strlen(configFile) + 1;
        fEngine->configFilePath = (MPChar)malloc(len);
        memset(fEngine->configFilePath, 0, len);
        strcpy(fEngine->configFilePath, configFile);
        LOGI("config: %s\r\n", configFile);
        ReadFromJsonFile(fEngine, configFile);
    }
}

MVoid StopFilterEngine(MVoid* pEngine) {
    if (pEngine == NULL) {
        return;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    LOGI("StopFilterEngine In bDebug: %d\r\n", fEngine->bDebug);
    
    if (fEngine->bDebug == TRUE) {
        StopDebug(fEngine);
    }
    
    ReleaseDelayProcessor(fEngine);
    
    if (fEngine->configFilePath) {
        free(fEngine->configFilePath);
        fEngine->configFilePath = NULL;
    }
    ResetFilter(fEngine);
    if (fEngine->tempbuffer) {
        free(fEngine->tempbuffer);
        fEngine->tempbuffer = NULL;
    }
}

MVoid AddChannelDelays(MVoid* pEngine, const MFloat* channel_delays_ms) {
    if (pEngine == NULL) {
        return;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    
    if (fEngine->aParam.samples != FRAME_LENGTH) {
        LOGE("samples[%d] don't support delay\r\n", fEngine->aParam.samples);
        return;
    }
    
    ReleaseDelayProcessor(fEngine);
    
    fEngine->pDelayProcessor = create_delay_processor();
    MInt32 ret =
    initialize_delay_processor(fEngine->pDelayProcessor, fEngine->aParam.freq,
                               fEngine->aParam.channels, channel_delays_ms);
    LOGI(
         "AddChannelDelays init delay processor freq: %d, channels: %d, delay: "
         "%f, ret: %d\r\n",
         fEngine->aParam.freq, fEngine->aParam.channels, channel_delays_ms[0],
         ret);
    if (ret != 0) {
        LOGE("Failed to initialize delay processor. ret: %d\n", ret);
        destroy_delay_processor(fEngine->pDelayProcessor);
        fEngine->pDelayProcessor = NULL;
    } else {
        MInt32 delayOutSize = fEngine->aParam.channels * fEngine->aParam.samples;
        fEngine->pDelayOut = (MInt16*)calloc(delayOutSize, sizeof(MInt16));
        memset(fEngine->pDelayOut, 0, delayOutSize * sizeof(MInt16));
    }
}

MVoid AddFilter(MVoid* pEngine, EqulizerParam* eqParam) {
    if (pEngine == NULL) {
        return;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    
    pthread_mutex_lock(fEngine->pFilterMutex);
    char key[20];
    memset(key, 0, 20);
    snprintf(key, 20, "%d/%d/%d", eqParam->type, eqParam->centre_freq,
             eqParam->enabled_channel_bit);
    std::string keyString(key);
    auto it = fEngine->pFilters->find(keyString);
    if (it == fEngine->pFilters->end()) {
        LPFILTERINFO filter = InitFilter(&fEngine->aParam, eqParam);
        fEngine->pFilters->insert(std::make_pair(keyString, filter));
    } else {
        LPFILTERINFO filter = it->second;
        UpdateFilter(filter, eqParam);
    }
    
    LOGD("AddFilter type: %d, count: %zu\r\n", eqParam->type,
         fEngine->pFilters->size());
    pthread_mutex_unlock(fEngine->pFilterMutex);
}

MVoid ResetFilter(MVoid* pEngine) {
    if (pEngine == NULL) {
        return;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    
    if (fEngine->pFilters->size() > 0) {
        pthread_mutex_lock(fEngine->pFilterMutex);
        for (auto it = fEngine->pFilters->begin(); it != fEngine->pFilters->end();
             it++) {
            LPFILTERINFO filter = it->second;
            free(filter);
        }
        fEngine->pFilters->clear();
        pthread_mutex_unlock(fEngine->pFilterMutex);
    }
    
    if (fEngine->configFilePath != NULL) {
        ReadFromJsonFile(fEngine, fEngine->configFilePath);
    }
}

MInt8* FilterAudio(MVoid* pEngine, MInt8* inData, MUInt32 inLen) {
    if (pEngine == NULL) {
        return inData;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    
    MInt8* outData = NULL;
    if (fEngine->pFilters->size() > 0) {
        pthread_mutex_lock(fEngine->pFilterMutex);
        MInt8* pIn = inData;
        for (auto it : *fEngine->pFilters) {
            LPFILTERINFO pFilter = it.second;
            MUInt32 ret = FilterAudioData(pFilter, pIn, inLen, fEngine->tempbuffer);
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
        MInt16 input_data[fEngine->aParam.channels][FRAME_LENGTH];
        MInt16 output_data[fEngine->aParam.channels][FRAME_LENGTH];
        MInt16* inbuf = (MInt16*)outData;
        for (MInt32 ch = 0; ch < fEngine->aParam.channels; ch++) {
            for (MInt32 i = 0; i < fEngine->aParam.samples; i++) {
                input_data[ch][i] = inbuf[fEngine->aParam.channels * i + ch];
            }
        }
        
        MInt32 ret = process_audio(
                                   fEngine->pDelayProcessor, (MInt16(*)[FRAME_LENGTH])input_data,
                                   (MInt16(*)[FRAME_LENGTH])output_data, fEngine->aParam.samples);
        if (ret != 0) {
            LOGE("Failed to process audio. ret: %d\r\n", ret);
            ReleaseDelayProcessor(fEngine);
        } else {
            for (MInt32 ch = 0; ch < fEngine->aParam.channels; ch++) {
                for (MInt32 i = 0; i < fEngine->aParam.samples; i++) {
                    fEngine->pDelayOut[fEngine->aParam.channels * i + ch] =
                    output_data[ch][i];
                }
            }
            outData = (MInt8*)fEngine->pDelayOut;
        }
    }
    return outData;
}

MVoid StartDebug(MVoid* pEngine) {
    if (pEngine == NULL) {
        return;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);
    
    fEngine->socketFD = socket(AF_INET, SOCK_STREAM, 0);
    LOGI("StartDebug socket fd : %d\r\n", fEngine->socketFD);
    if (fEngine->socketFD == 0) {
        LOGE("StartDebug() create socket error, %d(%s)\n", errno, strerror(errno));
        return;
    }
    MInt32 reuse = 1;
    setsockopt(fEngine->socketFD, SOL_SOCKET, SO_REUSEPORT, (const MVoid*)&reuse,
               sizeof(int));
    setsockopt(fEngine->socketFD, SOL_SOCKET, SO_REUSEADDR, (const MVoid*)&reuse,
               sizeof(int));
    
    if (bind(fEngine->socketFD, (struct sockaddr*)&address, sizeof(address)) <
        0) {
        LOGE("StartDebug() bind socket error, %d(%s)\n", errno, strerror(errno));
        close(fEngine->socketFD);
        return;
    }
    if (listen(fEngine->socketFD, 10) < 0) {
        LOGE("StartDebug() listen socket error, %d(%s)\n", errno, strerror(errno));
        close(fEngine->socketFD);
        return;
    }
    pthread_t thread;
    MInt32 ret = pthread_create(&thread, NULL, ThreadRecvProcess, fEngine);
    if (ret == 0) {
        pthread_detach(thread);
    }
}

MVoid StopDebug(MVoid* pEngine) {
    if (pEngine == NULL) {
        return;
    }
    LPFilterEngine fEngine = (LPFilterEngine)pEngine;
    LOGI("StopDebug In bDebug: %d, fEngine->socketFD: %d\r\n", fEngine->bDebug,
         fEngine->socketFD);
    
    fEngine->bDebug = FALSE;
}
