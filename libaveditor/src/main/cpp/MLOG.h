//
// Created by 86193 on 2023/3/22.
//
#ifndef AVEDITOR_MLOG_H
#define AVEDITOR_MLOG_H

#ifdef __WIN32__

#include <stdio.h>

#define LOG_TAG "WIN32_LOG"
#define LOGE(...) ((void)printf(LOG_TAG, __VA_ARGS__))

#endif

#ifdef __ANDROID__

#include <android/log.h>

#define LOG_TAG "ANDROID_LOG"
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

#endif

#endif //AVEDITOR_MLOG_H
