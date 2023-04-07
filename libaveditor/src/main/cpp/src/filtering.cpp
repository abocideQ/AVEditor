//
// Created by 86193 on 2023/4/4.
//

#include "filtering.h"

int filtering::go_filter(const string &in_url,
                         const std::string &out_url,
                         const std::string &filter_desc_video,
                         const std::string &filter_desc_audio) {

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
            model->av_filter_desc = model->codec_type == AVMEDIA_TYPE_VIDEO ?
                                    filter_desc_video :
                                    filter_desc_audio;
            model->stream_index = (int) i;
            m_av_filter_models[i] = model;
        }
        //out
        for (size_t i = 0; i < m_av_filter_models_size; i++) {
            AVFilterModel *model = m_av_filter_models[i];
            AVCodec *out_av_encoder = avcodec_find_encoder(model->codec_type == AVMEDIA_TYPE_VIDEO ?
                                                           m_out_avformat_ctx->oformat->video_codec
                                                                                                   :
                                                           m_out_avformat_ctx->oformat->audio_codec);
            if (!out_av_encoder) {
                line = __LINE__;
                goto __ERR;
            }
            AVCodecContext *out_av_encoder_ctx = avcodec_alloc_context3(out_av_encoder);
            if (!out_av_encoder_ctx) {
                line = __LINE__;
                goto __ERR;
            }
            LOGE("lib_x264 / fdk_aac, not build for current ffmpeg.so");
            LOGE("mpeg_encoder: %s", avcodec_get_name(out_av_encoder->id));
            if (model->codec_type == AVMEDIA_TYPE_VIDEO) {
                out_av_encoder_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
                out_av_encoder_ctx->codec_id = out_av_encoder->id;
                out_av_encoder_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
                out_av_encoder_ctx->width = model->in_av_decode_ctx->width;
                out_av_encoder_ctx->height = model->in_av_decode_ctx->height;
                out_av_encoder_ctx->gop_size = model->in_av_decode_ctx->gop_size;
                out_av_encoder_ctx->time_base = AVRational{1, (int) av_q2d(
                        model->in_av_decode_ctx->framerate)};
                out_av_encoder_ctx->bit_rate = model->in_av_decode_ctx->bit_rate;
            } else if (model->codec_type == AVMEDIA_TYPE_AUDIO) {
                out_av_encoder_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
                out_av_encoder_ctx->codec_id = out_av_encoder->id;
                out_av_encoder_ctx->sample_fmt = out_av_encoder->sample_fmts ?
                                                 out_av_encoder->sample_fmts[0] :
                                                 AV_SAMPLE_FMT_FLTP;
                int sample_rate = model->in_av_decode_ctx->sample_rate;
                if (out_av_encoder_ctx->codec->supported_samplerates) {
                    out_av_encoder_ctx->sample_rate = out_av_encoder_ctx->codec->supported_samplerates[0];
                    for (i = 0; out_av_encoder_ctx->codec->supported_samplerates[i]; i++) {
                        if (out_av_encoder_ctx->codec->supported_samplerates[i] == sample_rate) {
                            out_av_encoder_ctx->sample_rate = sample_rate;
                            break;
                        }
                    }
                }
                uint64_t channel_layout = model->in_av_decode_ctx->channel_layout;
                out_av_encoder_ctx->channel_layout = channel_layout;
                if (out_av_encoder_ctx->codec->channel_layouts) {
                    out_av_encoder_ctx->channel_layout = out_av_encoder_ctx->codec->channel_layouts[0];
                    for (i = 0; out_av_encoder_ctx->codec->channel_layouts[i]; i++) {
                        if (out_av_encoder_ctx->codec->channel_layouts[i] == channel_layout) {
                            out_av_encoder_ctx->channel_layout = channel_layout;
                            break;
                        }
                    }
                }
                out_av_encoder_ctx->channels = model->in_av_decode_ctx->channels;
                out_av_encoder_ctx->time_base = (AVRational) {1, out_av_encoder_ctx->sample_rate};
                out_av_encoder_ctx->bit_rate = model->in_av_decode_ctx->bit_rate;
            }
            if (m_out_avformat_ctx->flags & AVFMT_GLOBALHEADER) {
                //Some formats want stream headers to be separate
                out_av_encoder_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }
            if ((err = avcodec_open2(out_av_encoder_ctx, out_av_encoder, nullptr)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            AVStream *out_stream = avformat_new_stream(m_out_avformat_ctx, nullptr);
            if (out_stream == nullptr) {
                line = __LINE__;
                goto __ERR;
            }
            out_stream->id = (int) m_out_avformat_ctx->nb_streams - 1;
            out_stream->time_base = out_av_encoder_ctx->time_base;
            if ((err = avcodec_parameters_from_context(out_stream->codecpar,
                                                       out_av_encoder_ctx)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            model->out_av_decode_ctx = out_av_encoder_ctx;
        }
    }
    {//open filter
        for (size_t i = 0; i < m_av_filter_models_size; i++) {
            AVFilterModel *model = m_av_filter_models[i];
            if (model->av_filter_desc.empty()) {
                continue;
            }
            if (model->codec_type == AVMEDIA_TYPE_VIDEO) {
                if (!(model->av_filter_graph = avfilter_graph_alloc())) {
                    line = __LINE__;
                    goto __ERR;
                }
                char args[512];
                snprintf(args, sizeof(args),
                         "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                         model->in_av_decode_ctx->width,
                         model->in_av_decode_ctx->height,
                         model->in_av_decode_ctx->pix_fmt,
                         (int) 1,
                         (int) av_q2d(model->in_av_decode_ctx->framerate),
                         model->in_av_decode_ctx->sample_aspect_ratio.num,
                         model->in_av_decode_ctx->sample_aspect_ratio.den);
                if ((err = avfilter_graph_create_filter(&model->av_filter_buffer_src_ctx,
                                                        avfilter_get_by_name("buffer"),
                                                        "in",
                                                        args,
                                                        nullptr,
                                                        model->av_filter_graph)) < 0) {
                    line = __LINE__;
                    goto __ERR;
                }
                if ((err = avfilter_graph_create_filter(&model->av_filter_buffer_sink_ctx,
                                                        avfilter_get_by_name("buffersink"),
                                                        "out",
                                                        nullptr,
                                                        nullptr,
                                                        model->av_filter_graph)) < 0) {
                    line = __LINE__;
                    goto __ERR;
                }
                enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};
                if ((err = av_opt_set_int_list(model->av_filter_buffer_sink_ctx,
                                               "pix_fmts",
                                               pix_fmts,
                                               AV_PIX_FMT_NONE,
                                               AV_OPT_SEARCH_CHILDREN)) < 0) {
                    line = __LINE__;
                    goto __ERR;
                }
                if (!(model->av_filter_in = avfilter_inout_alloc()) ||
                    !(model->av_filter_out = avfilter_inout_alloc())) {
                    line = __LINE__;
                    goto __ERR;
                }
                model->av_filter_in->name = av_strdup("in");
                model->av_filter_in->filter_ctx = model->av_filter_buffer_src_ctx;
                model->av_filter_in->pad_idx = 0;
                model->av_filter_in->next = nullptr;
                model->av_filter_out->name = av_strdup("out");
                model->av_filter_out->filter_ctx = model->av_filter_buffer_sink_ctx;
                model->av_filter_out->pad_idx = 0;
                model->av_filter_out->next = nullptr;
                if ((err = avfilter_graph_parse_ptr(model->av_filter_graph,
                                                    model->av_filter_desc.c_str(),
                                                    &model->av_filter_out,//* link out to in
                                                    &model->av_filter_in,//* link in to out
                                                    nullptr)) < 0) {
                    line = __LINE__;
                    goto __ERR;
                }
                if ((err = avfilter_graph_config(model->av_filter_graph, nullptr)) < 0) {
                    line = __LINE__;
                    goto __ERR;
                }
            } else if (model->codec_type == AVMEDIA_TYPE_AUDIO) {

            }
        }
    }
    {//write header
        if ((err = avformat_write_header(m_out_avformat_ctx, nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
    }
    {//filter
        m_packet = av_packet_alloc();
        if (m_packet == nullptr) {
            line = __LINE__;
            goto __ERR;
        }
        m_frame = av_frame_alloc();
        if (m_frame == nullptr) {
            line = __LINE__;
            goto __ERR;
        }
        while (av_read_frame(m_in_avformat_ctx, m_packet) >= 0) {
            AVFilterModel *model = m_av_filter_models[m_packet->stream_index];
            if (model == nullptr) {
                continue;
            }
            if ((err = avcodec_send_packet(model->in_av_decode_ctx, m_packet)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            while (avcodec_receive_frame(model->in_av_decode_ctx, m_frame) >= 0) {
                AVFrame *dst_frame = av_frame_alloc();
                if (model->av_filter_desc.empty()) {
                    dst_frame = av_frame_clone(m_frame);
                    if ((err = av_frame_make_writable(dst_frame)) < 0) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    if ((err = avcodec_send_frame(model->out_av_decode_ctx,
                                                  dst_frame)) < 0) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    AVPacket *dst_packet = av_packet_alloc();
                    while (avcodec_receive_packet(model->out_av_decode_ctx,
                                                  dst_packet) >= 0) {
                        dst_packet->stream_index = model->stream_index;
                        av_packet_rescale_ts(dst_packet,
                                             model->out_av_decode_ctx->time_base,
                                             m_out_avformat_ctx->streams[model->stream_index]->time_base);
                        if ((err = av_interleaved_write_frame(m_out_avformat_ctx,
                                                              dst_packet)) < 0) {
                            line = __LINE__;
                            goto __ERR;
                        }
                        av_packet_unref(dst_packet);
                    }
                    av_frame_unref(dst_frame);
                    av_packet_free(&dst_packet);
                } else {
                    if (av_buffersrc_add_frame_flags(model->av_filter_buffer_src_ctx,
                                                     m_frame,
                                                     AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    while (av_buffersink_get_frame(model->av_filter_buffer_sink_ctx,
                                                   dst_frame) >= 0) {
                        if ((err = av_frame_make_writable(dst_frame)) < 0) {
                            line = __LINE__;
                            goto __ERR;
                        }
                        if ((err = avcodec_send_frame(model->out_av_decode_ctx,
                                                      dst_frame)) < 0) {
                            line = __LINE__;
                            goto __ERR;
                        }
                        AVPacket *dst_packet = av_packet_alloc();
                        while (avcodec_receive_packet(model->out_av_decode_ctx,
                                                      dst_packet) >= 0) {
                            dst_packet->stream_index = model->stream_index;
                            av_packet_rescale_ts(dst_packet,
                                                 model->out_av_decode_ctx->time_base,
                                                 m_out_avformat_ctx->streams[model->stream_index]->time_base);
                            if ((err = av_interleaved_write_frame(m_out_avformat_ctx,
                                                                  dst_packet)) < 0) {
                                line = __LINE__;
                                goto __ERR;
                            }
                            av_packet_unref(dst_packet);
                        }
                        av_frame_unref(dst_frame);
                        av_packet_free(&dst_packet);
                    }
                    av_frame_free(&dst_frame);
                }
                av_frame_unref(m_frame);
            }
            av_packet_unref(m_packet);
        }
    }
    {//write footer
        av_write_trailer(m_out_avformat_ctx);
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
