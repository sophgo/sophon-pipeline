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
#include "inference_serial.h"
#include "bmcv_api_ext.h"
#include "face_common.h"

class FaceExtract : public bm::DetectorDelegate<bm::FrameInfo2> {
    bm::BMNNContextPtr m_bmctx;
    bm::BMNNNetworkPtr m_bmnet;
    float m_alpha_fp32;
    float m_beta_fp32;
    float m_alpha_int8;
    float m_beta_int8;
    int MAX_BATCH=1;
    cv::Size m_inputSize;

public:
    FaceExtract(bm::BMNNContextPtr bmctx);
    ~FaceExtract();

    virtual int preprocess(std::vector<bm::FrameInfo2> &in) override;
    virtual int forward(std::vector<bm::FrameInfo2> &frames) override;
    virtual int postprocess(std::vector<bm::FrameInfo2> &frames) override;
private:
    bm::BMNNTensorPtr get_output_tensor(const std::string &name, bm::NetForward *inferIO, float scale=1.0);
    void extract_facefeature_cpu(bm::FrameInfo2& frame);
    int get_complex_idx(int idx, std::vector<bm::NetOutputDatum> out, int *p_frameIdx, int *prc_idx);
    void free_fwds(std::vector<bm::NetForward> &fwds);
};


#endif 
