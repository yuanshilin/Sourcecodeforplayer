

#ifndef FILTER_LOG_H
#define FILTER_LOG_H

#ifdef _LINUX_ANDROID_
#include <android/log.h>

#define TAG __PRETTY_FUNCTION__
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#else
#define LOGD printf
#define LOGI printf
#define LOGW printf
#define LOGE printf
#endif

#endif //FILTER_LOG_H