//
// Created by 86193 on 2023/3/21.
//
#ifndef AVEDITOR_COMMON_H
#define AVEDITOR_COMMON_H

//platform
#include "MLOG.h"

//stl
#include <string>
#include <vector>
#include <map>

//ffmpeg
extern "C" {
#include "include/ffmpeg/libavformat/avformat.h"
#include "include/ffmpeg/libavcodec//avcodec.h"
#include "include/ffmpeg/libswscale/swscale.h"
#include "include/ffmpeg/libswresample/swresample.h"
#include "include/ffmpeg/libavutil/timestamp.h"
#include "include/ffmpeg/libavutil/channel_layout.h"
#include "include/ffmpeg/libavutil/opt.h"
}


#endif //AVEDITOR_COMMON_H
