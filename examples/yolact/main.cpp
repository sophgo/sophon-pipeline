//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "opencv2/opencv.hpp"
#include "worker.h"
#include "configuration.h"
#include "bmutility_timer.h"
#include <iomanip>



int main(int argc, char *argv[])
{
    const char *base_keys=
                         "{help | 0 | Print help information.}"
                         "{config | ./cameras_yolact.json | path to cameras_yolact.json}";

    std::string keys;
    keys = base_keys;
    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.get<bool>("help")) {
        parser.printMessage();
        return 0;
    }

    std::string config_file = parser.get<std::string>("config");

    Config cfg(config_file.c_str());

    int total_num = cfg.totalChanNums();
    AppStatis appStatis(total_num);

    int card_num = cfg.cardNums();
    int channel_num_per_card = total_num/card_num;
    int last_channel_num = total_num % card_num == 0 ? 0:total_num % card_num;

    std::shared_ptr<bm::VideoUIApp> gui;
#if USE_QTGUI
    gui = bm::VideoUIApp::create(argc, argv);
    gui->bootUI(total_num);
#endif

    bm::TimerQueuePtr tqp = bm::TimerQueue::create();
    int start_chan_index = 0;
    std::vector<OneCardInferAppPtr> apps;

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

        std::shared_ptr<YOLACT> detector = std::make_shared<YOLACT>(contextPtr, model_cfg.model_type);
        // model thresholds
        detector->set_obj(model_cfg.obj_threshold);
        detector->set_nms(model_cfg.nms_threshold);
        OneCardInferAppPtr appPtr = std::make_shared<OneCardInferApp>(appStatis, gui,
                tqp, contextPtr, model_cfg.output_path, start_chan_index, channel_num, model_cfg.skip_frame, detector->get_Batch());

        start_chan_index += channel_num;
        // set detector delegator
        appPtr->setDetectorDelegate(detector);
        appPtr->start(cfg.cardUrls(card_idx), cfg);
        apps.push_back(appPtr);
    }

    uint64_t timer_id;
    tqp->create_timer(1000, [&appStatis](){
        int ch = 0;
        appStatis.m_stat_imgps->update(appStatis.m_chan_statis[ch]);
        appStatis.m_total_fpsPtr->update(appStatis.m_total_statis);

        double imgfps = appStatis.m_stat_imgps->getSpeed();
        double totalfps = appStatis.m_total_fpsPtr->getSpeed();

        std::cout << "[" << bm::timeToString(time(0)) << "] total fps ="
        << std::setiosflags(std::ios::fixed) << std::setprecision(1) << totalfps
        <<  ",ch=" << ch << ": speed=" << imgfps << std::endl;
    }, 1, &timer_id);

    tqp->run_loop();

    return 0;
}
