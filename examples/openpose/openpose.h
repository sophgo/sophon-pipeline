//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//


#ifndef SOPHON_PIPELINE_OPENPOSE_H
#define SOPHON_PIPELINE_OPENPOSE_H

#include "bmutility.h"
#include "inference.h"


class OpenPose : public bm::DetectorDelegate<bm::FrameBaseInfo, bm::FrameInfo> {
    int MAX_BATCH = 1;
    bm::BMNNContextPtr m_bmctx;
    bm::BMNNNetworkPtr m_bmnet;
    int m_net_h, m_net_w;
    std::function<void(bm::FrameInfo &infos)> m_detect_finish_func;

    //configuration
    bool m_use_custom_scale {false};
    float m_input_scale;
    float m_output_scale;
    float m_nms_threshold;
    bm::PoseKeyPoints::EModelType m_model_type;

public:
    OpenPose(bm::BMNNContextPtr bmctx, std::string strModelType = "coco", float nms_threshold=0.05);
    void setParams(bool useCustomScale, float customInputScale, float customOutputScale);

    ~OpenPose();

    virtual int preprocess(std::vector <bm::FrameBaseInfo> &frames, std::vector <bm::FrameInfo> &frame_info) override;

    virtual int forward(std::vector <bm::FrameInfo> &frame_info) override;

    virtual int postprocess(std::vector <bm::FrameInfo> &frame_info) override;

private:
    bm::BMNNTensorPtr get_output_tensor(const std::string &name, bm::FrameInfo& frame_info, float scale=1.0);
    void decode_from_output_tensor(bm::FrameInfo& frame_info);

};


#endif 
