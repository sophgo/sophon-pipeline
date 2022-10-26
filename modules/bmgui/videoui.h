//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef VIDEOUI_VIDEOUI_H
#define VIDEOUI_VIDEOUI_H

#include <iostream>

class VideoUI {
public:
    std::shared<VideoUI> create(int num);
    ~VideoUI() {
        std::cout << "VideoUI() dtor" << std::endl;
    }

    virtual int push_frame(int chan, AVFrame *frame) = 0;
    virtual int push_frame(int chan, cv::Mat& img) = 0;
};

using std::shared<VideoUI> VideoUIPtr;


#endif //VIDEOUI_VIDEOUI_H
