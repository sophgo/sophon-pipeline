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
#include "configuration_cvs.h"
#include "bmutility_timer.h"
#include <iomanip>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <cstdio>
#include "face_extract.h"
#include "resnet50.h"

enum ModelType {
    MODEL_FACE_DETECT=0,
    MODEL_RESNET50=1
};

#ifndef WITH_DETECTOR
#define WITH_DETECTOR 1
#endif
#ifndef WITH_EXTRACTOR
#define WITH_EXTRACTOR 1
#endif
#ifndef WITH_OUTPUTER
#define WITH_OUTPUTER 1
#endif

void flush_console(int x, int y, int w, int h){
    int startX = x; // 清除区域的左上角 x 坐标
    int startY = y;  // 清除区域的左上角 y 坐标
    int width = w;  // 清除区域的宽度
    int height = h; // 清除区域的高度
    // 使用 ANSI 转义序列清除指定区域
    for (int y = startY; y < startY + height; ++y) {
        for (int x = startX; x < startX + width; ++x) {
            printf("\033[%d;%dH ", y, x); // 将指定位置的字符设置为空格
        }
    }
}

double get_cpu_usage() {
    std::ifstream statFile("/proc/stat");
    if (!statFile) {
        std::cerr << "Failed to open /proc/stat." << std::endl;
        return 0.0;
    }

    std::string line;
    std::getline(statFile, line); // 读取第一行，即总体 CPU 统计信息

    std::istringstream iss(line);
    std::string cpuLabel;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

    iss >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    unsigned long long totalTime = user + nice + system + idle + iowait + irq + softirq + steal;
    unsigned long long totalIdle = idle + iowait;

    // 等待一段时间以获取下一个时间戳
    sleep(1); // 可根据需要调整时间间隔

    statFile.seekg(std::ios_base::beg); // 将文件指针返回开头，准备读取下一个时间戳的数据
    std::getline(statFile, line); // 再次读取第一行，即新的 CPU 统计信息

    std::istringstream iss2(line);
    std::string cpuLabel2;
    unsigned long long user2, nice2, system2, idle2, iowait2, irq2, softirq2, steal2, guest2, guest_nice2;

    iss2 >> cpuLabel2 >> user2 >> nice2 >> system2 >> idle2 >> iowait2 >> irq2 >> softirq2 >> steal2 >> guest2 >> guest_nice2;

    unsigned long long totalTime2 = user2 + nice2 + system2 + idle2 + iowait2 + irq2 + softirq2 + steal2;
    unsigned long long totalIdle2 = idle2 + iowait2;

    // 计算 CPU 使用率
    double usagePercentage = static_cast<double>(totalTime2 - totalTime) / (totalTime2 + totalIdle2 - totalTime - totalIdle) * 100.0;
    return usagePercentage;
    // return 0;
}

double get_mem_usage(long long& totalMemory, long long& freeMemory, long long &usedMemory) {
    struct sysinfo memInfo;
    if (sysinfo(&memInfo) == -1) {
        std::cerr << "Failed to get memory info." << std::endl;
        return 0.0;
    }

    totalMemory = memInfo.totalram;
    freeMemory = memInfo.freeram;
    usedMemory = totalMemory - freeMemory;

    double usagePercentage = (static_cast<double>(usedMemory) / static_cast<double>(totalMemory)) * 100.0;
    return usagePercentage;
}

float cal_avg_usage(int* usage_arr, int num){//vpu vpp jpu
    float avg_usage = 0;
    for(int i = 0; i < num; i++){
        if(usage_arr[i] == 0 && i > 0){
            avg_usage /= (float) i;
            break;
        } else{
            avg_usage += usage_arr[i];
        }
    }
    return avg_usage;
}

