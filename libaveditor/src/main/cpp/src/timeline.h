//
// Created by 86193 on 2023/4/3.
//

#ifndef AVEDITOR_TIMELINE_H
#define AVEDITOR_TIMELINE_H

#include "common.h"

using namespace std;

class timeline {
public:
    static vector<long double> get_time_stamps(const string &);

    static int write_time_line_jpg(const string &, const string &, int);

private:
    static int write_jpg(AVFrame *frame, const string &, int);
};


#endif //AVEDITOR_TIMELINE_H
