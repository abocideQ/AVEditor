//
// Created by 86193 on 2023/4/10.
//

#include "merger.h"

int merger::merger_merge(const std::vector <std::string> &in_urls,
                         const std::string &out_url) {
    int line, err = -1;
    {//open avformat
        //out
        m_out_av_fmt_ctx = avformat_alloc_context();
        if ((err = avformat_alloc_output_context2(&m_out_av_fmt_ctx,
                                                  nullptr,
                                                  nullptr,
                                                  out_url.c_str())) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if ((err = avio_open(&m_out_av_fmt_ctx->pb, out_url.c_str(), AVIO_FLAG_WRITE)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
    }
    //in
    AVFormatContext *m_in_av_fmt_ctx;
    for (size_t i = 0; i < in_urls.size(); i++) {
        {//open avcodec & avformat
            m_in_av_fmt_ctx = avformat_alloc_context();
            if ((err = avformat_open_input(&m_in_av_fmt_ctx,
                                           in_urls[i].c_str(),
                                           nullptr,
                                           nullptr)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            if ((err = avformat_find_stream_info(m_in_av_fmt_ctx, nullptr)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            for (size_t j = 0; i < m_in_av_fmt_ctx->nb_streams; j++) {
                if (m_in_av_fmt_ctx->streams[j]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
                    m_in_av_fmt_ctx->streams[j]->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
                    continue;
                }
                AVStream *in_stream = m_in_av_fmt_ctx->streams[i];
                AVCodec *in_decoder = avcodec_find_decoder(in_stream->codecpar->codec_id);
                if (!in_decoder) {
                    line = __LINE__;
                    goto __ERR;
                }
                AVCodecContext *in_decoder_ctx = avcodec_alloc_context3(in_decoder);
                if (!in_decoder_ctx) {
                    line = __LINE__;
                    goto __ERR;
                }
                if ((err = avcodec_parameters_to_context(in_decoder_ctx, in_stream->codecpar)) <
                    0) {
                    line = __LINE__;
                    goto __ERR;
                }
                if ((err = avcodec_open2(in_decoder_ctx, in_decoder, nullptr)) < 0) {
                    line = __LINE__;
                    goto __ERR;
                }
                in_decoder_ctx->framerate = in_stream->avg_frame_rate;
                if (i == 0) {//out
                    if (m_stream_models_size == 0) {
                        m_stream_models_size = (int) m_in_av_fmt_ctx->nb_streams;
                        m_stream_models = new AVStreamModel *[m_stream_models_size];
                    }
                    AVCodec *out_encoder = in_decoder_ctx->codec_type == AVMEDIA_TYPE_VIDEO ?
                                           avcodec_find_encoder(m_out_av_fmt_ctx->video_codec->id) :
                                           avcodec_find_encoder(m_out_av_fmt_ctx->audio_codec->id);
                    if (!out_encoder) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    AVCodecContext *out_encoder_ctx = avcodec_alloc_context3(out_encoder);
                    if (!out_encoder_ctx) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    if (in_decoder_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                        out_encoder_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
                        out_encoder_ctx->codec_id = out_encoder->id;
                        out_encoder_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
                        out_encoder_ctx->width = in_decoder_ctx->width;
                        out_encoder_ctx->height = in_decoder_ctx->height;
                        out_encoder_ctx->gop_size = in_decoder_ctx->gop_size;
                        out_encoder_ctx->time_base = AVRational{1, (int) av_q2d(
                                in_decoder_ctx->framerate)};
                        out_encoder_ctx->bit_rate = in_decoder_ctx->bit_rate;
                    } else if (in_decoder_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
                        out_encoder_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
                        out_encoder_ctx->codec_id = out_encoder->id;
                        out_encoder_ctx->sample_fmt = out_encoder->sample_fmts ?
                                                      out_encoder->sample_fmts[0] :
                                                      AV_SAMPLE_FMT_FLTP;
                        int sample_rate = in_decoder_ctx->sample_rate;
                        if (out_encoder_ctx->codec->supported_samplerates) {
                            out_encoder_ctx->sample_rate = out_encoder_ctx->codec->supported_samplerates[0];
                            for (i = 0; out_encoder_ctx->codec->supported_samplerates[i]; i++) {
                                if (out_encoder_ctx->codec->supported_samplerates[i] ==
                                    sample_rate) {
                                    out_encoder_ctx->sample_rate = sample_rate;
                                    break;
                                }
                            }
                        }
                        uint64_t channel_layout = in_decoder_ctx->channel_layout;
                        out_encoder_ctx->channel_layout = channel_layout;
                        if (out_encoder_ctx->codec->channel_layouts) {
                            out_encoder_ctx->channel_layout = out_encoder_ctx->codec->channel_layouts[0];
                            for (i = 0; out_encoder_ctx->codec->channel_layouts[i]; i++) {
                                if (out_encoder_ctx->codec->channel_layouts[i] == channel_layout) {
                                    out_encoder_ctx->channel_layout = channel_layout;
                                    break;
                                }
                            }
                        }
                        out_encoder_ctx->channels = in_decoder_ctx->channels;
                        out_encoder_ctx->time_base = (AVRational) {1, out_encoder_ctx->sample_rate};
                        out_encoder_ctx->bit_rate = in_decoder_ctx->bit_rate;
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
                    auto *model = new AVStreamModel();
                    model->codec_type = in_stream->codecpar->codec_type;
                    model->stream_index = (int) j;
                    model->time_base = in_stream->time_base;
                    model->out_av_decode_ctx = out_encoder_ctx;
                    m_stream_models[j] = model;
                }
                m_stream_models[j]->in_av_decode_ctx = in_decoder_ctx;
            }
        }
        {//merge
            AVPacket *m_packet;
            AVFrame *m_frame;
            if ((m_packet = av_packet_alloc()) == nullptr) {
                line = __LINE__;
                goto __ERR;
            }
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
                    AVFrame *dst_frame;
                    dst_frame = av_frame_clone(m_frame);
                    if ((err = av_frame_make_writable(dst_frame)) < 0) {
                        line = __LINE__;
                        goto __ERR;
                    }
                    AVPacket *dst_packet = av_packet_alloc();
                    while (avcodec_receive_packet(model->out_av_decode_ctx, dst_packet) >= 0) {
                        if (model->codec_type == AVMEDIA_TYPE_VIDEO) {
                            dst_packet->pts = model->next_pts;
                            model->next_pts++;
                        } else if (model->codec_type == AVMEDIA_TYPE_AUDIO) {
                            dst_packet->pts = model->next_pts;
                            model->next_pts = dst_frame->nb_samples;
                        }
                        dst_packet->stream_index = model->stream_index;
                        av_packet_rescale_ts(dst_packet, model->out_av_decode_ctx->time_base,
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
        for (size_t j = 0; j < m_stream_models_size; j++) {
            AVStreamModel *model = m_stream_models[j];
            avcodec_free_context(&model->in_av_decode_ctx);
        }
        avformat_close_input(&m_in_av_fmt_ctx);
        avformat_free_context(m_in_av_fmt_ctx);
    }
    for (size_t j = 0; j < m_stream_models_size; j++) {
        AVStreamModel *model = m_stream_models[j];
        avcodec_free_context(&model->out_av_decode_ctx);
    }
    avformat_free_context(m_out_av_fmt_ctx);
    return 0;
    __ERR:
    {
        LOGE("__ERR__ERR__ERR__ERR__ERR__recode_codec(), line=%d, err=%d", line, err);
        return err;
    }
}
