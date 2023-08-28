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
#if WITH_OUTPUTER
    if (bm::start_with(m_output_url, "rtsp://") || bm::start_with(m_output_url, "udp://") ||
        bm::start_with(m_output_url, "tcp://")) {
        enable_outputer = true;
    }
    enable_outputer = true;
#endif
#if WITH_DETECTOR
    m_detectorDelegate->set_detected_callback([this, enable_outputer](bm::cvs10FrameInfo &frameInfo) {
        for (int frame_idx = 0; frame_idx < frameInfo.frames.size(); ++frame_idx) {
            int ch = frameInfo.frames[frame_idx].chan_id;

            m_appStatis.m_chan_statis[ch]++;
            m_appStatis.m_statis_lock.lock();
            m_appStatis.m_total_statis++;
            m_appStatis.m_statis_lock.unlock();
#if USE_DEBUG
            std::ostringstream oss;
            oss << " frame index: " << frameInfo.frames[frame_idx].seq << ", detect " << frameInfo.out_datums[frame_idx].obj_rects.size() << " objs.";
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

                                
            }

            m_frame_count += 1;
            if ((m_frame_count >= m_stop_frame_num*m_channel_num/(1+m_skipN)) && (m_stop_frame_num*m_channel_num/(1+m_skipN) >= 0)){
                std::cout <<  "-=-=-=-======exit==============>>> " << std::endl;
                if (enable_outputer && ((ch < m_save_num) || (m_save_num == -1))){
                    m_chans[ch]->outputer->CloseOutputStream();
                }
                exit(-1);
                   
            }
        }
    });
#endif //WITH_DETECTOR

    // feature register
#if WITH_EXTRACTOR
    m_featureDelegate->set_detected_callback([this](bm::FeatureFrameInfo &frameInfo) {
        for (int i = 0; i < frameInfo.frames.size(); ++i) {
            int ch = frameInfo.frames[i].chan_id;

            m_appStatis.m_chan_feat_stat[ch]++;
            m_appStatis.m_total_feat_stat++;
        }
    });
#endif
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

#if WITH_DETECTOR
    param.batch_num = m_detectorDelegate->get_max_batch();
    m_inferPipe.init(param, m_detectorDelegate);
#endif
    //feature
#if WITH_EXTRACTOR
    param->batch_num = m_featureDelegate->get_max_batch();
    m_featurePipe.init(param, m_featureDelegate);
