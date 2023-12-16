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

        #if WITH_TRACKER
        #if PLD
            std::cout<<"go to bm_tracker..."<<std::endl;
        #endif
            if (frameInfo.out_datums[frame_idx].obj_rects.size() > 0) {
                m_chans[ch]->tracker->update(frameInfo.out_datums[frame_idx].obj_rects, frameInfo.out_datums[frame_idx].track_rects);
            }
        #endif

            // std::cout<<"skip_frame_queue size: "<<frameInfo.frames[frame_idx].skip_frame_queue.size()<<std::endl;
            m_appStatis.m_statis_lock.lock();
            m_appStatis.m_chan_statis[ch]++;
            // m_appStatis.m_chan_statis[ch]+=frameInfo.frames[frame_idx].skip_frame_queue.size();
            m_appStatis.m_total_statis++;
            // m_appStatis.m_total_statis+=frameInfo.frames[frame_idx].skip_frame_queue.size();
            m_appStatis.m_statis_lock.unlock();

            // display
#if USE_QTGUI
            if (ch < m_display_num){
                while(!frameInfo.frames[frame_idx].skip_frame_queue.empty()){
                    auto skip_frame = frameInfo.frames[frame_idx].skip_frame_queue.front();
                    frameInfo.frames[frame_idx].skip_frame_queue.pop();
                    bm::UIFrame skip_ui_frame;
                    skip_ui_frame.jpeg_data = skip_frame.img_data;
                    skip_ui_frame.chan_id = ch;
                    skip_ui_frame.h = frameInfo.frames[frame_idx].height;
                    skip_ui_frame.w = frameInfo.frames[frame_idx].width;
                    skip_ui_frame.datum = frameInfo.out_datums[frame_idx];
                    // std::cout<<"push skiped frame!!!"<<std::endl;
                    m_guiReceiver->pushFrame(skip_ui_frame);
                }
                    // std::cout<<"push key frame!!!"<<std::endl;
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
                
                while(!frameInfo.frames[frame_idx].skip_frame_queue.empty()){
                    auto reff = frameInfo.frames[frame_idx].skip_frame_queue.front();
                    av_packet_copy_props(&sei_pkt, reff.avpkt);
                    sei_pkt.stream_index = reff.avpkt->stream_index;
                    m_chans[ch]->outputer->InputPacket(&sei_pkt);
                    m_chans[ch]->outputer->InputPacket(reff.avpkt);
                    if (reff.avpkt){
                        av_packet_unref(reff.avpkt);
                        av_packet_free(&reff.avpkt);
                    }
                    frameInfo.frames[frame_idx].skip_frame_queue.pop();
                }
                AVPacket *pkt1 = frameInfo.frames[frame_idx].avpkt;
                av_packet_copy_props(&sei_pkt, pkt1);
                sei_pkt.stream_index = pkt1->stream_index;
                m_chans[ch]->outputer->InputPacket(&sei_pkt);
                m_chans[ch]->outputer->InputPacket(frameInfo.frames[frame_idx].avpkt);
                av_packet_unref(&sei_pkt);
            }

            // m_frame_count += 1;
            // if ((m_frame_count >= m_stop_frame_num*m_channel_num) && (m_stop_frame_num*m_channel_num >= 0)){
            //     std::cout<<"======================"<<std::endl;
            //     std::cout<<"m_frame_count:"<<m_frame_count<<std::endl;
            //     std::cout<<"======================"<<std::endl;
            //     m_frame_count = 0;
            //     // std::cout <<  "-=-=-=-======exit==============>>> " << std::endl;
            //     // if (enable_outputer && ((ch < m_save_num) || (m_save_num == -1))){
            //     //     m_chans[ch]->outputer->CloseOutputStream();
            //     // }
            //     // exit(-1);
            // }
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
    m_inferPipe.init(param, m_detectorDelegate, "detector");
#endif
    //feature
    cv::Mat feature_mat;
