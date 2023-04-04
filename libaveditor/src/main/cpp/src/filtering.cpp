//
// Created by 86193 on 2023/4/4.
//

#include "filtering.h"

int filtering::go_filter(const string &in_url,
                         const std::string &out_url,
                         const std::string &filter_desc) {
    int line, err = -1;
    AVFormatContext *av_fmt_ctx;
    AVCodec *av_codec;
    AVCodecContext *av_codec_ctx;
    {//init input
        if ((err = avformat_open_input(&av_fmt_ctx, in_url.c_str(), nullptr, nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if ((err = avformat_find_stream_info(av_fmt_ctx, nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if (!(av_codec = avcodec_find_decoder(av_fmt_ctx->video_codec_id))) {
            line = __LINE__;
            goto __ERR;
        }
        if (!(av_codec_ctx = avcodec_alloc_context3(av_codec))) {
            line = __LINE__;
            goto __ERR;
        }
    }
    AVFilterGraph *av_filter_graph;
    AVFilterContext *av_filter_buffer_src_ctx;
    AVFilterContext *av_filter_buffer_sink_ctx;
    AVFilterInOut *av_filter_in;
    AVFilterInOut *av_filter_out;
    {//init filter
        if (!(av_filter_graph = avfilter_graph_alloc())) {
            line = __LINE__;
            goto __ERR;
        }
        char args[512];
        snprintf(args, sizeof(args),
                 "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                 av_codec_ctx->width, av_codec_ctx->height, av_codec_ctx->pix_fmt,
                 av_codec_ctx->time_base.num, av_codec_ctx->time_base.den,
                 av_codec_ctx->sample_aspect_ratio.num, av_codec_ctx->sample_aspect_ratio.den);
        if ((err = avfilter_graph_create_filter(&av_filter_buffer_src_ctx,
                                                avfilter_get_by_name("buffer"),
                                                "in",
                                                args,
                                                nullptr,
                                                av_filter_graph)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if ((err = avfilter_graph_create_filter(&av_filter_buffer_sink_ctx,
                                                avfilter_get_by_name("buffersink"),
                                                "out",
                                                nullptr,
                                                nullptr,
                                                av_filter_graph)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if (!(av_filter_in = avfilter_inout_alloc()) ||
            !(av_filter_out = avfilter_inout_alloc())) {
            line = __LINE__;
            goto __ERR;
        }
        av_filter_in->name = av_strdup("in");
        av_filter_in->filter_ctx = av_filter_buffer_src_ctx;
        av_filter_in->pad_idx = 0;
        av_filter_in->next = nullptr;
        av_filter_out->name = av_strdup("out");
        av_filter_out->filter_ctx = av_filter_buffer_sink_ctx;
        av_filter_out->pad_idx = 0;
        av_filter_out->next = nullptr;
        if ((err = avfilter_graph_parse_ptr(av_filter_graph,
                                            filter_desc.c_str(),
                                            &av_filter_in,
                                            &av_filter_out,
                                            nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if ((err = avfilter_graph_config(av_filter_graph, nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
    }
    goto __FREE;
    __ERR:
    {
        LOGE("__ERR__ERR__ERR__ERR__ERR__go_filter(), line=%d, err=%d", line, err);
        avformat_close_input(&av_fmt_ctx);
        avformat_free_context(av_fmt_ctx);
        avcodec_free_context(&av_codec_ctx);
        avfilter_inout_free(&av_filter_in);
        avfilter_inout_free(&av_filter_out);
        return err;
    }
    __FREE:
    {
        avformat_close_input(&av_fmt_ctx);
        avformat_free_context(av_fmt_ctx);
        avcodec_free_context(&av_codec_ctx);
        avfilter_inout_free(&av_filter_in);
        avfilter_inout_free(&av_filter_out);
        return 0;
    }
}
