
#ifndef BMNN_QTWIN_H
#define BMNN_QTWIN_H

#include "opencv2/opencv.hpp"
namespace bm {
    void imshow(const cv::String &winname, cv::InputArray _img);
    void waitkey(int delay);
}

#endif //!BMNN_QTWIN_H
