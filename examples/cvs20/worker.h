//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_PIPELINE_WORKER_H
#define SOPHON_PIPELINE_WORKER_H
#include "bmutility.h"
#include "bmgui.h"
#include "inference.h"
#include "stream_pusher.h"
#include "configuration_cvs.h"
#include "face_detector.h"
// #include "bm_tracker.h"
#include "common_types.h"
#include "ff_video_encode.h"
#ifndef WITH_DECODE
#define WITH_DECODE 1
#endif
#ifndef WITH_DETECTOR
#define WITH_DETECTOR 1
#endif
#ifndef WITH_EXTRACTOR
#define WITH_EXTRACTOR 1
#endif
#ifndef WITH_OUTPUTER
#define WITH_OUTPUTER 1
#endif
#define DECODE_TIMER 0

struct TChannel: public bm::NoCopyable {
    int channel_id;
    uint64_t seq;
    bm::StreamDemuxer *demuxer;
    bm::FfmpegOutputer *outputer;
    // std::shared_ptr<bm::BMTracker> tracker;
    uint64_t m_last_feature_time=0; // last do feature time
    VideoEnc_FFMPEG writer;
#if DECODE_TIMER
    int decoder_timer_count = 0;
    double this_chan_total_secs_send = 0;
    double this_chan_total_secs_receive = 0;
#endif

    int64_t ref_pkt_id = -1;
    AVCodecContext* m_decoder=nullptr;

    TChannel():channel_id(0), seq(0), demuxer(nullptr) {

        //  tracker = bm::BMTracker::create();
         m_last_feature_time = 0;
         outputer = nullptr;
    }

    ~TChannel() {
        if (demuxer) delete demuxer;
        if (m_decoder) {
            avcodec_close(m_decoder);
            avcodec_free_context(&m_decoder);
        }
        if (writer.is_opened){
            writer.closeEnc();
        }
        std::cout << "TChannel(chan_id=" << channel_id << ") dtor" <<std::endl;
    }

    int create_video_decoder(int dev_id, AVFormatContext *ifmt_ctx) {
        int video_index = 0;

#if LIBAVCODEC_VERSION_MAJOR > 56
        auto codec_id = ifmt_ctx->streams[video_index]->codecpar->codec_id;
#else
        auto codec_id = ifmt_ctx->streams[video_index]->codec->codec_id;
#endif

        const AVCodec *pCodec = avcodec_find_decoder(codec_id);
        if (NULL == pCodec) {
            printf("can't find code_id %d\n", codec_id);
            return -1;
        }

        m_decoder = avcodec_alloc_context3(pCodec);
        if (m_decoder == NULL) {
            printf("avcodec_alloc_context3 err");
            return -1;
        }

        int ret = 0;

#if LIBAVCODEC_VERSION_MAJOR > 56
        if ((ret = avcodec_parameters_to_context(m_decoder, ifmt_ctx->streams[video_index]->codecpar)) < 0) {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
            return ret;
        }
#else
        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_copy_context(m_dec_ctx, ifmt_ctx->streams[video_index]->codec)) < 0) {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
            return ret;
        }
#endif

        // if (pCodec->capabilities & AV_CODEC_CAP_TRUNCATED) {
        //     m_decoder->flags |= AV_CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
        // }

        //for PCIE
        AVDictionary* opts = NULL;
        av_dict_set_int(&opts, "sophon_idx", dev_id, 0x0);
        av_dict_set(&opts, "extra_frame_buffer_num", "6", 0); //6 37%，12 41%
    #if WITH_ENCODE_H264
        av_dict_set(&opts, "cbcr_interleave", "0", 0);
        av_dict_set(&opts, "output_format", "0", 0);
    #else
        av_dict_set(&opts, "output_format", "101", 0);
    #endif
    #if PLD
        std::cout<<"opening decoder!"<<std::endl;
    #endif
        if (avcodec_open2(m_decoder, pCodec, &opts) < 0) {
            std::cout << "Unable to open codec";
            return -1;
        }
    #if PLD
        std::cout<<"open decoder success!"<<std::endl;
    #endif
        return 0;
    }

    int decode_video2(AVCodecContext* dec_ctx, AVFrame *frame, int *got_picture, AVPacket* pkt)
    {
        int ret;
        *got_picture = 0;
    #if DECODE_TIMER
        decoder_timer_count++;
        auto start_send = std::chrono::high_resolution_clock::now();
    #endif
        ret = avcodec_send_packet(dec_ctx, pkt);
        
    #if DECODE_TIMER
        auto end_send = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_send = end_send - start_send;
        double seconds_send = 1000 * elapsed_send.count();
        this_chan_total_secs_send += seconds_send;
        if((decoder_timer_count + 1)% 25 == 0){
            std::cout << "channel_id: "<< channel_id << "; avg send time: " << this_chan_total_secs_send / 25 << " ms;";
            this_chan_total_secs_send = 0;
        }
    #endif

        if (ret == AVERROR_EOF) {
            ret = 0;
        }
        else if (ret < 0) {
            char err[256] = {0};
            av_strerror(ret, err, sizeof(err));
            fprintf(stderr, "Error sending a packet for decoding, %s\n", err);
            return -1;
        }

        while (ret >= 0) {
    #if DECODE_TIMER
        auto start_receive = std::chrono::high_resolution_clock::now();
    #endif
            ret = avcodec_receive_frame(dec_ctx, frame);
    #if DECODE_TIMER
        auto end_receive = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_receive = end_receive - start_receive;
        double seconds_receive = 1000 * elapsed_receive.count();
        this_chan_total_secs_receive += seconds_receive;
        if((decoder_timer_count + 1) % 25 == 0){
            std::cout << " avg receive time: " << this_chan_total_secs_receive/25 << " ms" << std::endl;
            this_chan_total_secs_receive = 0;
            decoder_timer_count = 0;
        }
    #endif
            if (ret == AVERROR(EAGAIN)) {
# if USE_DEBUG
                printf("need more data!\n");
# endif
                ret = 0;
                break;
            }else if (ret == AVERROR_EOF) {
                printf("File end!\n");
                avcodec_flush_buffers(dec_ctx);
                ret = 0;
                break;
            }
            else if (ret < 0) {
                fprintf(stderr, "Error during decoding\n");
                break;
            }
            *got_picture += 1;
            static int fff = 0;
            // std::cout<<"cout: decoded_format:"<<frame->format << ", times: " << fff++ <<std::endl;
            break;
        }

        if (*got_picture > 1) {
            printf("got picture %d\n", *got_picture);
        }

        return ret;
    }
};

