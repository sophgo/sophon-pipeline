//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_PIPELINE_FACE_EXTRACT_H
#define SOPHON_PIPELINE_FACE_EXTRACT_H


#include "bmutility.h"
#include "bmutility_types.h"
#include "inference.h"
#include "common_types.h"

class FaceExtract : public bm::DetectorDelegate<bm::FeatureFrame, bm::FeatureFrameInfo>  {
    bm::BMNNContextPtr m_bmctx;
    bm::BMNNNetworkPtr m_bmnet;
    float m_alpha_fp32;
    float m_beta_fp32;
    float m_alpha_int8;
    float m_beta_int8;
    int MAX_BATCH=1;
    int m_net_h;
    int m_net_w;

public:
    FaceExtract(bm::BMNNContextPtr bmctx);
    ~FaceExtract();

    virtual int preprocess(std::vector<bm::FeatureFrame> &in, std::vector<bm::FeatureFrameInfo> &of) override;
    virtual int forward(std::vector<bm::FeatureFrameInfo> &frames) override;
    virtual int postprocess(std::vector<bm::FeatureFrameInfo> &frames) override;
private:
    bm::BMNNTensorPtr get_output_tensor(const std::string &name, bm::FeatureFrameInfo& frame_info, float scale);
    void extract_facefeature_cpu(bm::FeatureFrameInfo& frame);
};


#endif //SOPHON_PIPELINE_FACE_EXTRACT_H
