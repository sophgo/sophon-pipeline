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
#include "bmutility_string.h"
#include "inference.h"
#include "bmutility_basemodel.hpp"


class PPYoloe : public bm::DetectorDelegate<bm::FrameBaseInfo, bm::FrameInfo> 
             , public bm::BaseModel 
{
    int MAX_BATCH;
    bm::BMNNContextPtr m_bmctx;
    bm::BMNNNetworkPtr m_bmnet;
    int m_net_h, m_net_w;

    //configuration
    int m_class_num; 
    std::vector <int> m_outputs_order;
    std::vector <float> m_alpha;
    std::vector <float> m_beta;

public:
    PPYoloe(bm::BMNNContextPtr bmctx, std::string model_type);
    ~PPYoloe();

    virtual int get_Batch();
    virtual int preprocess(std::vector<bm::FrameBaseInfo>& frames, std::vector<bm::FrameInfo>& frame_info) override ;
    virtual int forward(std::vector<bm::FrameInfo>& frame_info) override ;
    virtual int postprocess(std::vector<bm::FrameInfo> &frame_info) override;
private:
    int init_yolo(std::string model_type);
    void NMS(bm::NetOutputObjects &dets, float nmsConfidence);

    void extract_yolobox_cpu(bm::FrameInfo& frameInfo);
};