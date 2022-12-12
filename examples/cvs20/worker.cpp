//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "worker.h"
#include "stream_sei.h"

void OneCardInferApp::start(const std::vector<std::string>& urls, Config& config)
{
    bool enable_outputer = false;
    if (bm::start_with(m_output_url, "rtsp://") || bm::start_with(m_output_url, "udp://") ||
        bm::start_with(m_output_url, "tcp://")) {
        enable_outputer = true;
    }
    enable_outputer = true;
    m_detectorDelegate->set_detected_callback([this, enable_outputer](bm::cvs10FrameInfo &frameInfo) {
        for (int frame_idx = 0; frame_idx < frameInfo.frames.size(); ++frame_idx) {
            int ch = frameInfo.frames[frame_idx].chan_id;

            m_appStatis.m_chan_statis[ch]++;
            m_appStatis.m_statis_lock.lock();
            m_appStatis.m_total_statis++;
            m_appStatis.m_statis_lock.unlock();
#if USE_DEBUG
            std::ostringstream oss;
            oss << " detect " << frameInfo.out_datums[frame_idx].obj_rects.size() << " objs.";
            for (int i = 0; i < frameInfo.out_datums[frame_idx].obj_rects.size(); i++) {
                oss << " object " << i << ": ["
                    << " x1:" << frameInfo.out_datums[frame_idx].obj_rects[i].x1
                    << " y1:" << frameInfo.out_datums[frame_idx].obj_rects[i].y1
                    << " x2:" << frameInfo.out_datums[frame_idx].obj_rects[i].x2
                    << " y2:" << frameInfo.out_datums[frame_idx].obj_rects[i].y2
                    << "]";
            }
            std::cout << "[" << bm::timeToString(time(0)) << "] ch:" << ch
                << oss.str() << std::endl;
#endif

            // tracker
            if (frameInfo.out_datums[frame_idx].obj_rects.size() > 0) {
                m_chans[ch]->tracker->update(frameInfo.out_datums[frame_idx].obj_rects, frameInfo.out_datums[frame_idx].track_rects);
            }
            // display
#if USE_QTGUI
            if (ch < m_display_num){
                bm::UIFrame jpgframe;
                jpgframe.jpeg_data = frameInfo.frames[frame_idx].jpeg_data;
                jpgframe.chan_id = ch;
                jpgframe.h = frameInfo.frames[frame_idx].height;
                jpgframe.w = frameInfo.frames[frame_idx].width;
                jpgframe.datum = frameInfo.out_datums[frame_idx];
                m_guiReceiver->pushFrame(jpgframe);
            }
            
#endif
            // save
            if (enable_outputer && ((ch < m_save_num) || (m_save_num == -1))) {

                std::shared_ptr<bm::ByteBuffer> buf = frameInfo.out_datums[frame_idx].toByteBuffer();
                std::string base64_str = bm::base64_enc(buf->data(), buf->size());

                AVPacket sei_pkt;
                av_init_packet(&sei_pkt);
                AVPacket *pkt1 = frameInfo.frames[frame_idx].avpkt;
                av_packet_copy_props(&sei_pkt, pkt1);
                sei_pkt.stream_index = pkt1->stream_index;

                //AVCodecID codec_id = m_chans[ch]->decoder->get_video_codec_id();

                if (true) {// (codec_id == AV_CODEC_ID_H264) {
                    int packet_size = h264sei_calc_packet_size(base64_str.length());
                    AVBufferRef *buf = av_buffer_alloc(packet_size << 1);
                    //assert(packet_size < 16384);
                    int real_size = h264sei_packet_write(buf->data, true, (uint8_t *) base64_str.data(),
                                                         base64_str.length());
                    sei_pkt.data = buf->data;
                    sei_pkt.size = real_size;
                    sei_pkt.buf = buf;

                } 

                m_chans[ch]->outputer->InputPacket(&sei_pkt);
                m_chans[ch]->outputer->InputPacket(frameInfo.frames[frame_idx].avpkt);
                av_packet_unref(&sei_pkt);

                m_frame_count += 1;
                if (m_frame_count >= m_stop_frame_num){
                    std::cout <<  "-=-=-=-======exit==============>>> " << m_frame_count << std::endl;
                    m_chans[ch]->outputer->CloseOutputStream();
                    exit(-1);
                }
                
            }
        }
    });

    // feature register
    m_featureDelegate->set_detected_callback([this](bm::FeatureFrameInfo &frameInfo) {
        for (int i = 0; i < frameInfo.frames.size(); ++i) {
            int ch = frameInfo.frames[i].chan_id;

            m_appStatis.m_chan_feat_stat[ch]++;
            m_appStatis.m_total_feat_stat++;
        }
    });

    //detector
    bm::DetectorParam param;
    int cpu_num = std::thread::hardware_concurrency();
    int tpu_num = 1;
    param.preprocess_thread_num = cpu_num;
    param.preprocess_queue_size = std::max(m_channel_num, 8);
    param.inference_thread_num = tpu_num;
    param.inference_queue_size = m_channel_num;
    param.postprocess_thread_num = cpu_num;
    param.postprocess_queue_size = m_channel_num;

    loadConfig(param, config);

    m_inferPipe.init(param, m_detectorDelegate);

    //feature
    m_featurePipe.init(param, m_featureDelegate);

    for(int i = 0; i < m_channel_num; ++i) {
        int ch = m_channel_start + i;

        TChannelPtr pchan = std::make_shared<TChannel>();
        pchan->demuxer = new bm::StreamDemuxer(ch);
        if (enable_outputer) pchan->outputer = new bm::FfmpegOutputer();
        pchan->channel_id = ch;

        std::string media_file;
        AVDictionary *opts = NULL;
        av_dict_set_int(&opts, "sophon_idx", m_dev_id, 0);
        av_dict_set(&opts, "output_format", "101", 18);
        av_dict_set(&opts, "extra_frame_buffer_num", "5", 0);

        pchan->demuxer->set_avformat_opend_callback([this, pchan](AVFormatContext *ifmt) {
            pchan->create_video_decoder(m_dev_id, ifmt);
            // todo create DDR reduction for optimization

            if (pchan->outputer && pchan->channel_id < m_save_num) {
                // std::string connect = "_";
                // std::string prefix_url = "cvs10_save";
                // std::string postfix_url = ".flv";
                // std::string url = bm::format("%s%s%d%s", prefix_url.c_str(), connect.c_str(), pchan->channel_id, postfix_url.c_str()); // "/home/frotms/hdd/liuchenxi/repo/cvs20/test_save/sophon-pipeline/release/cvs10/sav/cvs10_save.flv";
                
                std::string url;
                if (bm::start_with(m_output_url, "rtsp://")) {
                    std::string connect = "_";
                    url = bm::format("%s%s%d", m_output_url.c_str(), connect.c_str(), pchan->channel_id);
                }
                else if (bm::start_with(m_output_url, "udp://") || bm::start_with(m_output_url, "tcp://")){
                    size_t pos = m_output_url.rfind(":");
                    std::string base_url = m_output_url.substr(0, pos);
                    int base_port = std::strtol(m_output_url.substr(pos + 1).c_str(), 0, 10);
                    url = bm::format("%s:%d", base_url.c_str(), base_port + pchan->channel_id);
                }
                else if (bm::end_with(m_output_url, ".flv")){
                    std::string connect = "_";
                    std::string suffix_url = ".flv";
                    size_t pos = m_output_url.rfind(suffix_url);
                    std::string base_url = m_output_url.substr(0, pos);
                    int base_port = std::strtol(m_output_url.substr(pos + 1).c_str(), 0, 10);
                    url = bm::format("%s%s%d%s", base_url.c_str(), connect.c_str(), pchan->channel_id, suffix_url.c_str());;
                }
                else{
                    size_t pos = m_output_url.rfind(":");
                    std::string base_url = m_output_url.substr(0, pos);
                    int base_port = std::strtol(m_output_url.substr(pos + 1).c_str(), 0, 10);
                    url = bm::format("%s:%d", base_url.c_str(), base_port + pchan->channel_id);
                }
                pchan->outputer->OpenOutputStream(url, ifmt);
            }

        });

        pchan->demuxer->set_avformat_closed_callback([this, pchan]() {
            if (pchan->outputer) pchan->outputer->CloseOutputStream();
        });

        pchan->demuxer->set_read_Frame_callback([this, pchan, ch](AVPacket* pkt){
            int ret = 0;
            //not use ddr reduction
            int got_picture = 0;
            AVFrame *frame = av_frame_alloc();
            pchan->decode_video2(pchan->m_decoder, frame, &got_picture, pkt);
            if (got_picture) {
                bm::cvs10FrameBaseInfo fbi;
                fbi.seq = pchan->seq++;
                fbi.chan_id = ch;
                fbi.pkt_id = 0;

                if (m_skipN > 0) {
                    if (fbi.seq % (m_skipN + 1) != 0) fbi.skip = true;
                }

#if 0
                if (ch == 0) std::cout << " seq = " << fbi.seq << " skip= " << fbi.skip << std::endl;
#endif
                if (!fbi.skip) {
                    fbi.avframe = av_frame_alloc();
                    fbi.avpkt = av_packet_alloc();
                    av_frame_ref(fbi.avframe, frame);
                    av_packet_ref(fbi.avpkt, pkt);
                    m_appStatis.m_statis_lock.lock();
                    m_appStatis.m_total_decode++;
                    m_appStatis.m_statis_lock.unlock();
                    m_inferPipe.push_frame(&fbi);
                }
            }

            av_frame_free(&frame);

            uint64_t current_time = bm::gettime_msec();
            if (current_time - m_chans[ch]->m_last_feature_time > m_feature_delay) {
                for(int feat_idx = 0; feat_idx < m_feature_num; feat_idx++) {
                    bm::FeatureFrame featureFrame;
                    featureFrame.chan_id = ch;
                    featureFrame.seq++;
#if USE_SOPHON_OPENCV
                    featureFrame.img = cv::imread("face.jpeg", cv::IMREAD_COLOR, m_dev_id);
#else
                    featureFrame.img = cv::imread("face.jpeg", cv::IMREAD_COLOR);
#endif
                    if (featureFrame.img.empty()) {
                        printf("ERROR:Can't find face.jpg in workdir!\n");
                        exit(0);
                    }
                    m_appStatis.m_statis_lock.lock();
                    m_appStatis.m_total_feat_decode++;
                    m_appStatis.m_statis_lock.unlock();
                    m_featurePipe.push_frame(&featureFrame);
                }

                m_chans[ch]->m_last_feature_time = current_time;
            }


        });

        pchan->demuxer->open_stream(urls[i % urls.size()], nullptr, true, opts);
        av_dict_free(&opts);
        m_chans[ch] = pchan;
    }
}
