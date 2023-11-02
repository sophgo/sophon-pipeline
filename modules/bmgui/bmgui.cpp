//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "bmgui.h"
#include <QtWidgets>
#include "thread_queue.h"
#include "mainwindow.h"
#include "video_widget.h"
#include "chrono"
#define MAX_CHAN_NUM 16
namespace bm {
    class VideoUIAppQT: public VideoUIApp {
        int m_argc;
        char **m_argv;
        QApplication *m_appInst{nullptr};
        std::shared_ptr <std::thread> m_pUIThread;
        mainwindow *m_pMainWindow;
        BlockingQueue <UIFrame> m_frameQue[MAX_CHAN_NUM];
        std::shared_ptr <std::thread> m_pFrameDispatchThread[MAX_CHAN_NUM];
        int m_channel_num = 0;
        float flow_control_interval = 40;

        void uithread_entry(int num) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
            QApplication app(m_argc, m_argv);
            m_appInst = &app;
            mainwindow w;
            m_pMainWindow = &w;
            w.createWidgets(num);
            w.show();
            for(int i = 0; i < num; i++){
                m_pFrameDispatchThread[i] = std::make_shared<std::thread>(&VideoUIAppQT::frame_dispatch_entry, this, i);
                assert(m_pFrameDispatchThread[i] != nullptr);
            }

            app.exec();
            m_appInst = nullptr;
            for(int i = 0; i < MAX_CHAN_NUM; i++){
                m_frameQue[i].stop();
            }
            std::cout << "UI thread exit!" << std::endl;
        }

        void frame_dispatch_entry(int chan_id) {
            #if FLOW_CONTROL
                auto pre_ts = std::chrono::high_resolution_clock::now();
            #endif
            while (m_appInst != nullptr) {
                std::vector <UIFrame> frames;
                m_frameQue[chan_id].pop_front(frames, 1, 16);
                for (auto &it : frames) {
                    assert(chan_id == it.chan_id);
                    video_widget *pWnd = m_pMainWindow->videoWidget(it.chan_id);
                #if FLOW_CONTROL // todo: control for each channel
                    auto current_ts = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> interval = current_ts - pre_ts;
                    double interval_ms = 1000 * interval.count();
                    pre_ts = current_ts;
                    if((flow_control_interval - interval_ms) > 0.1){
                        // std::cout<<"sleep "<< flow_control_interval - interval_ms <<std::endl;
                        bm::msleep(flow_control_interval - interval_ms);
                    }
                #endif
                    if (pWnd) {
                        if (it.jpeg_data) {
                            pWnd->GetVideoHwnd()->draw_frame(it.jpeg_data, it.datum, it.h, it.w);
                        }else if (it.avframe != nullptr) {
                            pWnd->GetVideoHwnd()->draw_frame(it.avframe);
                            av_frame_unref(it.avframe);
                            av_frame_free(&it.avframe);
                        }else{
                            pWnd->GetVideoHwnd()->draw_info(it.datum, it.h, it.w);
                        }
                    
                    }
                }
            }

            std::cout << "frame dispatch thread " << chan_id << " exit!" << std::endl;
        }

    public:
        VideoUIAppQT(int argc, char *argv[]) : m_argc(argc), m_argv(argv),
                                             m_appInst(nullptr), m_pUIThread(nullptr) {

        }

        ~VideoUIAppQT() {
            std::cout << "Waiting for UI shutdown ..." << std::endl;
            m_pUIThread->join();
            std::cout << "UI thread shutdown successfully." << std::endl;
            std::cout << "VideoUIApp exit!" << std::endl;
        }

        int bootUI(int num, int gui_delay) {
            if(num > MAX_CHAN_NUM){
                std::cerr << "Please adjust macro MAX_CHAN_NUM!" << std::endl;
                exit(1);
            }
            m_channel_num = num;
            m_pUIThread = std::make_shared<std::thread>(&VideoUIAppQT::uithread_entry, this, num);
            assert(m_pUIThread != nullptr);
            flow_control_interval = gui_delay;
            return 0;
        }

        int shutdownUI() {
            return 0;
        }

        int pushFrame(UIFrame &frame) {
            m_frameQue[frame.chan_id].push(frame);
            return 0;
        }

    };

    std::shared_ptr<VideoUIApp> VideoUIApp::create(int argc, char *argv[])
    {
        return std::make_shared<VideoUIAppQT>(argc, argv);
    }
}