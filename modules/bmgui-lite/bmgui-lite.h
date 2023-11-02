//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef BMNN_QTWIN_H
#define BMNN_QTWIN_H

#include "opencv2/opencv.hpp"
namespace bm {
    void imshow(const cv::String &winname, cv::InputArray _img);
    void waitkey(int delay);
}

#endif //!BMNN_QTWIN_H
