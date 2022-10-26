//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_PIPELINE_BMGUI_H
#define SOPHON_PIPELINE_BMGUI_H

#include <thread>
#include "bmutility_types.h"
#include "opencv2/opencv.hpp"
#ifdef __cplusplus
extern "C" {
#include "libavcodec/avcodec.h"
}
#endif

namespace bm {
    struct UIFrame {
        int chan_id;
        bm::DataPtr jpeg_data;
        AVFrame *avframe {nullptr};
        int h, w;
        NetOutputDatum datum;
    };

    class VideoUIApp {
    public:
        static std::shared_ptr<VideoUIApp> create(int argc, char *argv[]);

        ~VideoUIApp() {
            std::cout << "VideoUIApp exit!" << std::endl;
        }

        virtual int bootUI(int window_num=1) = 0;
        virtual int shutdownUI() = 0;
        virtual int pushFrame(UIFrame &frame) = 0;
    };

    using VideoUIAppPtr = std::shared_ptr<VideoUIApp>;
}



#endif //SOPHON_PIPELINE_BMGUI_H
