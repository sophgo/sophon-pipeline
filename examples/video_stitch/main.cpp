//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include <iomanip>
#include "opencv2/opencv.hpp"
#include "worker.h"
#include "configuration.h"
#include "bmutility_timer.h"
#include "rtsp/Live555RtspServer.h"
#include "encoder.h"
#include "stitch.h"



int main(int argc, char *argv[])
{
    const char *base_keys="{help | 0 | Print help information.}"
                         "{config | ./cameras_video_stitch.json | path to cameras_video_stitch.json}";

    std::string keys;
    keys = base_keys;
    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.get<bool>("help")) {
        parser.printMessage();
        return 0;
    }

    std::string config_file = parser.get<std::string>("config");
    Config cfg(config_file.c_str());
    
    int total_num = 4;
    std::cout <<"total_num=" <<total_num << std::endl;
    
    if (total_num != 4) {
        std::cerr << "Only support 2x2 layout, make the num be equal to 4!!";
        return -1;
    }
    
    if (!cfg.valid_check()) {
        std::cout << "ERROR:cameras.json config error, please check!" << std::endl;
        return -1;
    }
    
    
    int card_num = cfg.cardNums();
    int channel_num_per_card = total_num/card_num;
    int last_channel_num = total_num % card_num == 0 ? 0:total_num % card_num;
    
    std::shared_ptr<bm::VideoUIApp> gui;

    bm::TimerQueuePtr tqp = bm::TimerQueue::create();
    int start_chan_index = 0;
    std::vector<OneCardInferAppPtr> apps;

    // live555 RTSP Server
    CreateRtspServer();
    BTRTSPServer* pRtspServer = GetRTSPInstance();

    // Only statistics encoder fps
    AppStatis appStatis(1);
    int dev_id_ = cfg.cardDevId(0); 
    std::set<std::string> distinct_ = cfg.getDistinctModels(0);
    std::string model_name = (*distinct_.begin());
    auto modelConfig = cfg.getModelConfig();
    auto& model_cfg = modelConfig[model_name];

    std::shared_ptr<CVEncoder>       encoder = std::make_shared<CVEncoder>(25 / (model_cfg.skip_frame + 1), 1920, 1080, dev_id_, pRtspServer, &appStatis);
    std::shared_ptr<VideoStitchImpl> stitch  = std::make_shared<VideoStitchImpl>(0, total_num, encoder);
    bm::BMMediaPipeline<bm::FrameBaseInfo, bm::FrameInfo> m_media_pipeline;
    bm::MediaParam param;
    param.draw_thread_num = 1;
    param.draw_queue_size = 8;
    param.stitch_thread_num = 1;
    param.stitch_queue_size = 8;
    param.encode_thread_num = 1;
    param.encode_queue_size = 8;

    m_media_pipeline.init(
        param, stitch,
        bm::FrameInfo::FrameInfoDestroyFn,
        bm::FrameInfo::FrameInfoDestroyFn,
        bm::FrameBaseInfo::FrameBaseInfoDestroyFn
    );

    for(int card_idx = 0; card_idx < card_num; ++card_idx) {
        int dev_id = cfg.cardDevId(card_idx);

         std::set<std::string> distinct_models = cfg.getDistinctModels(card_idx);
         std::string model_name = (*distinct_models.begin());
         auto modelConfig = cfg.getModelConfig();
         auto& model_cfg = modelConfig[model_name];

        // load balance
        int channel_num = 0;
        if (card_idx < last_channel_num) {
            channel_num = channel_num_per_card + 1;
        }else{
            channel_num = channel_num_per_card;
        }

        bm::BMNNHandlePtr handle = std::make_shared<bm::BMNNHandle>(dev_id);
        bm::BMNNContextPtr contextPtr = std::make_shared<bm::BMNNContext>(handle, model_cfg.path);
        bmlib_log_set_level(BMLIB_LOG_VERBOSE);

        if (card_idx == card_num - 1) {
            stitch->setHandle(contextPtr->handle());
        }

        std::shared_ptr<YoloV5> detector = std::make_shared<YoloV5>(contextPtr, start_chan_index, channel_num, model_cfg.class_num);
        // model thresholds
        detector->set_cls(model_cfg.class_threshold);
        detector->set_obj(model_cfg.obj_threshold);
        detector->set_nms(model_cfg.nms_threshold);
        detector->set_next_inference_pipe(&m_media_pipeline);

        OneCardInferAppPtr appPtr = std::make_shared<OneCardInferApp>(
                tqp, contextPtr, start_chan_index, channel_num, handle->handle(), model_cfg.skip_frame, detector->get_Batch());
        start_chan_index += channel_num;
        
        // set detector delegator
        appPtr->setDetectorDelegate(detector);
        appPtr->start(cfg.cardUrls(card_idx), cfg);
        apps.push_back(appPtr);
    }

    uint64_t timer_id;
    tqp->create_timer(1000, [&appStatis](){
        appStatis.m_total_fpsPtr->update(appStatis.m_total_statis);
        double totalfps = appStatis.m_total_fpsPtr->getSpeed();
        std::cout << "[" << bm::timeToString(time(0)) << "] encode fps ="
        << std::setiosflags(std::ios::fixed) << std::setprecision(1) << totalfps << std::endl;
    }, 1, &timer_id);

    tqp->run_loop();

    return 0;
}