#if WITH_EXTRACTOR
    param.batch_num = m_featureDelegate->get_max_batch();
    param.preprocess_thread_num = 1;
    param.preprocess_queue_size = 4;
    m_featurePipe.init(param, m_featureDelegate, "extractor");
#if USE_SOPHON_OPENCV
    feature_mat = cv::imread("face.jpeg", cv::IMREAD_COLOR, m_dev_id);
#else
    feature_mat = cv::imread("face.jpeg", cv::IMREAD_COLOR);
#endif
#endif
#if WITH_JPEG_160FPS
    FILE *jpeg_fp = fopen("face.jpeg", "rb+");
    assert(jpeg_fp != NULL);
    fseek(jpeg_fp, 0, SEEK_END);
    size_t jpeg_size = ftell(jpeg_fp);
    uint8_t* jpeg_data = (uint8_t*)malloc(jpeg_size);
    fseek(jpeg_fp, 0, SEEK_SET);
    fread(jpeg_data, jpeg_size, 1, jpeg_fp);
    fclose(jpeg_fp);
    bm_image jpeg_bmimg;
    memset((char*)&jpeg_bmimg, 0, sizeof(bm_image));
    assert(BM_SUCCESS == bmcv_image_jpeg_dec(m_bmctx->handle(), (void**)&jpeg_data, &jpeg_size, 1, &jpeg_bmimg));
    assert(BM_SUCCESS == bm_image_create(m_bmctx->handle(), 128, 128, jpeg_bmimg.image_format, jpeg_bmimg.data_type, &jpeg_bmimg_40x40));
    assert(BM_SUCCESS == bmcv_image_vpp_convert(m_bmctx->handle(), 1, jpeg_bmimg, &jpeg_bmimg_40x40));
    assert(BM_SUCCESS == bm_image_destroy_allinone(&jpeg_bmimg)); 

        int jpeg_channel_num = m_channel_num;
        int jpeg_delay_thresh = 100 / jpeg_channel_num;
        for(int ch = 0; ch < jpeg_channel_num; ch++){
            auto jpeg_thread = new std::thread([this, ch, jpeg_delay_thresh]{
                int jpeg_feat_num = 10;
                int jpeg_idx = 0;
                uint64_t last_time = 0;
                int m_jpeg_delay = m_feature_delay / 10;
                while(true){
                    jpeg_idx %= jpeg_feat_num;
                    uint64_t current_time = bm::gettime_msec();
                    int jpeg_delay = int(current_time - last_time);
                    if((m_jpeg_delay - jpeg_delay) < jpeg_delay_thresh){
                        void *jpeg_data = NULL;
                        size_t out_size = 0;
                        if(BM_SUCCESS == bmcv_image_jpeg_enc(m_bmctx->handle(), 1, &jpeg_bmimg_40x40, &jpeg_data, &out_size)){
                            std::string save_path = "results/jpegs/jpeg_40x40_ch"+std::to_string(ch)+"_id"+std::to_string(jpeg_idx)+".jpg";
                            FILE *jpeg40x40_fp = fopen(save_path.c_str(), "wb");
                            fwrite(jpeg_data, out_size, 1, jpeg40x40_fp);
                            fclose(jpeg40x40_fp);
                        }else{
                            std::cerr << "enc jpeg failed......" << std::endl;
                        }
                        free(jpeg_data);
                        m_appStatis.m_statis_feat_encode_lock.lock();
                        m_appStatis.m_total_feat_encode+=1;
                        m_appStatis.m_statis_feat_encode_lock.unlock();
                        last_time = current_time;
                        jpeg_idx += 1; 
                    }
                    else{
                        bm::msleep(m_jpeg_delay - jpeg_delay - 1);
                    }
                }
            });
        }
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
        av_dict_set(&opts, "extra_frame_buffer_num", "5", 0);

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

        pchan->demuxer->set_read_Frame_callback([this, pchan, ch, feature_mat](AVPacket* pkt){
            int ret = 0;
            bm_handle_t handle = m_bmctx->handle();

        #if WITH_DECODE
            int got_picture = 0;
            AVFrame *frame = av_frame_alloc();
            // static int ddd = 0;
            // ddd++;
            // if(ddd % 1000 == 0){
            //     std::cout<<"ch: "<<ch<<", decode times:"<<ddd<<std::endl;
            //     if(ddd > 2147483600) ddd = 0;
            // }
            // std::cout<<"decode times: " << ddd << std::endl;
            pchan->decode_video2(pchan->m_decoder, frame, &got_picture, pkt);
            if(got_picture){
                m_appStatis.m_statis_decode_lock.lock();
                m_appStatis.m_total_decode++;
                m_appStatis.m_statis_decode_lock.unlock();
            }
        #endif
        #if USE_QTGUI && WITH_HDMI
            bm::NetOutputObject fake_object(100,100,200,200);
            bm::NetOutputObjects fake_objects{fake_object};
            bm::NetOutputDatum fake_datum(fake_objects);
            if(ch < m_display_num && got_picture){
                bm_image image1;
                bm::BMImage::from_avframe(m_bmctx->handle(), frame, image1, true);
                bm_image image2;
                int image2_h = gui_resize_h;
                int image2_w = gui_resize_w;
                bm_image_create(m_bmctx->handle(), image2_h, image2_w, FORMAT_RGB_PACKED, image1.data_type, &image2, NULL);
                bmcv_image_vpp_convert(m_bmctx->handle(), 1, image1, &image2, NULL, BMCV_INTER_LINEAR);
                bm_image_destroy_allinone(&image1);
                int plane_num = bm_image_get_plane_num(image2);
                int plane_size[1];
                assert(0 == bm_image_get_byte_size(image2, plane_size));
                uint8_t *buffers_image2[1]={0};
            #if USE_D2S
                buffers_image2[0] = new uint8_t[plane_size[0]];
                assert(BM_SUCCESS == bm_image_copy_device_to_host(image2, (void**)buffers_image2));//RGB
            #elif USE_MMAP
                unsigned long long addr;
                bm_device_mem_t image2_dmem;
                bm_image_get_device_mem(image2, &image2_dmem);
                bm_mem_mmap_device_mem(m_bmctx->handle(), &image2_dmem, &addr);
                buffers_image2[0] = (uint8_t*)addr;
            #endif
                bm::UIFrame jpgframe;
                jpgframe.jpeg_data = std::make_shared<bm::Data>(buffers_image2[0], plane_size[0]);
            #if USE_MMAP
                jpgframe.jpeg_data->bmimg_formmap = image2;
                jpgframe.jpeg_data->is_mmap = true;
            #endif
                jpgframe.jpeg_data->height = image2.height;
                jpgframe.jpeg_data->width = image2.width;
                jpgframe.jpeg_data->image_format = FORMAT_RGB_PACKED;
                jpgframe.chan_id = ch;
                jpgframe.h = image2.height;
                jpgframe.w = image2.width;
                jpgframe.datum = fake_datum;
                m_guiReceiver->pushFrame(jpgframe);

            #if USE_D2S
                bm_image_destroy_allinone(&image2);
            #endif
            }
        #endif

        #if WITH_DETECTOR
            if (got_picture) {
                if(pchan->seq % m_skip_y < m_skip_x){
                    //push frame to skip frame queue
                    bm::skipedFrameinfo skip_fbi;
                #if USE_QTGUI
                    if(ch < m_display_num){
                        skip_fbi.avframe = av_frame_alloc();
                        av_frame_ref(skip_fbi.avframe, frame);
                    }
                #elif WITH_OUTPUTER //pipeline client
                    skip_fbi.avpkt = av_packet_alloc();
                    av_packet_ref(skip_fbi.avpkt, pkt);
                #endif
                    m_skipframe_queue[ch].push(skip_fbi);
                }else{
                    bm::cvs10FrameBaseInfo fbi;
                    fbi.seq = pchan->seq;
                    fbi.chan_id = ch;
                    fbi.pkt_id = 0;
                    // std::cout<<"m_skip_frame_queue size: "<<m_skipframe_queue[ch].size()<<std::endl;
                    while(!m_skipframe_queue[ch].empty()){
                        fbi.skip_frame_queue.push(m_skipframe_queue[ch].front());
                        m_skipframe_queue[ch].pop();
                    }
                    fbi.avframe = av_frame_alloc();
                    fbi.avpkt = av_packet_alloc();
                    av_frame_ref(fbi.avframe, frame);
                    av_packet_ref(fbi.avpkt, pkt);
                    m_inferPipe.push_frame(&fbi);
                }
            }
        #endif

        #if WITH_ENCODE_H264 //need output format=0
            if(got_picture && ch < m_save_num){
                AVFrame *frame_yuv420p = av_frame_alloc(); //for encoder
            #if DECODE_YUY420P //directly decode yuv420p
                av_frame_ref(frame_yuv420p, frame);
            #else
                bm_image* bm_image_yuv420p = NULL;
                bm_image_yuv420p = (bm_image *) malloc(sizeof(bm_image));;
                bm::BMImage::from_avframe(handle, frame, *bm_image_yuv420p, true);
                bm_image_to_avframe(handle, bm_image_yuv420p, frame_yuv420p);
                // AVFrameConvert(handle, frame, frame_yuv420p, frame->height, frame->width, AV_PIX_FMT_YUV420P);
            #endif
            #if !FF_ENCODE_WRITE_AVFRAME
                #define STEP_ALIGNMENT 32
                int stride = (frame_yuv420p->width + STEP_ALIGNMENT - 1) & ~(STEP_ALIGNMENT - 1);
                int buffer_size = av_image_get_buffer_size(
                                        (AVPixelFormat)frame_yuv420p->format, frame_yuv420p->width, frame_yuv420p->height, 32
                                    );
                uint8_t *yuv_buffer = (uint8_t *)av_malloc(buffer_size);
                if (!yuv_buffer) {
                    fprintf(stderr, "Could not allocate buffer for YUV data\n");
                    av_frame_free(&frame_yuv420p);
                    return;
                }
                av_image_copy_to_buffer(
                    yuv_buffer, buffer_size,
                    (const uint8_t * const *)frame_yuv420p->data, frame_yuv420p->linesize,
                    (AVPixelFormat)frame_yuv420p->format, frame_yuv420p->width, frame_yuv420p->height, 32
                );
            #endif
                if(!pchan->writer.is_opened){
                    std::string output_path = "results/output_" + std::to_string(pchan->channel_id) + ".h264";
                    int ret_writer = pchan->writer.openEnc(output_path.c_str(), 
                                                                "h264_bm", 
                                                                0,
                                                                25, 
                                                                frame_yuv420p->width, 
                                                                frame_yuv420p->height, 
                                                                frame_yuv420p->format, 
                                                                25 * frame_yuv420p->width * frame_yuv420p->height / 8);
                    if (ret_writer != 0) {
                        av_log(NULL, AV_LOG_ERROR,"writer.openEnc failed\n");
                        return;
                    }
                    pchan->writer.is_opened = true;
                #if FF_ENCODE_WRITE_AVFRAME
                    pchan->writer.writeFrame(frame_yuv420p);
                #else
                    pchan->writer.writeFrame(yuv_buffer, stride, frame_yuv420p->width, frame_yuv420p->height);
                #endif
                } else if(pchan->seq % 10000 == 0){
                    std::cout << "ch: " << ch << "; encode frame num = 10000, restart encoder."<<std::endl;
                    pchan->writer.is_opened = false;
                    pchan->writer.closeEnc();
                } 
                else{
                #if FF_ENCODE_WRITE_AVFRAME
                    pchan->writer.writeFrame(frame_yuv420p);
                #else
                    pchan->writer.writeFrame(yuv_buffer, stride, frame_yuv420p->width, frame_yuv420p->height);
                #endif
                }
            #if !FF_ENCODE_WRITE_AVFRAME
                if (yuv_buffer) {
                    av_free(yuv_buffer);
                    yuv_buffer = NULL;
                }
            #endif
                av_frame_unref(frame_yuv420p);
                av_frame_free(&frame_yuv420p);
                m_appStatis.m_statis_encode_lock.lock();
                m_appStatis.m_total_encode++;
                m_appStatis.m_statis_encode_lock.unlock();
            }
        #endif
        #if WITH_ENCODE_JPEG || WITH_RESIZE_TEST //save decoded avframe
            if (got_picture){
                bm_image image1;
                bm::BMImage::from_avframe(m_bmctx->handle(), frame, image1, true);
                bm_image* image_for_enc;
            #if WITH_RESIZE_TEST
                int test_resize_h = 360;
                int test_resize_w = 640;
                int test_align = 64;
                bm_image resized;
                ret = bm::BMImage::create_batch(m_bmctx->handle(), test_resize_h, test_resize_w, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, &resized, 1, test_align);
                assert(ret == BM_SUCCESS);
                // std::cout<<"Now, bmcv_image_vpp_convert: "<<std::endl;
                ret = bmcv_image_vpp_convert(m_bmctx->handle(), 1, image1, &resized, NULL, BMCV_INTER_LINEAR);
                assert(ret == BM_SUCCESS);
            #if WITH_CONVERTO_TEST
                bm_image convertoed;
                ret = bm::BMImage::create_batch(m_bmctx->handle(), test_resize_h, test_resize_w, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE_SIGNED, &convertoed, 1, 1, false, true);
                assert(ret == BM_SUCCESS);
                bmcv_convert_to_attr convert_to_attr;
                convert_to_attr.alpha_0 = 1;
                convert_to_attr.alpha_1 = 1;
                convert_to_attr.alpha_2 = 1;
                convert_to_attr.beta_0  = -123.0f;
                convert_to_attr.beta_1  = -117.0f;
                convert_to_attr.beta_2  = -104.0f;
                // std::cout<<"Now, bmcv_image_convert_to: "<<std::endl;
                ret = bmcv_image_convert_to(m_bmctx->handle(), 1, convert_to_attr, &resized, &convertoed);
                assert(ret == BM_SUCCESS);
                // std::cout<<"calculate converto data:=="<<std::endl;
                // bm_image resized_;
                // bm::BMImage::create_batch(m_bmctx->handle(), test_resize_h, test_resize_w, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, &resized_, 1, 1);
                // bmcv_copy_to_atrr_t copy_to_attr{0,0,0,0,0,0};
                // bmcv_image_copy_to(m_bmctx->handle(), copy_to_attr, resized, resized_);
                // compare_resize_converto(resized_, convertoed, convert_to_attr);
                // bm_image_dump_size(resized, 50);
                // bm_image_dump_size(resized_, 50);
                // ret = bm_image_destroy_allinone(&resized_);
                assert(ret == BM_SUCCESS);
                // assert(ret == BM_SUCCESS);
                ret = bm_image_destroy_allinone(&convertoed);
                assert(ret == BM_SUCCESS);

            #endif
            #if SAVE_RESIZED_FRAMES
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
                    static int rr[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
                    rr[ch]++;
                    std::string img_file = "results/resized_frame_" + std::to_string(rr[ch]) + ".jpg";
                    FILE *fp = fopen(img_file.c_str(), "wb");
                    fwrite(jpeg_data_resized, out_size_resized, 1, fp);
                    fclose(fp);
                } else{
                    std::cout<<"bmcv_image_jpeg_enc resized failed!"<<std::endl;
                }
                free(jpeg_data_resized);
                ret = bm_image_destroy_allinone(&resized_yuv420);
                assert(ret == BM_SUCCESS);
            #endif
                ret = bm_image_destroy_allinone(&resized);
                assert(ret == BM_SUCCESS);
            #endif

            #if WITH_ENCODE_JPEG//OUTPUT OR SAVE_ENCODED_FRAMES
                #if PLD
                    std::cout<<"=========================="<<std::endl;
                    std::cout<<"==saving decoded frames!=="<<std::endl;
                    std::cout<<"=========================="<<std::endl;
                #endif
                uint8_t *jpeg_data=NULL;
                size_t out_size = 0;
                ret = bmcv_image_jpeg_enc(m_bmctx->handle(), 1, &image1, (void**)&jpeg_data, &out_size, 85);
                if (ret == BM_SUCCESS) {
                    static int ii[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
                    ii[ch]++;
                    std::string img_file = "results/ch" + std::to_string(ch) + "_" + std::to_string(ii[ch]) + ".jpg";
                    FILE *fp = fopen(img_file.c_str(), "wb");
                    fwrite(jpeg_data, out_size, 1, fp);
                    fclose(fp);
                    m_appStatis.m_statis_encode_lock.lock();
                    m_appStatis.m_total_encode++;
                    m_appStatis.m_statis_encode_lock.unlock();
                } else{
                    std::cout<<"bmcv_image_jpeg_enc decoded failed!"<<std::endl;
                }
                free(jpeg_data);
            #endif
                ret = bm_image_destroy_allinone(&image1);
                assert(ret == BM_SUCCESS);
            }
        #endif
        
        #if WITH_DECODE
            pchan->seq++;
            av_frame_unref(frame);
            av_frame_free(&frame);
        #endif
        #if WITH_EXTRACTOR
            uint64_t current_time = bm::gettime_msec();
            if (current_time - m_chans[ch]->m_last_feature_time > m_feature_delay) {
                for(int feat_idx = 0; feat_idx < m_feature_num; feat_idx++) {
                    bm::FeatureFrame featureFrame;
                    featureFrame.chan_id = ch;
                    featureFrame.seq++;
                    featureFrame.img = feature_mat;

                    if (featureFrame.img.empty()) {
                        printf("ERROR:Can't find face.jpg in workdir!\n");
                        exit(0);
                    }
                    m_appStatis.m_statis_lock.lock();
                    m_appStatis.m_total_feat_decode++;
                    m_appStatis.m_statis_lock.unlock();
                    m_featurePipe.push_frame(&featureFrame);
                }
            // write here cannot reach 160FPS
            // #if WITH_JPEG_160FPS
            //     int jpeg_feat_num = 10;
            //     for(int jpeg_idx = 0; jpeg_idx < jpeg_feat_num; jpeg_idx++){
            //         void *jpeg_data = NULL;
            //         size_t out_size = 0;
            //         if(BM_SUCCESS == bmcv_image_jpeg_enc(m_bmctx->handle(), 1, &jpeg_bmimg_40x40, &jpeg_data, &out_size)){
            //             std::string save_path = "results/jpegs/jpeg_40x40_ch"+std::to_string(ch)+"_id"+std::to_string(jpeg_idx)+".jpg";
            //             FILE *jpeg40x40_fp = fopen(save_path.c_str(), "wb");
            //             fwrite(jpeg_data, out_size, 1, jpeg40x40_fp);
            //             fclose(jpeg40x40_fp);
            //         }else{
            //             std::cerr << "enc jpeg failed......" << std::endl;
            //         }
            //         free(jpeg_data);
            //     }
            //     m_appStatis.m_statis_lock.lock();
            //     m_appStatis.m_total_feat_encode+=jpeg_feat_num;
            //     m_appStatis.m_statis_lock.unlock();
            // #endif
                m_chans[ch]->m_last_feature_time = current_time;
            }
        #endif
        });

        bool repeat = true;
        pchan->demuxer->open_stream(urls[i % urls.size()], nullptr, repeat, opts);
        av_dict_free(&opts);
        m_chans[ch] = pchan;
    }
}
