//
// Created by 86193 on 2023/3/24.
//
#include "recode.h"

int recode::recode_codec(const std::string &in_url, const std::string &out_url) {
    m_inAVFormatContext = avformat_alloc_context();
    if (avformat_open_input(&m_inAVFormatContext,
                            in_url.c_str(),
                            nullptr,
                            nullptr) < 0) {
        goto __ERR;
    }
    if (avformat_find_stream_info(m_inAVFormatContext, nullptr) < 0) {
        goto __ERR;
    }
    m_outAVFormatContext = avformat_alloc_context();
    if (avformat_alloc_output_context2(&m_outAVFormatContext,
                                       nullptr,
                                       nullptr,
                                       in_url.c_str()) < 0) {
        goto __ERR;
    }
    for (size_t i = 0; i < m_inAVFormatContext->nb_streams; i++) {

    }


    m_packet = av_packet_alloc();
    while (av_read_frame(m_inAVFormatContext, m_packet) >= 0) {

    }
    recode_free();
    return 0;
    __ERR:
    {
        LOGE("__ERR__ERR__ERR__ERR__ERR__recode_codec");
        recode_free();
        return -1;
    }
}

void recode::recode_free() {
    if (m_frame) {
        av_frame_free(&m_frame);
    }
    if (m_packet) {
        av_packet_free(&m_packet);
    }
    if (m_inAVFormatContext) {
        avformat_free_context(m_inAVFormatContext);
    }
    if (m_outAVFormatContext) {
        avformat_free_context(m_outAVFormatContext);
    }
}
