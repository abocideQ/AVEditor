//
// Created by 86193 on 2023/4/4.
//

#ifndef AVEDITOR_FILTERING_H
#define AVEDITOR_FILTERING_H

#include "common.h"

using namespace std;

typedef struct av_filter_model {
    int stream_index;
    AVMediaType codec_type;
    AVCodecContext *in_av_decode_ctx;

    std::string av_filter_desc;
    AVFilterGraph *av_filter_graph;
    AVFilterContext *av_filter_buffer_src_ctx;
    AVFilterContext *av_filter_buffer_sink_ctx;
    AVFilterInOut *av_filter_in;
    AVFilterInOut *av_filter_out;

    AVCodecContext *out_av_decode_ctx;
} AVFilterModel;

class filtering {
public:
    /**
     *  filter_desc_video ->
     *  scale缩放     :   scale=w:h (exp: iw*2:ih*2)
     *  crop裁剪      :   crop=w:h:x:y
     *  rotate旋转    :   rotate=angle (exp: 1*PI (1*PI = 1π = 180度))
     *  overlay叠加   :   marsk.png[marsk];[in][marsk]overlay=x:y
     *  main_w或W       主输入(背景窗口)宽度
     *  main_h或H       主输入(背景窗口)高度
     *  overlay_w或w    overlay输入(前景窗口)宽度
     *  overlay_h或h    overlay输入(前景窗口)高度
     *
     *
     *  filter_desc_audio ->
     *  ???
     */
    int go_filter(const std::string &,
                  const std::string &,
                  const std::string &,
                  const std::string &);

    void go_filter_free();

private:
    AVFormatContext *m_in_avformat_ctx;
    AVPacket *m_packet;
    AVFrame *m_frame;
    AVFormatContext *m_out_avformat_ctx;
    int m_av_filter_models_size;
    AVFilterModel **m_av_filter_models;
};


#endif //AVEDITOR_FILTERING_H
