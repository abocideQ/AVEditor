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
    AVRational in_time_base;

    std::string av_filter_desc;
    AVFilterGraph *av_filter_graph;
    AVFilterContext *av_filter_buffer_src_ctx;
    AVFilterContext *av_filter_buffer_sink_ctx;
    AVFilterInOut *av_filter_in;
    AVFilterInOut *av_filter_out;

    int64_t next_pts;
    AVCodecContext *out_av_decode_ctx;
} AVFilterModel;

class filtering {
public:
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
