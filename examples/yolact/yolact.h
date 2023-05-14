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
#include "opencv2/opencv.hpp"


class YOLACT : public bm::DetectorDelegate<bm::FrameBaseInfo, bm::FrameInfo> 
             , public bm::BaseModel 
{
    int MAX_BATCH;
    bm::BMNNContextPtr m_bmctx;
    bm::BMNNNetworkPtr m_bmnet;
    int m_net_h, m_net_w;

    //configuration
    int m_class_num; 
    int m_model_type {0};
    int m_keep_top_k {100};
    std::vector<std::string> m_class_names;
    std::vector<std::vector<std::vector<float>>> m_anchors;

    std::vector <float> m_alpha;
    std::vector <float> m_beta;

    std::vector<int> m_conv_ws;
    std::vector<int> m_conv_hs;
    std::vector<float> m_aspect_ratios;
    std::vector<float> m_scales;
    std::vector<float> m_variances;

    int m_num_priors;
    int m_num_scales;
    int m_num_aspect_ratios;
    std::vector<float> m_priorbox;

public:
    YOLACT(bm::BMNNContextPtr bmctx, std::string model_type);
    ~YOLACT();

    virtual int get_Batch();
    virtual int preprocess(std::vector<bm::FrameBaseInfo>& frames, std::vector<bm::FrameInfo>& frame_info) override ;
    virtual int forward(std::vector<bm::FrameInfo>& frame_info) override ;
    virtual int postprocess(std::vector<bm::FrameInfo> &frame_info) override;
private:
    int init_yolo(std::string model_type);

    void extract_yolobox_cpu(bm::FrameInfo& frameInfo);

    void make_priors();

    void qsort_descent_inplace(std::vector<bm::NetOutputObject>& datas, std::vector<std::vector<float>> masks);

    void qsort_descent_inplace(
        std::vector<bm::NetOutputObject>& datas, std::vector<std::vector<float>>& masks, int left, int right
    );

    inline float intersection_area(const bm::NetOutputObject& a, const bm::NetOutputObject& b);

    void nms_sorted_bboxes(const std::vector<bm::NetOutputObject>& objects, std::vector<int>& picked, 
        float nms_threshold, bool agnostic);
    void sigmoid(cv::Mat& out, int length);
};