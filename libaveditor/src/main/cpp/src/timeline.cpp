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

int timeline::write_time_line_jpg(const string &in_url, const string &out_folder_url, int ratio) {
    int line, err = -1;
    AVFormatContext *m_av_fmt_ctx = avformat_alloc_context();
    {//open avformat
        if ((err = avformat_open_input(&m_av_fmt_ctx, in_url.c_str(), nullptr, nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if ((err = avformat_find_stream_info(m_av_fmt_ctx, nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
    }
    AVCodecContext *m_av_decoder_ctx;
    {//open avcodec
        for (size_t i = 0; i < m_av_fmt_ctx->nb_streams; i++) {
            AVStream *stream = m_av_fmt_ctx->streams[i];
            if (stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
                continue;
            }
            AVCodec *av_decoder = avcodec_find_decoder(stream->codecpar->codec_id);
            if (av_decoder == nullptr) {
                line = __LINE__;
                goto __ERR;
            }
            AVCodecContext *av_decoder_ctx = avcodec_alloc_context3(av_decoder);
            if (av_decoder_ctx == nullptr) {
                line = __LINE__;
                goto __ERR;
            }
            if ((err = avcodec_parameters_to_context(av_decoder_ctx, stream->codecpar)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            if ((err = avcodec_open2(av_decoder_ctx, av_decoder, nullptr)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            m_av_decoder_ctx = av_decoder_ctx;
        }
    }
    {//write
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
        long double last_write_time = 0;
        while (av_read_frame(m_av_fmt_ctx, m_packet) >= 0) {
            AVStream *stream = m_av_fmt_ctx->streams[m_packet->stream_index];
            if (stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
                continue;
            }
            if ((err = avcodec_send_packet(m_av_decoder_ctx, m_packet)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
            while (avcodec_receive_frame(m_av_decoder_ctx, m_frame) >= 0) {
                long double write_time = (long double) m_frame->pts * av_q2d(stream->time_base);
                if (write_time - last_write_time < 1) {
                    if ((err = write_jpg(m_frame,
                                         out_folder_url + "/"
                                         + std::to_string((int) write_time)
                                         + ".jpg",
                                         ratio) < 0)) {
                        line = __LINE__;
                        goto __ERR;
                    }
                }
                last_write_time = write_time;
                av_frame_unref(m_frame);
            }
            av_packet_unref(m_packet);
        }
        av_packet_free(&m_packet);
        av_frame_free(&m_frame);
    }
    goto __FREE;
    __ERR:
    {
        LOGE("__ERR__ERR__ERR__ERR__ERR__write_jpg(), line=%d, err=%d", line, err);
        goto __FREE;
    }
    __FREE:
    {
        avformat_close_input(&m_av_fmt_ctx);
        avformat_free_context(m_av_fmt_ctx);
        avcodec_free_context(&m_av_decoder_ctx);
        return err;
    }
}

int timeline::write_jpg(AVFrame *frame, const string &out_url, int ratio) {
    int line, err = -1;
    AVFormatContext *m_av_fmt_ctx = nullptr;
    AVCodecContext *m_av_encoder_ctx = nullptr;
    {//open avformat
        m_av_fmt_ctx = avformat_alloc_context();
        if ((err = avformat_alloc_output_context2(&m_av_fmt_ctx,
                                                  av_guess_format(nullptr,
                                                                  nullptr,
                                                                  "image/jpeg"),
                                                  nullptr,
                                                  out_url.c_str())) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if ((err = avio_open(&m_av_fmt_ctx->pb, out_url.c_str(), AVIO_FLAG_READ_WRITE)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
    }
    {//open avcodec
        AVCodec *av_encoder = avcodec_find_encoder(m_av_fmt_ctx->oformat->video_codec);
        if (av_encoder == nullptr) {
            line = __LINE__;
            goto __ERR;
        }
        AVCodecContext *av_encoder_ctx = avcodec_alloc_context3(av_encoder);
        if (av_encoder_ctx == nullptr) {
            line = __LINE__;
            goto __ERR;
        }
        av_encoder_ctx->time_base = (AVRational) {1, 25};
        AVStream *stream = avformat_new_stream(m_av_fmt_ctx, nullptr);
        AVCodecParameters *parameters = stream->codecpar;
        parameters->codec_id = m_av_fmt_ctx->oformat->video_codec;
        parameters->codec_type = AVMEDIA_TYPE_VIDEO;
        parameters->format = AV_PIX_FMT_YUVJ420P;
        parameters->width = frame->width / ratio;
        parameters->height = frame->height / ratio;
        if ((avcodec_parameters_to_context(av_encoder_ctx, stream->codecpar)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if ((err = avcodec_open2(av_encoder_ctx, av_encoder, nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        m_av_encoder_ctx = av_encoder_ctx;
    }
    {//write header
        if ((err = avformat_write_header(m_av_fmt_ctx, nullptr)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
    }
    {//write
        AVFrame *dst_frame = av_frame_alloc();
        dst_frame->format = m_av_encoder_ctx->pix_fmt;
        dst_frame->width = m_av_encoder_ctx->width;
        dst_frame->height = m_av_encoder_ctx->height;
        SwsContext *sws_ctx = sws_getContext(frame->width,
                                             frame->height,
                                             m_av_encoder_ctx->pix_fmt,
                                             dst_frame->width,
                                             dst_frame->height,
                                             AVPixelFormat(dst_frame->format),
                                             SWS_FAST_BILINEAR,
                                             nullptr,
                                             nullptr,
                                             nullptr);
        av_frame_get_buffer(dst_frame, 0);//malloc frame->buffer
        sws_scale(sws_ctx,
                  frame->data,
                  frame->linesize,
                  0,
                  frame->height,
                  dst_frame->data,
                  dst_frame->linesize);
        if ((err = av_frame_make_writable(dst_frame)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        if ((err = avcodec_send_frame(m_av_encoder_ctx, dst_frame)) < 0) {
            line = __LINE__;
            goto __ERR;
        }
        AVPacket *dst_packet = av_packet_alloc();
        if (avcodec_receive_packet(m_av_encoder_ctx, dst_packet) >= 0) {
            if ((err = av_write_frame(m_av_fmt_ctx, dst_packet)) < 0) {
                line = __LINE__;
                goto __ERR;
            }
        }
        av_packet_unref(dst_packet);
        av_packet_free(&dst_packet);
        av_frame_unref(dst_frame);
        av_frame_free(&dst_frame);
        av_free(sws_ctx);
    }
    {//write footer
        av_write_trailer(m_av_fmt_ctx);
    }
    goto __FREE;
    __ERR:
    {
        LOGE("__ERR__ERR__ERR__ERR__ERR__write_jpg(), line=%d, err=%d", line, err);
        goto __FREE;
    }
    __FREE:
    {
        if (m_av_fmt_ctx) {
            avformat_close_input(&m_av_fmt_ctx);
            avformat_free_context(m_av_fmt_ctx);
        }
        if (m_av_encoder_ctx != nullptr) {
            avcodec_free_context(&m_av_encoder_ctx);
        }
        return err;
    }
}
