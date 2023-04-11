//
// Created by 86193 on 2023/4/10.
//

#include "merger.h"

int merger::merger_merge(const std::vector<std::string> &in_urls,
                         const std::string &out_url) {
    int line, err = -1;
    for (size_t position = 0; position < in_urls.size(); position++) {
        AVFormatContext *m_in_av_fmt_ctx;
        {//open avformat
            //in
            m_in_av_fmt_ctx = avformat_alloc_context();
            if ((err = avformat_open_input(&m_in_av_fmt_ctx,
                                           in_urls[position].c_str(),
                                           nullptr,
                                           nullptr)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            if ((err = avformat_find_stream_info(m_in_av_fmt_ctx, nullptr)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            //out
            if (position == 0) {
                m_out_av_fmt_ctx = avformat_alloc_context();
                if ((err = avformat_alloc_output_context2(&m_out_av_fmt_ctx,
                                                          nullptr,
                                                          nullptr,
                                                          out_url.c_str())) < 0) {
                    line = __LINE__;
                    goto __ERR;
                }
                if ((err = avio_open(&m_out_av_fmt_ctx->pb,
                                     out_url.c_str(),
                                     AVIO_FLAG_WRITE)) < 0) {
                    line = __LINE__;
                    goto __ERR;
                }
            }
        }
        {//open avcodec
            //in
            m_stream_models = position == 0 ? new AVStreamModel *[m_in_av_fmt_ctx->nb_streams]
                                            : m_stream_models;
            for (size_t i = 0; i < m_in_av_fmt_ctx->nb_streams; i++) {
                AVStream *in_stream = m_in_av_fmt_ctx->streams[i];
                if (in_stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
                    in_stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
                    continue;
                } else {
                    m_stream_models_size = position == 0 ? m_stream_models_size + 1
                                                         : m_stream_models_size;
                }
                AVCodec *in_decoder = avcodec_find_decoder(in_stream->codecpar->codec_id);
                if (!in_decoder) {
                    line = __LINE__;
                    goto __ERR;
                }
                AVCodecContext *in_av_decode_ctx = avcodec_alloc_context3(in_decoder);
                if (!in_av_decode_ctx) {
                    line = __LINE__;
                    goto __ERR;
                }
                if ((err = avcodec_parameters_to_context(in_av_decode_ctx,
                                                         in_stream->codecpar)) < 0) {
                    line = __LINE__;
                    goto __ERR;
                }
                if ((err = avcodec_open2(in_av_decode_ctx,
                                         in_decoder,
                                         nullptr)) < 0) {
                    line = __LINE__;
                    goto __ERR;
                }
                AVStreamModel *model = (position == 0) ? m_stream_models[i] = new AVStreamModel()
                                                       : m_stream_models[i];
                model->in_av_decode_ctx = in_av_decode_ctx;
                model->in_av_decode_ctx->framerate = in_stream->avg_frame_rate;
                model->codec_type = in_stream->codecpar->codec_type;
                model->time_base = in_stream->time_base;
                model->stream_index = (int) i;
            }
            //out
            if (position == 0) {
                for (size_t i = 0; i < m_stream_models_size; i++) {
                    AVStreamModel *model = m_stream_models[i];
                    AVCodec *out_encoder = model->codec_type == AVMEDIA_TYPE_VIDEO ?
                                           avcodec_find_encoder(
                                                   m_out_av_fmt_ctx->oformat->video_codec) :
                                           avcodec_find_encoder(
                                                   m_out_av_fmt_ctx->oformat->audio_codec);
                    if (!out_encoder) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    AVCodecContext *out_encoder_ctx = avcodec_alloc_context3(out_encoder);
                    if (!out_encoder_ctx) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    if (model->codec_type == AVMEDIA_TYPE_VIDEO) {
                        out_encoder_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
                        out_encoder_ctx->codec_id = out_encoder->id;
                        out_encoder_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
                        out_encoder_ctx->width = model->in_av_decode_ctx->width;
                        out_encoder_ctx->height = model->in_av_decode_ctx->height;
                        out_encoder_ctx->gop_size = model->in_av_decode_ctx->gop_size;
                        out_encoder_ctx->time_base = AVRational{1, (int) av_q2d(
                                model->in_av_decode_ctx->framerate)};
                        out_encoder_ctx->bit_rate = model->in_av_decode_ctx->bit_rate;
                    } else if (model->in_av_decode_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
                        out_encoder_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
                        out_encoder_ctx->codec_id = out_encoder->id;
                        out_encoder_ctx->sample_fmt = out_encoder->sample_fmts ?
                                                      out_encoder->sample_fmts[0] :
                                                      AV_SAMPLE_FMT_FLTP;
                        int sample_rate = model->in_av_decode_ctx->sample_rate;
                        if (out_encoder_ctx->codec->supported_samplerates) {
                            out_encoder_ctx->sample_rate = out_encoder_ctx->codec->supported_samplerates[0];
                            for (int k = 0; out_encoder_ctx->codec->supported_samplerates[k]; k++) {
                                if (out_encoder_ctx->codec->supported_samplerates[k] ==
                                    sample_rate) {
                                    out_encoder_ctx->sample_rate = sample_rate;
                                    break;
                                }
                            }
                        }
                        uint64_t channel_layout = model->in_av_decode_ctx->channel_layout;
                        out_encoder_ctx->channel_layout = channel_layout;
                        if (out_encoder_ctx->codec->channel_layouts) {
                            out_encoder_ctx->channel_layout = out_encoder_ctx->codec->channel_layouts[0];
                            for (int k = 0; out_encoder_ctx->codec->channel_layouts[k]; k++) {
                                if (out_encoder_ctx->codec->channel_layouts[k] == channel_layout) {
                                    out_encoder_ctx->channel_layout = channel_layout;
                                    break;
                                }
                            }
                        }
                        out_encoder_ctx->channels = model->in_av_decode_ctx->channels;
                        out_encoder_ctx->time_base = (AVRational) {1, out_encoder_ctx->sample_rate};
                        out_encoder_ctx->bit_rate = model->in_av_decode_ctx->bit_rate;
                    }
                    if (m_out_av_fmt_ctx->flags & AVFMT_GLOBALHEADER) {
                        //Some formats want stream headers to be separate
                        out_encoder_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
                    }
                    if ((err = avcodec_open2(out_encoder_ctx, out_encoder, nullptr)) < 0) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    AVStream *out_stream = avformat_new_stream(m_out_av_fmt_ctx, nullptr);
                    if (out_stream == nullptr) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    out_stream->id = (int) m_out_av_fmt_ctx->nb_streams - 1;
                    out_stream->time_base = out_encoder_ctx->time_base;
                    if ((err = avcodec_parameters_from_context(out_stream->codecpar,
                                                               out_encoder_ctx)) < 0) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    model->out_av_decode_ctx = out_encoder_ctx;
                }
            }
        }
        {//write header
            if (position == 0) {
                if ((err = avformat_write_header(m_out_av_fmt_ctx, nullptr)) < 0) {
                    line = __LINE__;
                    goto __ERR;
                }
            }
        }
        {//merge
            AVPacket *m_packet;
            if ((m_packet = av_packet_alloc()) == nullptr) {
                line = __LINE__;
                goto __ERR;
            }
            AVFrame *m_frame;
            if ((m_frame = av_frame_alloc()) == nullptr) {
                line = __LINE__;
                goto __ERR;
            }
            while (av_read_frame(m_in_av_fmt_ctx, m_packet) >= 0) {
                AVStreamModel *model = m_stream_models[m_packet->stream_index];
                if (!model) {
                    continue;
                }
                if ((err = avcodec_send_packet(model->in_av_decode_ctx, m_packet)) < 0) {
                    line = __LINE__;
                    goto __ERR;
                }
                while (avcodec_receive_frame(model->in_av_decode_ctx, m_frame) >= 0) {
                    AVFrame *dst_frame = av_frame_alloc();
                    if (model->codec_type == AVMEDIA_TYPE_VIDEO) {
                        dst_frame->format = model->out_av_decode_ctx->pix_fmt;
                        dst_frame->width = model->out_av_decode_ctx->width;
                        dst_frame->height = model->out_av_decode_ctx->height;
                        if (model->in_sws_ctx == nullptr) {
                            model->in_sws_ctx = sws_getContext(model->in_av_decode_ctx->width,
                                                               model->in_av_decode_ctx->height,
                                                               model->in_av_decode_ctx->pix_fmt,
                                                               dst_frame->width,
                                                               dst_frame->height,
                                                               AVPixelFormat(dst_frame->format),
                                                               SWS_FAST_BILINEAR,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr);
                        }
                        dst_frame->pts = model->next_pts;
                        model->next_pts++;
                        av_frame_get_buffer(dst_frame, 0);//malloc frame->buffer
                        sws_scale(model->in_sws_ctx,
                                  m_frame->data,
                                  m_frame->linesize,
                                  0,
                                  m_frame->height,
                                  dst_frame->data,
                                  dst_frame->linesize);
                    } else if (model->codec_type == AVMEDIA_TYPE_AUDIO) {
                        dst_frame->format = model->out_av_decode_ctx->sample_fmt;//Cross=L R L R
                        dst_frame->channel_layout = model->out_av_decode_ctx->channel_layout;//L&R
                        dst_frame->channels = model->out_av_decode_ctx->channels;
                        dst_frame->sample_rate = model->out_av_decode_ctx->sample_rate;
                        if (model->in_swr_ctx == nullptr) {
                            model->in_swr_ctx = swr_alloc();
                            swr_alloc_set_opts(model->in_swr_ctx,
                                               (int64_t) dst_frame->channel_layout,
                                               AVSampleFormat(dst_frame->format),
                                               dst_frame->sample_rate,
                                               (int64_t) model->in_av_decode_ctx->channel_layout,
                                               model->in_av_decode_ctx->sample_fmt,
                                               model->in_av_decode_ctx->sample_rate,
                                               0,
                                               nullptr);
                            swr_init(model->in_swr_ctx);
                        }
                        //swr_get_delay: (if set sample_rate) get nb_samples last time has not done.
                        int64_t delay_nb_samples = swr_get_delay(model->in_swr_ctx,
                                                                 m_frame->sample_rate);
                        dst_frame->nb_samples = (int) av_rescale_rnd(
                                m_frame->nb_samples + delay_nb_samples,
                                dst_frame->sample_rate,
                                m_frame->sample_rate,
                                AV_ROUND_UP
                        );
                        dst_frame->pts = model->next_pts;
                        model->next_pts += dst_frame->nb_samples;
                        av_frame_get_buffer(dst_frame, 0);//malloc frame->buffer
                        swr_convert(model->in_swr_ctx,
                                    dst_frame->data,
                                    dst_frame->nb_samples,
                                    (const uint8_t **) m_frame->data,
                                    m_frame->nb_samples);
                    }
                    if ((err = av_frame_make_writable(dst_frame)) < 0) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    if ((err = avcodec_send_frame(model->out_av_decode_ctx, dst_frame)) < 0) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    AVPacket *dst_packet = av_packet_alloc();
                    while (avcodec_receive_packet(model->out_av_decode_ctx, dst_packet) >= 0) {
                        dst_packet->stream_index = model->stream_index;
                        av_packet_rescale_ts(dst_packet,
                                             model->out_av_decode_ctx->time_base,
                                             model->time_base);
                        if ((err = av_interleaved_write_frame(m_out_av_fmt_ctx,
                                                              dst_packet)) < 0) {
                            line = __LINE__;
                            goto __ERR;
                        }
                        av_packet_unref(dst_packet);
                    }
                    av_packet_free(&dst_packet);
                    av_frame_unref(dst_frame);
                    av_frame_free(&dst_frame);
                    av_frame_unref(m_frame);
                }
                av_packet_unref(m_packet);
            }
            av_packet_free(&m_packet);
            av_frame_free(&m_frame);
        }
        {//write footer
            if (position == in_urls.size() - 1) {
                av_write_trailer(m_out_av_fmt_ctx);
            }
        }
        {//free in
            for (size_t j = 0; j < m_stream_models_size; j++) {
                AVStreamModel *model = m_stream_models[j];
                avcodec_free_context(&model->in_av_decode_ctx);
            }
            avformat_close_input(&m_in_av_fmt_ctx);
            avformat_free_context(m_in_av_fmt_ctx);
        }

    }
    {//free out
        for (size_t j = 0; j < m_stream_models_size; j++) {
            AVStreamModel *model = m_stream_models[j];
            avcodec_free_context(&model->out_av_decode_ctx);
        }
        avformat_free_context(m_out_av_fmt_ctx);
    }
    return 0;
    __ERR:
    {
        LOGE("__ERR__ERR__ERR__ERR__ERR__recode_codec(), line=%d, err=%d", line, err);
        return err;
    }
}
