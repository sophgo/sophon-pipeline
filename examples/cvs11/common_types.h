
#ifndef SOPHON_PIPELINE_COMMON_TYPES_H
#define SOPHON_PIPELINE_COMMON_TYPES_H

#include "ddr_reduction.h"

#define call(fn, ...) \
    do { \
        auto ret = fn(__VA_ARGS__); \
        if (ret != BM_SUCCESS) \
        { \
            std::cout << "[ERROR] " << #fn << " failed " << ret << std::endl; \
            throw std::runtime_error("api error"); \
        } \
    } while (false);

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

struct cvs11FrameBaseInfo {
    int chan_id;
    uint64_t seq;
    std::shared_ptr<DDRReduction> ddrr;
    int64_t pkt_id;
    //AVPacket *avpkt;
    AVFrame *avframe;
    bm::DataPtr jpeg_data;
    float x_offset = 0, y_offset = 0;
    float x_scale = 1, y_scale = 1;
    bm_image original, resized;
    int width, height, original_width, original_height;
    bool skip;

    cvs11FrameBaseInfo() : chan_id(0), seq(0), jpeg_data(nullptr), skip(false) {
        memset(&resized, 0, sizeof(bm_image));
        memset(&original, 0, sizeof(bm_image));
    }
};

struct cvs11FrameInfo {
    //AVFrame based
    std::vector<cvs11FrameBaseInfo> frames;
    std::vector<bm_tensor_t> input_tensors;
    std::vector<bm_tensor_t> output_tensors;
    std::vector<bm::NetOutputDatum> out_datums;
};

}







#endif //SOPHON_PIPELINE_COMMON_TYPES_H
