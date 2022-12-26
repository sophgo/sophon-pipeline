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
namespace bm {

    // declare before
    template<typename T> class BMInferencePipe;

    template<typename T>
    class DetectorDelegate {
    protected:
        using DetectedFinishFunc = std::function<void(T &of)>;
        DetectedFinishFunc m_pfnDetectFinish = nullptr;
        BMInferencePipe<T>* m_nextInferPipe = nullptr;
    public:
        virtual ~DetectorDelegate() {}
        void set_next_inference_pipe(BMInferencePipe<T> *nextPipe) { m_nextInferPipe = nextPipe; }
        int set_detected_callback(DetectedFinishFunc func) { m_pfnDetectFinish = func; return 0; }

        virtual int preprocess(std::vector<T> &frames) = 0;

        virtual int forward(std::vector<T> &frames) = 0;

        virtual int postprocess(std::vector<T> &frames) = 0;

    };

    struct DetectorParam {
        DetectorParam() {
            preprocess_queue_size = 5;
            preprocess_thread_num = 4;

            inference_queue_size = 5;
            inference_thread_num = 1;

            postprocess_queue_size = 5;
            postprocess_thread_num = 2;
            batch_size = 4;
        }

        int preprocess_queue_size;
        int preprocess_thread_num;

        int inference_queue_size;
        int inference_thread_num;

        int postprocess_queue_size;
        int postprocess_thread_num;

        int batch_size;
    };

    template<typename T>
    class BMInferencePipe {
        DetectorParam m_param;
        std::shared_ptr<DetectorDelegate<T>> m_detect_delegate;

        std::shared_ptr<BlockingQueue<T>> m_preprocessQue;
        std::shared_ptr<BlockingQueue<T>> m_postprocessQue;
        std::shared_ptr<BlockingQueue<T>> m_forwardQue;

        WorkerPool<T> m_preprocessWorkerPool;
        WorkerPool<T> m_forwardWorkerPool;
        WorkerPool<T> m_postprocessWorkerPool;


    public:
        BMInferencePipe() {

        }

        virtual ~BMInferencePipe() {

        }

        int init(const DetectorParam &param, std::shared_ptr<DetectorDelegate<T>> delegate) {
            m_param = param;
            m_detect_delegate = delegate;

            const int underlying_type_std_queue = 0;
            m_preprocessQue = std::make_shared<BlockingQueue<T>>(
                "preprocess", underlying_type_std_queue,
                param.preprocess_queue_size);
            m_postprocessQue = std::make_shared<BlockingQueue<T>>(
                "postprocess", underlying_type_std_queue,
                param.postprocess_queue_size);
            m_forwardQue = std::make_shared<BlockingQueue<T>>(
                "inference", underlying_type_std_queue,
                param.inference_queue_size);

            m_preprocessWorkerPool.init(m_preprocessQue.get(), param.preprocess_thread_num, param.batch_size, param.batch_size);
            m_preprocessWorkerPool.startWork([this, &param](std::vector<T> &items) {
                m_detect_delegate->preprocess(items);
                this->m_forwardQue->push(items);
            });

            m_forwardWorkerPool.init(m_forwardQue.get(), param.inference_thread_num, 1, 8);
            m_forwardWorkerPool.startWork([this, &param](std::vector<T> &items) {
                m_detect_delegate->forward(items);
                this->m_postprocessQue->push(items);
            });

            m_postprocessWorkerPool.init(m_postprocessQue.get(), param.postprocess_thread_num, 1, 8);
            m_postprocessWorkerPool.startWork([this, &param](std::vector<T> &items) {
                m_detect_delegate->postprocess(items);
            });
            return 0;
        }

        int flush_frame() {
            return m_preprocessWorkerPool.flush();
        }

        // int push_frame(T *frame) {
        //     return m_preprocessQue.push(*frame);
        // }
        int push_frame(T *frame) {
            m_preprocessQue->push(*frame);
            return 0;
        }
    };
} // end namespace bm


#endif 
