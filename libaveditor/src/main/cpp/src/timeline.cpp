//
// Created by 86193 on 2023/4/3.
//

#include "timeline.h"

vector<long long> timeline::get_time_stamps(const string &in_url) {
    int line, err = -1;
    vector<long long> vec_dts = vector<long long>();
    AVFormatContext *av_fmt_ctx = avformat_alloc_context();
    if ((err = avformat_open_input(&av_fmt_ctx, in_url.c_str(), nullptr, nullptr)) < 0) {
        line = __LINE__;
        goto __ERR;
    }
    AVPacket *pkt;
    pkt = av_packet_alloc();
    while (av_read_frame(av_fmt_ctx, pkt) >= 0) {
        int codec_type = av_fmt_ctx->streams[pkt->stream_index]->codecpar->codec_type;
        if (codec_type != AVMEDIA_TYPE_VIDEO) {
            continue;
        }
        vec_dts.push_back(pkt->dts);
        av_packet_unref(pkt);
    }
    goto __FREE;
    __ERR:
    {
        LOGE("__ERR__ERR__ERR__ERR__ERR__get_dts(), line=%d, err=%d", line, err);
        goto __FREE;
    }
    __FREE:
    {
        avformat_close_input(&av_fmt_ctx);
        avformat_free_context(av_fmt_ctx);
        av_packet_free(&pkt);
        return vec_dts;
    }
}
