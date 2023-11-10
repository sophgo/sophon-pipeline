//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_PIPELINE_COMMON_TYPES_H
#define SOPHON_PIPELINE_COMMON_TYPES_H



namespace bm {
    struct FeatureFrame {
        cv::Mat img;
        int chan_id;
        uint64_t seq;

        FeatureFrame():chan_id(0),seq(0) {


        }

        FeatureFrame(const struct FeatureFrame& rf)
        {
            img = rf.img;
            chan_id = rf.chan_id;
            seq = rf.seq;
        }

        FeatureFrame(struct FeatureFrame&& rf)
        {
            img = rf.img;
            chan_id = rf.chan_id;
            seq = rf.seq;
        }

        bm::FeatureFrame& operator =(const bm::FeatureFrame& rf)
        {
            img = rf.img;
            chan_id = rf.chan_id;
            seq = rf.seq;

            return *this;
        }

        bm::FeatureFrame& operator =(bm::FeatureFrame&& rf)
        {
            img = rf.img;
            chan_id = rf.chan_id;
            seq = rf.seq;

            return *this;
        }
    };

    struct FeatureFrameInfo {
        std::vector<FeatureFrame> frames;
        std::vector<bm_tensor_t> input_tensors;
        std::vector<bm_tensor_t> output_tensors;
        std::vector<bm::NetOutputDatum> out_datums;
    };

struct skipedFrameinfo{
    AVPacket *avpkt;
    AVFrame *avframe;
    bm::DataPtr img_data;
};
struct cvs10FrameBaseInfo {
    int chan_id;
    uint64_t seq;

    int64_t pkt_id;
    AVPacket *avpkt;
    AVFrame *avframe;
    bm::DataPtr jpeg_data;
    float x_offset = 0, y_offset = 0;
    float x_scale = 1, y_scale = 1;
    bm_image original, resized;
    int width, height, original_width, original_height;
    bool skip;
    std::queue<skipedFrameinfo> skip_frame_queue;

    cvs10FrameBaseInfo() : chan_id(0), seq(0), jpeg_data(nullptr), skip(false) {
        memset(&resized, 0, sizeof(bm_image));
        memset(&original, 0, sizeof(bm_image));
        avpkt = nullptr;
    }
};

struct cvs10FrameInfo {
    //AVFrame based
    std::vector<cvs10FrameBaseInfo> frames;
    std::vector<bm_tensor_t> input_tensors;
    std::vector<bm_tensor_t> output_tensors;
    std::vector<bm::NetOutputDatum> out_datums;
};

}







#endif //SOPHON_PIPELINE_COMMON_TYPES_H