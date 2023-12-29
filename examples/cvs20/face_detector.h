//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_PIPELINE_FACE_DETECTOR_H
#define SOPHON_PIPELINE_FACE_DETECTOR_H

#include "inference.h"
extern "C" {
    #include "bmcv_api_ext.h"
}
#include "common_types.h"
#include "bmutility_basemodel.hpp"

#define MAX_INT 2147483600

#ifndef USE_MMAP
#define USE_MMAP 1
#endif

#define USE_D2S !USE_MMAP

#ifndef PREPROCESS_TIMER
#define PREPROCESS_TIMER 1
#endif

class FaceDetector : public bm::DetectorDelegate<bm::cvs10FrameBaseInfo, bm::cvs10FrameInfo> 
                   , public bm::BaseModel 
{
    bm::BMNNContextPtr bmctx_;
    bm::BMNNNetworkPtr bmnet_;
    bool               is4N_;

    double             target_size_{360};
    double             max_size_ {640};
    double             im_scale_;

    // for squeezenet, not yolov5
    float              nms_threshold_{0.3};
    float              base_threshold_{0.5};
    
    std::vector<float> anchor_ratios_;
    std::vector<float> anchor_scales_;
    int                per_nms_topn_{1000};
    int                base_size_{16};
    int                min_size_{2};
    int                feat_stride_{8};
    int                anchor_num_;
    double             img_x_scale_;
    double             img_y_scale_;
    double             img_qt_x_scale_ = 1;
    double             img_qt_y_scale_ = 1;
    int  m_net_h, m_net_w;
    int m_output_num = 0;
    int MAX_BATCH = 1;
    int resize_num_;
    int m_display_num;
    int gui_resize_h = 360;
    int gui_resize_w = 640;
    int input_frame_width = 1920;
    int input_frame_height = 1080;
#if PREPROCESS_TIMER
    int preprocess_timer_count[32] = {0};
    double preprocess_api_time[32] = {0};
    std::mutex preprocess_timer_lock[32];
#endif

public:
    FaceDetector(bm::BMNNContextPtr bmctx, int resize_num, int display_num, int gui_resize_h_, int gui_resize_w_);
    ~FaceDetector();
    virtual int process_qtgui(std::vector<bm::cvs10FrameBaseInfo>& frames) override ;
    virtual int preprocess(std::vector<bm::cvs10FrameBaseInfo>& frames, std::vector<bm::cvs10FrameInfo>& frame_info) override ;
    virtual int forward(std::vector<bm::cvs10FrameInfo>& frame_info, int core_id=0) override ;
    virtual int postprocess(std::vector<bm::cvs10FrameInfo> &frame_info) override;
    virtual int get_max_batch() override{
        return MAX_BATCH;
    };
private:
    int extract_facebox_cpu(bm::cvs10FrameInfo &frame_info);
    bm::BMNNTensorPtr get_output_tensor(const std::string &name, bm::cvs10FrameInfo& frame_info, float scale);
    void generate_proposal(const float *          scores,
                           const float *          bbox_deltas,
                           const float            scale_factor,
                           const int              feat_factor,
                           const int              feat_w,
                           const int              feat_h,
                           const int              width,
                           const int              height,
                           bm::NetOutputObjects &proposals);
    void nms(const bm::NetOutputObjects &proposals,
             bm::NetOutputObjects&      nmsProposals);
    void calc_resized_HW(int image_h, int image_w, int *p_h, int *p_w);
    void get_data_qtgui(bm_image &image1, bm::DataPtr& img_data);

    int extract_facebox_cpu_yolov5(bm::cvs10FrameInfo &frame_info);
    void NMS_yolov5(bm::NetOutputObjects &dets, float nmsConfidence);

};


#endif //SOPHON_PIPELINE_FACE_DETECTOR_H
