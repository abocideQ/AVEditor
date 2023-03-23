//
// Created by 86193 on 2023/3/21.
//
#ifndef AVEDITOR_REMIX_TIMESTAMP_H
#define AVEDITOR_REMIX_TIMESTAMP_H

#include <map>
#include <string>
#include "common.h"

class remix_timestamp {

public:
    std::map<long, long> get_stamp(const std::string &);

    int remix_stamp(const std::string &,
                    const std::string &,
                    std::map<long, long> *);

private:
    AVFormatContext *m_inAVFormatContext;
    AVFormatContext *m_outAVFormatContext;
    AVPacket *m_packet;
    int m_stream_index = 0;
    int m_stream_mapping_size = 0;
    int *m_stream_mapping = NULL;

    void remix_free();
};

#endif //AVEDITOR_REMIX_TIMESTAMP_H