#endif

    for(int i = 0; i < m_channel_num; ++i) {
        int ch = m_channel_start + i;

        TChannelPtr pchan = std::make_shared<TChannel>();
        pchan->demuxer = new bm::StreamDemuxer(ch);
        if (enable_outputer) pchan->outputer = new bm::FfmpegOutputer();
        pchan->channel_id = ch;

        std::string media_file;
        AVDictionary *opts = NULL;
        av_dict_set_int(&opts, "sophon_idx", m_dev_id, 0);
        av_dict_set(&opts, "output_format", "101", 18); //101
        av_dict_set(&opts, "extra_frame_buffer_num", "18", 0);

        pchan->demuxer->set_avformat_opend_callback([this, pchan](AVFormatContext *ifmt) {
            pchan->create_video_decoder(m_dev_id, ifmt);
            // todo create DDR reduction for optimization

            if (pchan->outputer && ((pchan->channel_id < m_save_num) || (m_save_num == -1))) {
                
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
        #if WITH_DECODE
            int got_picture = 0;
            AVFrame *frame = av_frame_alloc();
            static int ddd = 0;
            ddd++;
            std::cout<<"decode times: " << ddd << std::endl;
            pchan->decode_video2(pchan->m_decoder, frame, &got_picture, pkt);

            #if 0// DUMP_BIN
            if(got_picture && frame->channel_layout == 101){
                FILE *fp_fbc[4] = {0};
                char fbc_filename[4][128];
                static int save_bin_id = 0;
                for(int ii=0; ii<4; ii++)
                {
                    sprintf(fbc_filename[ii], "results/%d_fbc_data%d.dat", save_bin_id, ii);
                    fp_fbc[ii] = fopen(fbc_filename[ii], "wb");
                }
                int size = frame->height * frame->linesize[4];
                
                //     (unsigned long long) in.data[6], size;
                // size = (in.height / 2) * in.linesize[5];
                //     (unsigned long long) in.data[4], size;
                // size = in.linesize[6];
                //     (unsigned long long) in.data[7], size;
                // size = in.linesize[7];
                //     (unsigned long long) in.data[5], size;
            }
            #endif

        #endif
        
        #if WITH_ENCODE_H264 //need output format=0
            if(got_picture){
                #define STEP_ALIGNMENT 32
                int stride = (frame->width + STEP_ALIGNMENT - 1) & ~(STEP_ALIGNMENT - 1);
                int buffer_size = av_image_get_buffer_size(
                                        (AVPixelFormat)frame->format, frame->width, frame->height, 32
                                    );
                uint8_t *yuv_buffer = (uint8_t *)av_malloc(buffer_size);
                if (!yuv_buffer) {
                    fprintf(stderr, "Could not allocate buffer for YUV data\n");
                    av_frame_free(&frame);
                    return;
                }
                av_image_copy_to_buffer(
                    yuv_buffer, buffer_size,
                    (const uint8_t * const *)frame->data, frame->linesize,
                    (AVPixelFormat)frame->format, frame->width, frame->height, 32
                );
                if(!pchan->writer.is_opened){
                    std::string output_path = "results/output_" + std::to_string(pchan->channel_id) + ".h264";
                    int ret_writer = pchan->writer.openEnc(output_path.c_str(), 
                                                                0, 
                                                                AV_CODEC_ID_H264, 
                                                                25, 
                                                                frame->width, 
                                                                frame->height, 
                                                                frame->format, 
                                                                25*frame->width*frame->height/8, 
                                                                0);
                    if (ret_writer !=0 ) {
                        av_log(NULL, AV_LOG_ERROR,"writer.openEnc failed\n");
                        return;
                    }
                    pchan->writer.is_opened = true;
                    pchan->writer.writeFrame(yuv_buffer, stride, frame->width, frame->height);
                } else{
                    pchan->writer.writeFrame(yuv_buffer, stride, frame->width, frame->height);
                }
            }
        #endif
        #if WITH_ENCODE_JPEG //save decoded avframe
            if (got_picture){

                bm_image image1;
                bm::BMImage::from_avframe(m_bmctx->handle(), frame, image1, true);
                bm_image* image_for_enc;
            #if WITH_RESIZE_TEST
                int test_resize_h = 400;
                int test_resize_w = 711;
                int test_align = 64;
                bm_image resized;
                ret = bm::BMImage::create_batch(m_bmctx->handle(), test_resize_h, test_resize_w, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, &resized, 1, test_align);
                assert(ret == BM_SUCCESS);
                ret = bmcv_image_vpp_convert(m_bmctx->handle(), 1, image1, &resized, NULL, BMCV_INTER_LINEAR);
                assert(ret == BM_SUCCESS);
            #if WITH_CONVERTO_TEST
                bm_image convertoed;
                ret = bm::BMImage::create_batch(m_bmctx->handle(), test_resize_h, test_resize_w, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_FLOAT32, &convertoed, 1, 1, false, true);
                assert(ret == BM_SUCCESS);
                bmcv_convert_to_attr convert_to_attr;
                convert_to_attr.alpha_0 = 1;
                convert_to_attr.alpha_1 = 1;
                convert_to_attr.alpha_2 = 1;
                convert_to_attr.beta_0  = -123.0f;
                convert_to_attr.beta_1  = -117.0f;
                convert_to_attr.beta_2  = -104.0f;
                ret = bmcv_image_convert_to(m_bmctx->handle(), 1, convert_to_attr, &resized, &convertoed);
                assert(ret == BM_SUCCESS);
                std::cout<<"calculate converto data:=="<<std::endl;
                // bm_image converto_;
                // bm::BMImage::create_batch(m_bmctx->handle(), test_resize_h, test_resize_w, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, &converto_, 1, 1);
                // bmcv_image_storage_convert(m_bmctx->handle(), 1, &convertoed, &converto_);
                bm_image resized_;
                bm::BMImage::create_batch(m_bmctx->handle(), test_resize_h, test_resize_w, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, &resized_, 1, 1);
                bmcv_copy_to_atrr_t copy_to_attr{0,0,0,0,0,0};
                bmcv_image_copy_to(m_bmctx->handle(), copy_to_attr, resized, resized_);
                compare_resize_converto(resized_, convertoed, convert_to_attr);
                // bm_image_dump_size(resized, 50);
                // bm_image_dump_size(converto_, 50);
                ret = bm_image_destroy_allinone(&resized_);
                assert(ret == BM_SUCCESS);
                // ret = bm_image_destroy_allinone(&converto_);
                // assert(ret == BM_SUCCESS);
                std::cout<<"========================="<<std::endl;
                ret = bm_image_destroy_allinone(&convertoed);
                assert(ret == BM_SUCCESS);
            #endif
                #if PLD
                    std::cout<<"=========================="<<std::endl;
                    std::cout<<"==saving resized frames!=="<<std::endl;
                    std::cout<<"=========================="<<std::endl;
                #endif
                uint8_t *jpeg_data_resized=NULL;
                size_t out_size_resized = 0;
                bm_image resized_yuv420;
                ret = bm::BMImage::create_batch(m_bmctx->handle(), test_resize_h, test_resize_w, FORMAT_YUV420P, DATA_TYPE_EXT_1N_BYTE, &resized_yuv420, 1, test_align);
                assert(ret == BM_SUCCESS);
                ret = bmcv_image_storage_convert(m_bmctx->handle(), 1, &resized, &resized_yuv420);
                assert(ret == BM_SUCCESS);

                // cv::Mat resized_mat;
                // cv::bmcv::toMAT(&resized_yuv420, resized_mat);
                // static int cc = 0;
                // std::string cvimg_file = "results/cvmat_resized_" + std::to_string(cc++) + ".jpg";
                // cv::imwrite(cvimg_file, resized_mat);      //400 711  
                bm_image_format_info_t  info;
                bm_image_get_format_info(&resized_yuv420, &info);

                ret = bmcv_image_jpeg_enc(m_bmctx->handle(), 1, &resized_yuv420, (void**)&jpeg_data_resized, &out_size_resized, 85);
                if (ret == BM_SUCCESS) {
                    static int ii = 0;
                    std::string img_file = "results/resized_frame_" + std::to_string(ii++) + ".jpg";
                    FILE *fp = fopen(img_file.c_str(), "wb");
                    fwrite(jpeg_data_resized, out_size_resized, 1, fp);
                    fclose(fp);
                } else{
                    std::cout<<"bmcv_image_jpeg_enc resized failed!"<<std::endl;
                }
                free(jpeg_data_resized);
                ret = bm_image_destroy_allinone(&resized);
                assert(ret == BM_SUCCESS);
                ret = bm_image_destroy_allinone(&resized_yuv420);
                assert(ret == BM_SUCCESS);
            #endif
                #if PLD
                    std::cout<<"=========================="<<std::endl;
                    std::cout<<"==saving decoded frames!=="<<std::endl;
                    std::cout<<"=========================="<<std::endl;
                #endif
                uint8_t *jpeg_data=NULL;
                size_t out_size = 0;
                ret = bmcv_image_jpeg_enc(m_bmctx->handle(), 1, &image1, (void**)&jpeg_data, &out_size, 85);
                if (ret == BM_SUCCESS) {
                    static int ii = 0;
                    std::string img_file = "results/decoded_frame_" + std::to_string(ii++) + ".jpg";
                    FILE *fp = fopen(img_file.c_str(), "wb");
                    fwrite(jpeg_data, out_size, 1, fp);
                    fclose(fp);
                } else{
                    std::cout<<"bmcv_image_jpeg_enc decoded failed!"<<std::endl;
                }
                free(jpeg_data);
                ret = bm_image_destroy_allinone(&image1);
                assert(ret == BM_SUCCESS);
            }
        #endif
        #if WITH_DETECTOR
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
        #endif
        #if WITH_DECODE
            av_frame_free(&frame);
        #endif
        #if WITH_EXTRACTOR
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
        #endif

        });

    #if PLD
        bool repeat = false;
    #else
        bool repeat = true
    #endif
        pchan->demuxer->open_stream(urls[i % urls.size()], nullptr, repeat, opts);
        av_dict_free(&opts);
        m_chans[ch] = pchan;
    }
}
