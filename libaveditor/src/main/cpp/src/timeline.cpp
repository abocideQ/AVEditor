//
// Created by 86193 on 2023/4/3.
//

#include "timeline.h"

vector<long double> timeline::get_time_stamps(const string &in_url) {
    int line, err = -1;
    vector<long double> vec_dts = vector<long double>();
    AVFormatContext *av_fmt_ctx = avformat_alloc_context();
    if ((err = avformat_open_input(&av_fmt_ctx, in_url.c_str(), nullptr, nullptr)) < 0) {
        line = __LINE__;
        goto __ERR;
    }
    AVPacket *pkt;
    pkt = av_packet_alloc();
    while (av_read_frame(av_fmt_ctx, pkt) >= 0) {
        AVStream *stream = av_fmt_ctx->streams[pkt->stream_index];
        int codec_type = stream->codecpar->codec_type;
        if (codec_type != AVMEDIA_TYPE_VIDEO) {
            continue;
        }
        long double pts = (long double) pkt->pts * av_q2d(stream->time_base);
        LOGE("timeline, pts=%Lf", pts);
        vec_dts.push_back(pts);
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
