//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//


#include "opencv2/opencv.hpp"
#include <sys/time.h>
#include "bmutility.h"
#include "inference.h"
#include "bmutility_basemodel.hpp"


class YoloV5 : public bm::DetectorDelegate<bm::FrameBaseInfo, bm::FrameInfo>
             , public bm::BaseModel 
{
    bm::BMNNContextPtr m_bmctx;
    bm::BMNNNetworkPtr m_bmnet;
    int m_net_h, m_net_w;

    //configuration
    int m_class_num = 80; // default is coco names
    //const float m_anchors[3][6] = {{10.0, 13.0, 16.0, 30.0, 33.0, 23.0}, {30.0, 61.0, 62.0, 45.0, 59.0, 119.0},{116.0, 90.0, 156.0, 198.0, 373.0, 326.0}};
    std::vector<std::vector<std::vector<int>>> m_anchors{{{10, 13}, {16, 30}, {33, 23}},
                                                         {{30, 61}, {62, 45}, {59, 119}},
                                                         {{116, 90}, {156, 198}, {373, 326}}};
    const int m_anchor_num = 3;

public:
    YoloV5(bm::BMNNContextPtr bmctx, int class_num=80);
    ~YoloV5();

    virtual int preprocess(std::vector<bm::FrameBaseInfo>& frames, std::vector<bm::FrameInfo>& frame_info) override ;
    virtual int forward(std::vector<bm::FrameInfo>& frame_info) override;
    virtual int postprocess(std::vector<bm::FrameInfo> &frame_info) override;
private:
    float sigmoid(float x);
    int argmax(float* data, int dsize);
    static float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h, bool *alignWidth);
    void NMS(bm::NetOutputObjects &dets, float nmsConfidence);

    void extract_yolobox_cpu(bm::FrameInfo& frameInfo);
};