//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_PIPELINE_RESNET50_H
#define SOPHON_PIPELINE_RESNET50_H

#include "inference.h"
extern "C" {
    #include "bmcv_api_ext.h"
}
#include "common_types.h"

class Resnet : public bm::DetectorDelegate<bm::cvs10FrameBaseInfo, bm::cvs10FrameInfo>  {
    bm::BMNNContextPtr m_bmctx;
    bm::BMNNNetworkPtr m_bmnet;

    float m_alpha;
    float m_beta;

    int MAX_BATCH=1;
    int m_net_h;
    int m_net_w;

public:
    Resnet(bm::BMNNContextPtr bmctx);
    ~Resnet();

    virtual int process_qtgui(std::vector<bm::cvs10FrameBaseInfo>& frames) override{return 0;};
    virtual int preprocess(std::vector<bm::cvs10FrameBaseInfo> &in, std::vector<bm::cvs10FrameInfo> &of) override;
    virtual int forward(std::vector<bm::cvs10FrameInfo> &frames) override;
    virtual int postprocess(std::vector<bm::cvs10FrameInfo> &frames) override;
    virtual int get_max_batch() override{
        return MAX_BATCH;
    };
private:
    bm::BMNNTensorPtr get_output_tensor(const std::string &name, bm::cvs10FrameInfo& frame_info, float scale);
    void extract_feature_cpu(bm::cvs10FrameInfo& frame);
};


#endif //SOPHON_PIPELINE_RESNET50_H