int main(int argc, char *argv[])
{
    const char *base_keys="{help | 0 | Print help information.}"
                          "{model_type | 0 | Model Type(0: face_detect 1: resnet50)}"
                          "{feat_delay | 500 | feature delay in msec}"
                          "{feat_num | 10 | feature num per channel}"
                          "{num | 1 | number of channel to infer}"
                          "{skip_frame_num | 1 | skip N frames to detect}"
                          "{display_num | 1 | display number of channel in QT}"
                          "{resize_num | 0 | resize number of channel in hardware emulation}"
                          "{stop_frame_num | 1 | frame number early stop}"
                          "{save_num | 0 | number of channel to save .flv}"
                          "{output | None | Output stream URL}"
                          "{config | ./cameras_cvs.json | path to cameras_cvs.json}";

    std::string keys;
    keys = base_keys;
    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.get<bool>("help")) {
        parser.printMessage();
        return 0;
    }
    if (access("results", 0) != F_OK){
        mkdir("results", S_IRWXU);
    }
    std::string output_url  = parser.get<std::string>("output");
    std::string config_file = parser.get<std::string>("config");

    int model_type = parser.get<int>("model_type");
    int feature_delay = parser.get<int>("feat_delay");
    int feature_num = parser.get<int>("feat_num");
    int num = parser.get<int>("num");
    int skip_frame_num = parser.get<int>("skip_frame_num");
    int stop_frame_num = parser.get<int>("stop_frame_num");
    int save_num = parser.get<int>("save_num");
    int display_num = parser.get<int>("display_num");
    int resize_num = parser.get<int>("resize_num");

    int enable_l2_ddrr = 0;

    Config cfg(config_file.c_str());
    if (!cfg.valid_check()) {
        std::cout << "ERROR:cameras.json config error, please check!" << std::endl;
        return -1;
    }

    int total_num = num;
    AppStatis appStatis(total_num);

    int card_num = cfg.cardNums();
    int channel_num_per_card = total_num/card_num;
    int last_channel_num = total_num % card_num == 0 ? 0:total_num % card_num;

    std::shared_ptr<bm::VideoUIApp> gui;
#if USE_QTGUI
    if (display_num > 0){
        gui = bm::VideoUIApp::create(argc, argv);
        gui->bootUI(total_num);
    }
#endif

    bm::TimerQueuePtr tqp = bm::TimerQueue::create();
    int start_chan_index = 0;
    std::vector<OneCardInferAppPtr> apps;
    std::thread profile_thread[card_num];
    for(int card_idx = 0; card_idx < card_num; ++card_idx) {
        int dev_id = cfg.cardDevId(card_idx);

        // load balance
        int channel_num = 0;
        if (card_idx < last_channel_num) {
            channel_num = channel_num_per_card + 1;
        }else{
            channel_num = channel_num_per_card;
        }

        bm::BMNNHandlePtr handle = std::make_shared<bm::BMNNHandle>(dev_id);
        bm::BMNNContextPtr contextPtr = std::make_shared<bm::BMNNContext>(handle, cfg.get_model_path());

        std::cout << "start_chan_index=" << start_chan_index << ", channel_num=" << channel_num << std::endl;
        OneCardInferAppPtr appPtr = std::make_shared<OneCardInferApp>(appStatis, gui,
                tqp, contextPtr, output_url, start_chan_index, channel_num, skip_frame_num, feature_delay, feature_num,
                enable_l2_ddrr, stop_frame_num, save_num, display_num);
        start_chan_index += channel_num;
    #if WITH_DETECTOR
        std::shared_ptr<bm::DetectorDelegate<bm::cvs10FrameBaseInfo, bm::cvs10FrameInfo>> detector;
        if (MODEL_FACE_DETECT == model_type) {
            detector = std::make_shared<FaceDetector>(contextPtr, resize_num);
        }else if (MODEL_RESNET50 == model_type) {
            detector = std::make_shared<Resnet>(contextPtr);
        }
        // set detector delegator
        appPtr->setDetectorDelegate(detector);
    #endif
    #if WITH_EXTRACTOR    
        std::shared_ptr<bm::DetectorDelegate<bm::FeatureFrame, bm::FeatureFrameInfo>> feature_delegate;
        feature_delegate = std::make_shared<FaceExtract>(contextPtr);
        appPtr->setFeatureDelegate(feature_delegate);
    #endif
        appPtr->start(cfg.cardUrls(card_idx), cfg);
        apps.push_back(appPtr);
    #if WITH_PROFILE
        profile_thread[card_idx] = std::thread([&](){
            bm_handle_t dev_handle;
            bm_dev_request(&dev_handle, dev_id);
            bm_misc_info misc_info;
            bm_get_misc_info(dev_handle, &misc_info);
            int profile_cnt = 0;
            while(true){
                bm_dev_stat_t bm_dev_stat;
                bm_get_stat(dev_handle, &bm_dev_stat);
                flush_console(0, dev_id * 10, 150, 12);
                printf("\033\033[H");
                printf("\033[%d;%dH", dev_id*10, 0); 
                printf("=====Card %d, bitmask: %ld, profile %d======\n", dev_id, misc_info.chipid_bit_mask, profile_cnt++);
                printf("tpu_util: %d\%\n", bm_dev_stat.tpu_util);
                printf("mem_used/mem_total: %d/%d, usage: %f\n", bm_dev_stat.mem_used, bm_dev_stat.mem_total, (float)bm_dev_stat.mem_used/(float)bm_dev_stat.mem_total);
                for(int hi = 0; hi < bm_dev_stat.heap_num; hi++){
                    bm_heap_stat& heap = bm_dev_stat.heap_stat[hi];
                    printf("heap %d, mem_used/mem_total: %d/%d, usage: %f\n", hi, heap.mem_used, heap.mem_total, (float)heap.mem_used/(float)heap.mem_total);
                }
                int vpu_usage[6], jpu_usage[5], vpp_usage[5] = {0};
                bm_get_vpu_instant_usage(dev_handle, vpu_usage);
                bm_get_jpu_core_usage(dev_handle, jpu_usage);
                bm_get_vpp_instant_usage(dev_handle, vpp_usage);
                printf("vpu_usage: %d, %d, %d, %d, %d, avg: %f\%\n", vpu_usage[0], vpu_usage[1], vpu_usage[2], vpu_usage[3], vpu_usage[4], cal_avg_usage(vpu_usage, 6));
                printf("jpu_usage: %d, %d, %d, %d, avg: %f\%\n", jpu_usage[0], jpu_usage[1], jpu_usage[2], jpu_usage[3], cal_avg_usage(jpu_usage, 5));
                printf("vpp_usage: %d, %d, %d, %d, avg: %f\%\n", vpp_usage[0], vpp_usage[1], vpp_usage[2], vpp_usage[3], cal_avg_usage(vpp_usage, 5));
                printf("==========================\n\n\n");
                // fflush(stdout);
                usleep(100000);
            }
        });
    #endif
    }

