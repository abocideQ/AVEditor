//
// Created by 86193 on 2023/4/4.
//

#include "filtering.h"

int filtering::go_filter(const string &in_url,
                         const std::string &out_url,
                         const std::string &filter_desc) {

    int line, err = -1;
    {//open avformat
        //in
        m_in_avformat_ctx = avformat_alloc_context();
        if ((err = avformat_open_input(&m_in_avformat_ctx,
                                       in_url.c_str(),
                                       nullptr,
                                       nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if ((err = avformat_find_stream_info(m_in_avformat_ctx, nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        //out
        if ((err = avformat_alloc_output_context2(&m_out_avformat_ctx,
                                                  nullptr,
                                                  nullptr,
                                                  out_url.c_str())) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if ((err = avio_open(&m_out_avformat_ctx->pb, out_url.c_str(), AVIO_FLAG_WRITE)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
    }
    {//open avcodec
        //in
        m_av_filter_models = new AVFilterModel *[m_in_avformat_ctx->nb_streams];
        for (size_t i = 0; i < m_in_avformat_ctx->nb_streams; ++i) {
            AVStream *in_stream = m_in_avformat_ctx->streams[i];
            if (in_stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
                in_stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
                continue;
            } else {
                m_av_filter_models_size++;
            }
            auto *model = new AVFilterModel();
            AVCodec *av_codec = avcodec_find_decoder(in_stream->codecpar->codec_id);
            if (!av_codec) {
                line = __LINE__;
                goto __ERR;
            }
            model->in_av_decode_ctx = avcodec_alloc_context3(av_codec);
            if (!model->in_av_decode_ctx) {
                line = __LINE__;
                goto __ERR;
            }
            if ((err = avcodec_parameters_to_context(model->in_av_decode_ctx,
                                                     in_stream->codecpar)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            if ((err = avcodec_open2(model->in_av_decode_ctx,
                                     av_codec,
                                     nullptr)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            model->in_av_decode_ctx->framerate = in_stream->avg_frame_rate;
            model->codec_type = in_stream->codecpar->codec_type;
            model->stream_index = (int) i;
            model->in_time_base = in_stream->time_base;
            m_av_filter_models[i] = model;
        }
        //out

    }
    {//init filter
        /*if (!(av_filter_graph = avfilter_graph_alloc())) {
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
        }*/
    }
    goto __FREE;
    __ERR:
    {
        LOGE("__ERR__ERR__ERR__ERR__ERR__go_filter(), line=%d, err=%d", line, err);
        go_filter_free();
        return err;
    }
    __FREE:
    {
        go_filter_free();
        return 0;
    }
}

void filtering::go_filter_free() {
    if (m_frame) {
        av_frame_free(&m_frame);
    }
    if (m_packet) {
        av_packet_free(&m_packet);
    }
    if (m_av_filter_models) {
        for (int i = 0; i < m_av_filter_models_size; i++) {
            AVFilterModel *model = m_av_filter_models[i];
            if (model->in_av_decode_ctx) {
                avcodec_free_context(&model->in_av_decode_ctx);
            }
            if (model->out_av_decode_ctx) {
                avcodec_free_context(&model->out_av_decode_ctx);
            }
        }
        m_av_filter_models_size = 0;
    }
    if (m_in_avformat_ctx) {
        avformat_free_context(m_in_avformat_ctx);
    }
    if (m_out_avformat_ctx) {
        avformat_free_context(m_out_avformat_ctx);
    }
}
