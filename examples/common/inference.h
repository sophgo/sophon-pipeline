//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_PIPELINE_INFERENCE_H
#define SOPHON_PIPELINE_INFERENCE_H

#include "bmutility.h"
#include "thread_queue.h"
#define DO_QTGUI_IN_PREPROCESS 0

namespace bm {
    template<typename T1, typename T2>
    class DetectorDelegate {
    protected:
        using DetectedFinishFunc = std::function<void(T2 &of)>;
        DetectedFinishFunc m_pfnDetectFinish;
    public:
        virtual ~DetectorDelegate() {}

        virtual void decode_process(T1 &) {
            // do nothing by default
        }
        virtual int process_qtgui(std::vector<T1> &frames) = 0;
        
        virtual int preprocess(std::vector<T1> &frames, std::vector<T2> &of) = 0;

        virtual int forward(std::vector<T2> &frames, int core_id = 0) = 0;

        virtual int postprocess(std::vector<T2> &frames) = 0;

        virtual int set_detected_callback(DetectedFinishFunc func) { m_pfnDetectFinish = func; return 0;};

        virtual int get_max_batch() = 0;
    };

    struct DetectorParam {
        DetectorParam() {
            preprocess_queue_size = 5;
            preprocess_thread_num = 4;

            inference_queue_size = 5;
            inference_thread_num = 1;

            postprocess_queue_size = 5;
            postprocess_thread_num = 2;
            batch_num=4;
        }

        int preprocess_queue_size;
        int preprocess_thread_num;

        int inference_queue_size;
        int inference_thread_num;

        int postprocess_queue_size;
        int postprocess_thread_num;
        int batch_num;

    };

    template<typename T1, typename T2>
    class BMInferencePipe {
        DetectorParam m_param;
        std::shared_ptr<DetectorDelegate<T1, T2>> m_detect_delegate;
        std::string m_delegate_name;

        std::shared_ptr<BlockingQueue<T1>> m_qtprocessQue;
        std::shared_ptr<BlockingQueue<T1>> m_preprocessQue;
        std::shared_ptr<BlockingQueue<T2>> m_postprocessQue;
        std::shared_ptr<BlockingQueue<T2>> m_forwardQue;

        WorkerPool<T1> m_qtprocessWorkerPool;
        WorkerPool<T1> m_preprocessWorkerPool;
        WorkerPool<T2> m_forwardWorkerPool;
        WorkerPool<T2> m_postprocessWorkerPool;
    public:
        BMInferencePipe() {

        }

        virtual ~BMInferencePipe() {

        }

        int init(const DetectorParam &param, std::shared_ptr<DetectorDelegate<T1, T2>> delegate, std::string delegate_name) {
            m_param = param;
            m_detect_delegate = delegate;
            m_delegate_name = delegate_name;
            const int underlying_type_std_queue = 0;
            m_preprocessQue = std::make_shared<BlockingQueue<T1>>(
                delegate_name + "_preprocess", underlying_type_std_queue,
                param.preprocess_queue_size);
            m_postprocessQue = std::make_shared<BlockingQueue<T2>>(
                delegate_name + "_postprocess", underlying_type_std_queue,
                param.postprocess_queue_size);
            m_forwardQue = std::make_shared<BlockingQueue<T2>>(
                delegate_name + "_inference", underlying_type_std_queue,
                param.inference_queue_size);
        #if USE_QTGUI && !DO_QTGUI_IN_PREPROCESS
            if(delegate_name == "detector"){
                m_qtprocessQue = std::make_shared<BlockingQueue<T1>>(
                delegate_name + "_qtprocess", underlying_type_std_queue,
                param.preprocess_queue_size);

                m_qtprocessWorkerPool.init(m_qtprocessQue.get(), param.preprocess_thread_num, param.batch_num, param.batch_num);
                m_qtprocessWorkerPool.startWork([this, &param](std::vector<T1> &items, int thread_id) {
                    m_detect_delegate->process_qtgui(items);
                    this->m_preprocessQue->push(items);
                });
            }
        #endif

            m_preprocessWorkerPool.init(m_preprocessQue.get(), param.preprocess_thread_num, param.batch_num, param.batch_num);
            m_preprocessWorkerPool.startWork([this, &param](std::vector<T1> &items, int thread_id) {
                std::vector<T2> frames;
                m_detect_delegate->preprocess(items, frames);
                this->m_forwardQue->push(frames);
            });

            m_forwardWorkerPool.init(m_forwardQue.get(), param.inference_thread_num, param.batch_num, param.batch_num);
            m_forwardWorkerPool.startWork([this, &param](std::vector<T2> &items, int thread_id) {
                m_detect_delegate->forward(items, thread_id % 2); // for a2, 2 cores.
                this->m_postprocessQue->push(items);
            });

            m_postprocessWorkerPool.init(m_postprocessQue.get(), param.postprocess_thread_num, 1, param.batch_num);
            m_postprocessWorkerPool.startWork([this, &param](std::vector<T2> &items, int thread_id) {
                m_detect_delegate->postprocess(items);
            });
            return 0;
        }

        int flush_frame() {
            m_preprocessWorkerPool.flush();
            return 0;
        }

        int push_frame(T1 *frame) {
            if(m_delegate_name == "detector"){
            #if DO_QTGUI_IN_PREPROCESS || !USE_QTGUI
                m_preprocessQue->push(*frame);
            #else
                m_qtprocessQue->push(*frame);
            #endif
            } else{
                m_preprocessQue->push(*frame);
            }
            return 0;
        }
    };
} // end namespace bm


#endif //SOPHON_PIPELINE_INFERENCE_H