#if PLD
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
#else
    uint64_t timer_id;
    tqp->create_timer(1000, [&appStatis, &card_num](){
        int ch = 0;
        appStatis.m_chan_det_fpsPtr->update(appStatis.m_chan_statis[ch]);
        appStatis.m_total_det_fpsPtr->update(appStatis.m_total_statis);

        appStatis.m_chan_feat_fpsPtr->update(appStatis.m_chan_feat_stat[ch]);
        appStatis.m_total_feat_fpsPtr->update(appStatis.m_total_feat_stat);

        double chanfps = appStatis.m_chan_det_fpsPtr->getSpeed();
        double totalfps = appStatis.m_total_det_fpsPtr->getSpeed();

        double feat_chanfps = appStatis.m_chan_feat_fpsPtr->getSpeed();
        double feat_totalfps = appStatis.m_total_feat_fpsPtr->getSpeed();
    #if WITH_PROFILE
        printf("\033\033[H");
        printf("\033[%d;%dH", 80, 0); 
        double cpu_usage = get_cpu_usage();
        printf("==========================\n");
        printf("[cpu_usage: %lf]          \n", cpu_usage);
        long long total_mem, free_mem, used_mem;
        double mem_usage = get_mem_usage(total_mem, free_mem, used_mem);
        printf("[mem_usage, mem_used/mem_total: %lld/%lld, usage: %lf]   \n", used_mem, total_mem, mem_usage);
        printf("[%s] det ([SUCCESS: %d/%d] total fps = %.1f, ch = %d: speed = %.1f) feature ([SUCCESS: %d/%d] total fps = %.1f, ch = %d: speed = %.1f)\n",
                bm::timeToString(time(0)).c_str(),
                appStatis.m_total_statis, appStatis.m_total_decode,
                totalfps, ch, chanfps,
                appStatis.m_total_feat_stat, appStatis.m_total_feat_decode,
                feat_totalfps, ch, feat_chanfps);
        printf("==========================\n");
    #endif
        std::cout << "[" << bm::timeToString(time(0)) << "] det ([SUCCESS: "
        << appStatis.m_total_statis << "/" << appStatis.m_total_decode << "]total fps ="
        << std::setiosflags(std::ios::fixed) << std::setprecision(1) << totalfps
        <<  ",ch=" << ch << ": speed=" << chanfps
        << ") feature ([SUCCESS: " << appStatis.m_total_feat_stat << "/" << appStatis.m_total_feat_decode
        << "]total fps=" << std::setiosflags(std::ios::fixed) << std::setprecision(1)
        << feat_totalfps <<  ",ch=" << ch << ": speed=" << feat_chanfps << ")" << std::endl;
    }, 1, &timer_id);

    tqp->run_loop();
#endif
    return 0;
}
