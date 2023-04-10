//
// Created by 86193 on 2023/4/10.
//

#ifndef AVEDITOR_MERGER_H
#define AVEDITOR_MERGER_H

#include "common.h"

class merger {
public:
    typedef struct av_stream_model {
        int stream_index;
        AVMediaType codec_type;
        AVRational time_base;
        AVCodecContext *in_av_decode_ctx;
        AVCodecContext *out_av_decode_ctx;
        int64_t next_pts;
    } AVStreamModel;

public:
    int merger_merge(const std::vector<std::string> &, const std::string &);

private:
    AVFormatContext *m_out_av_fmt_ctx;
    int m_stream_models_size;
    AVStreamModel **m_stream_models;
};


#endif //AVEDITOR_MERGER_H