using TChannelPtr = std::shared_ptr<TChannel>;

class OneCardInferApp {
    bm::VideoUIAppPtr m_guiReceiver;
    AppStatis &m_appStatis;
    std::shared_ptr<bm::DetectorDelegate<bm::cvs10FrameBaseInfo, bm::cvs10FrameInfo>> m_detectorDelegate;
    std::shared_ptr<bm::DetectorDelegate<bm::FeatureFrame, bm::FeatureFrameInfo>> m_featureDelegate;
    bm::BMNNContextPtr m_bmctx;
    bm::TimerQueuePtr m_timeQueue;
    int m_channel_start;
    int m_channel_num;
    int m_dev_id;
    int m_skipN;
    std::string m_output_url;
    int m_feature_delay;
    int m_feature_num;
    int m_use_l2_ddrr= 0;
    int m_frame_count = 0;
    int m_stop_frame_num;
    int m_save_num;
    int m_display_num;
    int gui_resize_h = 360;
    int gui_resize_w = 640;

    FILE *outputFile;
    bm::BMInferencePipe<bm::cvs10FrameBaseInfo, bm::cvs10FrameInfo> m_inferPipe;
    //skip frame queue regather
    std::queue<bm::skipedFrameinfo> m_skipframe_queue[32]; //todo: set size by channel_num;

    bm::BMInferencePipe<bm::FeatureFrame, bm::FeatureFrameInfo> m_featurePipe;

    std::map<int, TChannelPtr> m_chans;
    std::vector<std::string> m_urls;
public:
    OneCardInferApp(AppStatis& statis,bm::VideoUIAppPtr& gui, bm::TimerQueuePtr tq, bm::BMNNContextPtr ctx, std::string& output_url, 
            int start_index, int num, int skip=0, int feat_delay=1000, int feat_num=8,
            int use_l2_ddrr=0, int stop_frame_num=0, int save_num=0, int display_num=1): m_detectorDelegate(nullptr), m_channel_num(num),
            m_bmctx(ctx), m_appStatis(statis),m_use_l2_ddrr(use_l2_ddrr), m_stop_frame_num(stop_frame_num), m_save_num(save_num), m_display_num(display_num)
    {
        m_guiReceiver = gui;
        m_dev_id = m_bmctx->dev_id();
        m_timeQueue = tq;
        m_channel_start = start_index;
        m_skipN = skip;
        m_feature_delay = feat_delay;
        m_feature_num = feat_num;
        m_output_url = output_url;

    }

    ~OneCardInferApp()
    {
        if (outputFile){
            fclose(outputFile);
        }
        std::cout << cv::format("OneCardInfoApp (devid=%d) dtor", m_dev_id) <<std::endl;
    }

    void setDetectorDelegate(std::shared_ptr<bm::DetectorDelegate<bm::cvs10FrameBaseInfo, bm::cvs10FrameInfo>> delegate){
        m_detectorDelegate = delegate;
    }

    void setFeatureDelegate(std::shared_ptr<bm::DetectorDelegate<bm::FeatureFrame, bm::FeatureFrameInfo>> delegate){
        m_featureDelegate = delegate;
    }

    void start(const std::vector<std::string>& vct_urls, Config& config);

    inline void loadConfig(bm::DetectorParam& param, Config& config) {
        SConcurrencyConfig cfg;
        if (config.get_phrase_config("preprocess", cfg)){
            param.preprocess_thread_num    = cfg.thread_num;
            param.preprocess_queue_size    = cfg.queue_size;
        }
        if (config.get_phrase_config("inference", cfg)){
            param.inference_thread_num    = cfg.thread_num;
            param.inference_queue_size    = cfg.queue_size;
        }
        if (config.get_phrase_config("postprocess", cfg)){
            param.postprocess_thread_num    = cfg.thread_num;
            param.postprocess_queue_size    = cfg.queue_size;
        }
    }

    void set_gui_resize_hw(int h, int w){
        gui_resize_h = h;
        gui_resize_w = w;
    }
};

using OneCardInferAppPtr = std::shared_ptr<OneCardInferApp>;


#endif //SOPHON_PIPELINE_MAIN_H
