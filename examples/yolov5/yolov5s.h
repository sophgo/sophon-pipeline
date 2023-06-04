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

/*----------------------------------------*/
/*You should not change kernel api params.*/
#define MAX_YOLO_INPUT_NUM 8
#define MAX_YOLO_ANCHOR_NUM 8
typedef struct {
  unsigned long long bottom_addr[MAX_YOLO_INPUT_NUM];
  unsigned long long top_addr;
  unsigned long long detected_num_addr;
  int input_num;
  int batch_num;
  int hw_shape[MAX_YOLO_INPUT_NUM][2];
  int num_classes;
  int num_boxes;
  int keep_top_k;
  float nms_threshold;
  float confidence_threshold;
  float bias[MAX_YOLO_INPUT_NUM * MAX_YOLO_ANCHOR_NUM * 2];
  float anchor_scale[MAX_YOLO_INPUT_NUM];
  int clip_box;
}__attribute__((packed)) tpu_kernel_api_yolov5NMS_t;
/*----------------------------------------*/

class YoloV5 : public bm::DetectorDelegate<bm::FrameBaseInfo, bm::FrameInfo> 
             , public bm::BaseModel 
{
    int MAX_BATCH;
    bm::BMNNContextPtr m_bmctx;
    bm::BMNNNetworkPtr m_bmnet;
    int m_net_h, m_net_w;

    //configuration
    int m_class_num; 
    int m_model_type {0};
    std::vector<std::string> m_class_names;
    std::vector<std::vector<std::vector<float>>> m_anchors;

    std::string tpu_kernel_module_path;
#if USE_TPUKERNEL
    tpu_kernel_function_t func_id = -1;
#endif
public:
    YoloV5(bm::BMNNContextPtr bmctx, std::string tpu_kernel_module_path, std::string model_type);
    ~YoloV5();

    virtual int get_Batch();
    virtual int preprocess(std::vector<bm::FrameBaseInfo>& frames, std::vector<bm::FrameInfo>& frame_info) override ;
    virtual int forward(std::vector<bm::FrameInfo>& frame_info) override ;
    virtual int postprocess(std::vector<bm::FrameInfo> &frame_info) override;
private:
    int init_yolo(std::string model_type);
    float sigmoid(float x);
    int argmax(float* data, int dsize);
    static float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h, bool *alignWidth);
    void NMS(bm::NetOutputObjects &dets, float nmsConfidence);

    void extract_yolobox_cpu(bm::FrameInfo& frameInfo);
#if USE_TPUKERNEL
    void extract_yolobox_tpukernel(bm::FrameInfo& frameInfo);
#endif
};