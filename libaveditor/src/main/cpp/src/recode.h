//
// Created by 86193 on 2023/3/24.
//

#ifndef AVEDITOR_RECODE_H
#define AVEDITOR_RECODE_H

#include "common.h"

class recode {
public:
    int recode_codec(const std::string &,
                     const std::string &);

private:
    AVFormatContext *m_inAVFormatContext;
    AVCodec *m_inAVCodec;

    AVFormatContext *m_outAVFormatContext;

    AVPacket *m_packet;
    AVFrame *m_frame;

    void recode_free();
};

#endif //AVEDITOR_RECODE_H
