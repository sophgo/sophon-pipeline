//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "face_detector.h"
#include <algorithm>
#include "bm_wrapper.hpp"

#define DUMP_FILE 0
#define DYNAMIC_SIZE 0
#define PLD_DEBUG_DUMP_DATA 0
#ifndef USE_RGBP_SEPARATE //convert_to do not support
#define USE_RGBP_SEPARATE 0
#endif

static inline bool compareBBox(const bm::NetOutputObject &a, const bm::NetOutputObject &b) {
    return a.score > b.score;
}

FaceDetector::FaceDetector(bm::BMNNContextPtr bmctx, int resize_num)
{
    auto net_name = "squeezenet"; // origin: 0
    bmctx_ = bmctx;
    anchor_ratios_.push_back(1.0f);
    anchor_scales_.push_back(1);
    anchor_scales_.push_back(2);
    anchor_num_ = 2;

    is4N_ = false;

    bmnet_ = std::make_shared<bm::BMNNNetwork>(bmctx_->bmrt(), net_name); //squeezenet_bmnetc
#if PLD
    std::cout << "net_name: "<< net_name << std::endl;
#endif
    assert(bmnet_ != nullptr);
    assert(bmnet_->inputTensorNum() == 1);

    auto tensor = bmnet_->inputTensor(0);
    m_net_h = tensor->get_shape()->dims[2]; // static net
    m_net_w = tensor->get_shape()->dims[3]; // static net

    MAX_BATCH = tensor->get_shape()->dims[0];

    resize_num_ = resize_num;
}

FaceDetector::~FaceDetector()
{

}

void FaceDetector::calc_resized_HW(int image_h, int image_w, int *p_h, int *p_w) {
    int im_size_min = std::min(image_h, image_w);
    int im_size_max = std::max(image_h, image_w);
    im_scale_  = target_size_ / im_size_min;
    if (im_scale_ * im_size_max > max_size_) {
        im_scale_ = max_size_ / im_size_max;
    }

#if DYNAMIC_SIZE

    img_x_scale_ = im_scale_;
    img_y_scale_ =  im_scale_;

    *p_h = (int)(image_h * im_scale_ + 0.5);
    *p_w = (int)(image_w * im_scale_ + 0.5);
#else

    float vw, vh;
    if (image_h > image_w) {
        vh = bmnet_->inputTensor(0)->get_shape()->dims[3];
        vw = bmnet_->inputTensor(0)->get_shape()->dims[2];
    }else{
        vh = bmnet_->inputTensor(0)->get_shape()->dims[2];
        vw = bmnet_->inputTensor(0)->get_shape()->dims[3];
    }

    *p_h = vh;
    *p_w = vw;

    img_x_scale_ = vw / image_w;
    img_y_scale_ =  vh / image_h;
#endif
}


int FaceDetector::preprocess(std::vector<bm::cvs10FrameBaseInfo>& frames, std::vector<bm::cvs10FrameInfo> &frame_infos)
{
#if 1
    int ret = 0;
    bm_handle_t handle = bmctx_->handle();
    calc_resized_HW(1080, 1920, &m_net_h, &m_net_w);

    // Check input
    int total = frames.size();
    if (total != 4) {
        printf("total = %d\n", total);
    }
    int left = (total%MAX_BATCH == 0 ? MAX_BATCH: total%MAX_BATCH);
    int batch_num = total%MAX_BATCH==0 ? total/MAX_BATCH: (total/MAX_BATCH + 1);
    for(int batch_idx = 0; batch_idx < batch_num; ++ batch_idx) {
        int num = MAX_BATCH;
        int start_idx = batch_idx*MAX_BATCH;
        if (batch_idx == batch_num-1) {
            // last one
            num = left;
        }

# if 0 // resize 1/3 for cvs20 test forcely
        
        int pre_width = 1920;
        int pre_height = 1080;
        pre_width /= 3;
        pre_height /= 3;

        bm_image cvs20_resized_imgs[MAX_BATCH];
    #if USE_RGBP_SEPARATE
        ret = bm::BMImage::create_batch(handle, pre_height, pre_width, FORMAT_RGBP_SEPARATE, DATA_TYPE_EXT_1N_BYTE, cvs20_resized_imgs, num, 64);
    #else
        ret = bm::BMImage::create_batch(handle, pre_height, pre_width, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, cvs20_resized_imgs, num, 64);
    #endif
        assert(BM_SUCCESS == ret);

        for(int i = 0;i < num; ++i) {
            if ((frames[start_idx + i].chan_id < resize_num_) || (resize_num_ == -1)){
                bm_image cvs20_image1;
                bm::BMImage::from_avframe(handle, frames[start_idx + i].avframe, cvs20_image1, true);
                std::cout<<"bm_image_format: "<<cvs20_image1.image_format<<std::endl;
                ret = bmcv_image_vpp_convert(handle, 1, cvs20_image1, &cvs20_resized_imgs[i], NULL, BMCV_INTER_LINEAR);
                assert(BM_SUCCESS == ret);

                bm_image_destroy_allinone(&cvs20_image1);
            }
        }

        bm::BMImage::destroy_batch(cvs20_resized_imgs, num);

#endif

        bm::cvs10FrameInfo finfo;
        //1. Resize
        bm_image resized_imgs[MAX_BATCH];
    #if USE_RGBP_SEPARATE
        ret = bm::BMImage::create_batch(handle, m_net_h, m_net_w, FORMAT_RGBP_SEPARATE, DATA_TYPE_EXT_1N_BYTE, resized_imgs, num, 64);
    #else
        ret = bm::BMImage::create_batch(handle, m_net_h, m_net_w, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, resized_imgs, num, 64);
    #endif
        assert(BM_SUCCESS == ret);

        for(int i = 0;i < num; ++i) {
            bm_image image1;
            bm::BMImage::from_avframe(handle, frames[start_idx + i].avframe, image1, true);
        #if PLD
            std::cout<<"This is face_detector.cpp:159: bmcv_image_vpp_convert."<<std::endl;
        #endif
            ret = bmcv_image_vpp_convert(handle, 1, image1, &resized_imgs[i], NULL, BMCV_INTER_LINEAR);
            assert(BM_SUCCESS == ret);

            uint8_t *jpeg_data=NULL;
            size_t out_size = 0;
#if USE_QTGUI
            bmcv_image_jpeg_enc(handle, 1, &image1, (void**)&jpeg_data, &out_size, 85);
#endif
            frames[start_idx + i].jpeg_data = std::make_shared<bm::Data>(jpeg_data, out_size);
            frames[start_idx + i].height= image1.height;
            frames[start_idx + i].width = image1.width;

            av_frame_unref(frames[start_idx + i].avframe);
            av_frame_free(&frames[start_idx + i].avframe);
        #if DRAW_DETECTOR
            frames[start_idx + i].original = image1;
        #else
            bm_image_destroy_allinone(&image1);
        #endif
            finfo.frames.push_back(frames[start_idx + i]);

#ifdef DEBUG
            if (frames[start_idx].chan_id == 0)
                 std::cout << "[" << frames[start_idx].chan_id << "]total index =" << start_idx + i << std::endl;
#endif
        }

        //2. Convert to
        bm_image convertto_imgs[MAX_BATCH];

        float        R_bias  = -123.0f;
        float        G_bias  = -117.0f;
        float        B_bias  = -104.0f;
        float alpha, beta;

        bm_image_data_format_ext img_type = DATA_TYPE_EXT_FLOAT32;
        auto tensor = bmnet_->inputTensor(0);
        if (tensor->get_dtype() == BM_INT8) {
            img_type = DATA_TYPE_EXT_1N_BYTE_SIGNED;
            alpha            = tensor->get_scale() * 1.0;//0.847682119;
            beta             = 0.0;
        #if A2_SDK
            img_type = DATA_TYPE_EXT_1N_BYTE_SIGNED;
        #else
            img_type = (is4N_) ? (DATA_TYPE_EXT_4N_BYTE_SIGNED)
                               : (DATA_TYPE_EXT_1N_BYTE_SIGNED);
        #endif
        }else{
            alpha            = 1.0;
            beta             = 0.0;
            img_type = DATA_TYPE_EXT_FLOAT32;
        }
    #if PLD
        std::cout<<"creating converto_imgs"<<std::endl;
    #endif
    #if USE_RGBP_SEPARATE
        ret = bm::BMImage::create_batch(handle, m_net_h, m_net_w, FORMAT_RGBP_SEPARATE, img_type, convertto_imgs, num, 1, false, true); //set A2 PLD new stride = 64
    #else
        ret = bm::BMImage::create_batch(handle, m_net_h, m_net_w, FORMAT_RGB_PLANAR, img_type, convertto_imgs, num, 1, false, true);
    #endif
        assert(BM_SUCCESS == ret);

        bm_tensor_t input_tensor = *tensor->bm_tensor();
        bm::bm_tensor_reshape_NCHW(handle, &input_tensor, num, 3, m_net_h, m_net_w);

        ret = bm_image_attach_contiguous_mem(num, convertto_imgs, input_tensor.device_mem);
        assert(BM_SUCCESS == ret);

        bmcv_convert_to_attr convert_to_attr;
        convert_to_attr.alpha_0 = alpha;
        convert_to_attr.alpha_1 = alpha;
        convert_to_attr.alpha_2 = alpha;
        convert_to_attr.beta_0  = beta + B_bias;
        convert_to_attr.beta_1  = beta + G_bias;
        convert_to_attr.beta_2  = beta + R_bias;

    #if PLD
        std::cout<<"This is face_detector.cpp:227, bmcv_image_convert_to."<<std::endl;
    #endif
        ret = bmcv_image_convert_to(bmctx_->handle(), num, convert_to_attr, resized_imgs, convertto_imgs);
        assert(ret == 0);

    #if A2_SDK
        bm_image_detach_contiguous_mem(num, convertto_imgs);
    #else
        bm_image_dettach_contiguous_mem(num, convertto_imgs);
    #endif
    #if PLD
        std::cout<<"face_detect: input_tensor:"<<std::endl;
        std::cout<<"========================="<<std::endl;
        bm::BMNNTensorPtr tensor_ = std::make_shared<bm::BMNNTensor>(bmctx_->handle(), "detector_in", 1.0,
                                                                &input_tensor);
        float* input_cpu_data = tensor_->get_cpu_data();
        for(int kk = 0; kk < 20; kk++){
            std::cout<<*(input_cpu_data+kk)<<" ";
        }
        std::cout<<std::endl<<"========================="<<std::endl;
    #endif
        finfo.input_tensors.push_back(input_tensor);

        bm::BMImage::destroy_batch(resized_imgs, num);
        bm::BMImage::destroy_batch(convertto_imgs, num);

        frame_infos.push_back(finfo);
    }


#else
    bm::FrameInfo frame_info;
    for(int i = 0;i < frames.size(); ++i) {
        bm::FrameBaseInfo finfo;
        finfo.avframe = frames[i].avframe;
        finfo.avpkt = frames[i].avpkt;
        frame_info.frames.push_back(finfo);
    }

    frame_infos.push_back(frame_info);

#endif

    return 0;
}

int FaceDetector::forward(std::vector<bm::cvs10FrameInfo>& frame_infos)
{
#if 1
    int ret = 0;
    for(int b = 0; b < frame_infos.size(); ++b) {
        for (int i = 0; i < bmnet_->outputTensorNum(); ++i) {
            bm_tensor_t tensor;
            frame_infos[b].output_tensors.push_back(tensor);
        }

#if DUMP_FILE
        bm::BMImage::dump_dev_memory(bmctx_->handle(), frame_infos[b].input_tensors[0].device_mem, "convertto",
                frame_infos[b].frames.size(), m_net_h, m_net_w, false, false);
#endif
        //printf("shape batch = %d\n", frame_infos[b].input_tensors[0].shape.dims[0]);

    #if PLD
        std::cout<<"this is face_detector, forward."<<std::endl;
        std::cout<<"input_tensor shape:"<<std::endl;
        for(int i = 0; i < frame_infos[b].input_tensors.size(); i++){
            std::cout<<std::endl;
            for(int j = 0; j < frame_infos[b].input_tensors[i].shape.num_dims; j++){
                std::cout<<frame_infos[b].input_tensors[i].shape.dims[j]<<" ";
            }
        }
        std::cout<<std::endl;
    #endif
        ret = bmnet_->forward(frame_infos[b].input_tensors.data(), frame_infos[b].input_tensors.size(),
                               frame_infos[b].output_tensors.data(), frame_infos[b].output_tensors.size());
        assert(BM_SUCCESS == ret);
    #if PLD
        std::cout<<"face_detector forward success."<<std::endl;
            const char* dtypeMap[] = {
            "FLOAT32",
            "FLOAT16",
            "INT8",
            "UINT8",
            "INT16",
            "UINT16",
            "INT32",
            "UINT32",
            };

            for(int i = 0; i < frame_infos[b].input_tensors.size(); i++){
                auto shapeStr = shape_to_str(frame_infos[b].input_tensors[i].shape);
                printf("  Input %d) shape=%s dtype=%s\n",
                    i,
                    shapeStr.c_str(),
                    dtypeMap[frame_infos[b].input_tensors[i].dtype]);
            }
            for(int i = 0; i < frame_infos[b].output_tensors.size(); i++){
                auto shapeStr = shape_to_str(frame_infos[b].output_tensors[i].shape);
                printf("  Output %d) shape=%s dtype=%s\n",
                    i,
                    shapeStr.c_str(),
                    dtypeMap[frame_infos[b].output_tensors[i].dtype]);
            }
    #endif
    }
#else
    bm::FrameInfo frame4batch;
    for(int i = 0; i< frame_infos.size(); ++i) {
        for(int j=0;j < frame_infos[i].frames.size();j++) {
            frame4batch.frames.push_back(frame_infos[i].frames[j]);
            frame4batch.input_tensors.push_back(frame_infos[i].input_tensors);
        }

    }
    ret = bmnet_->forward(frame_infos[b].input_tensors.data(), frame_infos[b].input_tensors.size(),
                          frame_infos[b].output_tensors.data(), frame_infos[b].output_tensors.size());
    assert(BM_SUCCESS == ret);
#endif

    return 0;
}

int FaceDetector::postprocess(std::vector<bm::cvs10FrameInfo> &frames)
{
    #if PLD
        std::cout<<"This is face_detector.cpp, postprocess."<<std::endl;
    #endif
    for(int i=0;i < frames.size(); ++i) {

        // Free AVFrames
        auto &frame_info = frames[i];

        // extract face detection
        extract_facebox_cpu(frame_info);

        if (m_pfnDetectFinish != nullptr) {
        #if PLD
            std::cout<<"postprocess outputer or hdmi..."<<std::endl;
        #endif
            m_pfnDetectFinish(frame_info);
        }

        for(int j = 0; j < frame_info.frames.size(); ++j) {

            auto& reff = frame_info.frames[j];

            if (reff.avframe) {
                av_frame_unref(reff.avframe);
                av_frame_free(&reff.avframe);
            }

            if (reff.avpkt){
                av_packet_unref(reff.avpkt);
                av_packet_free(&reff.avpkt);
            }
        }

        // Free Tensors
        for(auto& tensor : frame_info.input_tensors) {
            bm_free_device(bmctx_->handle(), tensor.device_mem);
        }

        for(auto& tensor: frame_info.output_tensors) {
            bm_free_device(bmctx_->handle(), tensor.device_mem);
        }

    }
    return 0;
}

bm::BMNNTensorPtr FaceDetector::get_output_tensor(const std::string &name, bm::cvs10FrameInfo& frame_info, float scale=1.0) {
    int output_tensor_num = frame_info.output_tensors.size();
    int idx = bmnet_->outputName2Index(name);
    if (idx < 0 && idx > output_tensor_num-1) {
        std::cout << "ERROR:idx=" << idx << std::endl;
        assert(0);
    }
    #if PLD_DEBUG_DUMP_DATA
        std::cout<<"FaceDetector::get_output_tensor:"<<std::endl;
        const char* dtypeMap[] = {
        "FLOAT32",
        "FLOAT16",
        "INT8",
        "UINT8",
        "INT16",
        "UINT16",
        "INT32",
        "UINT32",
        };
        auto shapeStr = shape_to_str(frame_info.output_tensors[idx].shape);
        printf("Tensor name: %s) shape=%s dtype=%s\n",
            name.c_str(),
            shapeStr.c_str(),
            dtypeMap[frame_info.output_tensors[idx].dtype]);
    #endif
    bm::BMNNTensorPtr tensor = std::make_shared<bm::BMNNTensor>(bmctx_->handle(), name, scale,
                                  &frame_info.output_tensors[idx]);
    return tensor;
}

int FaceDetector::extract_facebox_cpu(bm::cvs10FrameInfo &frame_info)
{
    int image_n = frame_info.frames.size();

    float m3_scale_to_float = bmnet_->get_output_scale(0);//0.00587051f;
    float m2_scale_to_float = bmnet_->get_output_scale(2);//0.00527f;
    float m1_scale_to_float = bmnet_->get_output_scale(4);//0.00376741f;

    float m3_cls_scale_to_float = bmnet_->get_output_scale(1);
    float m2_cls_scale_to_float = bmnet_->get_output_scale(3);
    float m1_cls_scale_to_float = bmnet_->get_output_scale(5);

#if 0 // TODO: improve get output name method.
    bm::BMNNTensorPtr m3_bbox_tensor = get_output_tensor("m3@ssh_bbox_pred_output_f32_Conv_f32", frame_info, m3_scale_to_float);
    bm::BMNNTensorPtr m2_bbox_tensor = get_output_tensor("m2@ssh_bbox_pred_output_f32_Conv_f32", frame_info, m2_scale_to_float);
    bm::BMNNTensorPtr m1_bbox_tensor = get_output_tensor("m1@ssh_bbox_pred_output_f32_Conv_f32", frame_info, m1_scale_to_float);
    bm::BMNNTensorPtr m3_cls_tensor = get_output_tensor("m3@ssh_cls_prob_reshape_output_Conv_f32", frame_info, m3_cls_scale_to_float);
    bm::BMNNTensorPtr m2_cls_tensor = get_output_tensor("m2@ssh_bbox_pred_output_f32_Conv_f32", frame_info, m2_cls_scale_to_float);
    bm::BMNNTensorPtr m1_cls_tensor = get_output_tensor("m1@ssh_cls_prob_reshape_output_Conv_f32", frame_info, m1_cls_scale_to_float);
#else
    bm::BMNNTensorPtr m3_bbox_tensor = get_output_tensor(bmnet_->m_netinfo->output_names[4], frame_info, m3_scale_to_float);
    bm::BMNNTensorPtr m2_bbox_tensor = get_output_tensor(bmnet_->m_netinfo->output_names[2], frame_info, m2_scale_to_float);
    bm::BMNNTensorPtr m1_bbox_tensor = get_output_tensor(bmnet_->m_netinfo->output_names[0], frame_info, m1_scale_to_float);
    bm::BMNNTensorPtr m3_cls_tensor = get_output_tensor(bmnet_->m_netinfo->output_names[5], frame_info, m3_cls_scale_to_float);
    bm::BMNNTensorPtr m2_cls_tensor = get_output_tensor(bmnet_->m_netinfo->output_names[3], frame_info, m2_cls_scale_to_float);
    bm::BMNNTensorPtr m1_cls_tensor = get_output_tensor(bmnet_->m_netinfo->output_names[1], frame_info, m1_cls_scale_to_float);
#endif


    // NCHW
    int m3_c = m3_cls_tensor->get_shape()->dims[1];
    int m3_w = m3_cls_tensor->get_shape()->dims[3];
    int m3_h = m3_cls_tensor->get_shape()->dims[2];

    int m2_c = m2_cls_tensor->get_shape()->dims[1];
    int m2_w = m2_cls_tensor->get_shape()->dims[3];
    int m2_h = m2_cls_tensor->get_shape()->dims[2];

    int m1_c = m1_cls_tensor->get_shape()->dims[1];
    int m1_w = m1_cls_tensor->get_shape()->dims[3];
    int m1_h = m1_cls_tensor->get_shape()->dims[2];

    int b3_c = m3_bbox_tensor->get_shape()->dims[1];
    int b3_w = m3_bbox_tensor->get_shape()->dims[3];
    int b3_h = m3_bbox_tensor->get_shape()->dims[2];

    int b2_c = m2_bbox_tensor->get_shape()->dims[1];
    int b2_w = m2_bbox_tensor->get_shape()->dims[3];
    int b2_h = m2_bbox_tensor->get_shape()->dims[2];

    int b1_c = m1_bbox_tensor->get_shape()->dims[1];
    int b1_w = m1_bbox_tensor->get_shape()->dims[3];
    int b1_h = m1_bbox_tensor->get_shape()->dims[2];

    float *m3_scores      = (float*)m3_cls_tensor->get_cpu_data();
    float *m2_scores      = (float*)m2_cls_tensor->get_cpu_data();
    float *m1_scores      = (float*)m1_cls_tensor->get_cpu_data();
    #if PLD
        std::cout<<"dump face_detector m3_scores outputs:"<<std::endl;
        for(int kk = 0; kk < 10; kk++){
            std::cout<<*(m3_scores+kk) << " ";
        }
        std::cout<<std::endl;
    #endif
    float *m3_bbox_deltas = (float*)m3_bbox_tensor->get_cpu_data();
    float *m2_bbox_deltas = (float*)m2_bbox_tensor->get_cpu_data();
    float *m1_bbox_deltas = (float*)m1_bbox_tensor->get_cpu_data();
    
    #if WITH_FAKEDATA 
        std::cout<<"LOADING_FAKE_DATA!"<<std::endl;
        if (access("results/fakedata", 0) != F_OK){
            mkdir("results/fakedata", S_IRWXU);
        }
        static int dd = 0;
        char fp_filename[6][128];
        sprintf(fp_filename[0], "results/fakedata/m1_scores_%d.dat", dd);
        sprintf(fp_filename[1], "results/fakedata/m2_scores_%d.dat", dd);
        sprintf(fp_filename[2], "results/fakedata/m3_scores_%d.dat", dd);
        sprintf(fp_filename[3], "results/fakedata/m1_boxes_%d.dat", dd);
        sprintf(fp_filename[4], "results/fakedata/m2_boxes_%d.dat", dd);
        sprintf(fp_filename[5], "results/fakedata/m3_boxes_%d.dat", dd);
    #if A2_SDK//load postprocess data.
        FILE *m1_scores_fp = fopen(fp_filename[0], "rb");
        fread((void*)m1_scores, sizeof(float), m1_c * m1_w * m1_h, m1_scores_fp);
        fclose(m1_scores_fp);
        FILE *m2_scores_fp = fopen(fp_filename[1], "rb+");
        fread((void*)m2_scores, sizeof(float), m2_c * m2_w * m2_h, m2_scores_fp);
        fclose(m2_scores_fp);
        FILE *m3_scores_fp = fopen(fp_filename[2], "rb+");
        fread((void*)m3_scores, sizeof(float), m3_c * m3_w * m3_h, m3_scores_fp);
        fclose(m3_scores_fp);

        FILE *m1_bbox_deltas_fp = fopen(fp_filename[3], "rb+");
        fread(m1_bbox_deltas, sizeof(float), b1_c * b1_w * b1_h, m1_bbox_deltas_fp);
        fclose(m1_bbox_deltas_fp);
        FILE *m2_bbox_deltas_fp = fopen(fp_filename[4], "rb+");
        fread(m2_bbox_deltas, sizeof(float), b2_c * b2_w * b2_h, m2_bbox_deltas_fp);
        fclose(m2_bbox_deltas_fp);
        FILE *m3_bbox_deltas_fp = fopen(fp_filename[5], "rb+");
        fread(m3_bbox_deltas, sizeof(float), b3_c * b3_w * b3_h, m3_bbox_deltas_fp);
        fclose(m3_bbox_deltas_fp);
    #else //dump postprocess data
        dd++;
        FILE *m1_scores_fp = fopen(fp_filename[0], "wb+");
        fwrite(m1_scores, sizeof(float), m1_c * m1_w * m1_h, m1_scores_fp);
        fclose(m1_scores_fp);
        FILE *m2_scores_fp = fopen(fp_filename[1], "wb+");
        fwrite(m2_scores, sizeof(float), m2_c * m2_w * m2_h, m2_scores_fp);
        fclose(m2_scores_fp);
        FILE *m3_scores_fp = fopen(fp_filename[2], "wb+");
        fwrite(m3_scores, sizeof(float), m3_c * m3_w * m3_h, m3_scores_fp);
        fclose(m3_scores_fp);

        FILE *m1_bbox_deltas_fp = fopen(fp_filename[3], "wb+");
        fwrite(m1_bbox_deltas, sizeof(float), b1_c * b1_w * b1_h, m1_bbox_deltas_fp);
        fclose(m1_bbox_deltas_fp);
        FILE *m2_bbox_deltas_fp = fopen(fp_filename[4], "wb+");
        fwrite(m2_bbox_deltas, sizeof(float), b2_c * b2_w * b2_h, m2_bbox_deltas_fp);
        fclose(m2_bbox_deltas_fp);
        FILE *m3_bbox_deltas_fp = fopen(fp_filename[5], "wb+");
        fwrite(m3_bbox_deltas, sizeof(float), b3_c * b3_w * b3_h, m3_bbox_deltas_fp);
        fclose(m3_bbox_deltas_fp);
    #endif
    #endif

    #if PLD
        std::cout<<"dump face_detector m3_bbox_deltas outputs:"<<std::endl;
        for(int kk = 0; kk < 10; kk++){
            std::cout<<*(m3_bbox_deltas+kk) << " ";
        }
        std::cout<<std::endl;
    #endif
    for (int n = 0; n < image_n; n++) {
        int                   width  = frame_info.frames[n].width;
        int                   height = frame_info.frames[n].height;
        std::vector<bm::NetOutputObject> proposals;
        proposals.clear();
        generate_proposal(m3_scores + (m3_c * n + anchor_num_) * m3_h * m3_w,
                          m3_bbox_deltas + b3_c * b3_h * b3_w * n,
                          16.0,
                          4,
                          m3_w,
                          m3_h,
                          width,
                          height,
                          proposals);
        generate_proposal(m2_scores + (m2_c * n + anchor_num_) * m2_h * m2_w,
                          m2_bbox_deltas + b2_c * b2_h * b2_w * n,
                          4.0,
                          2,
                          m2_w,
                          m2_h,
                          width,
                          height,
                          proposals);
        generate_proposal(m1_scores + (m1_c * n + anchor_num_) * m1_h * m1_w,
                          m1_bbox_deltas + b1_c * b1_h * b1_w * n,
                          1.0,
                          1,
                          m1_w,
                          m1_h,
                          width,
                          height,
                          proposals);
        std::vector<bm::NetOutputObject> nmsProposals;
        nmsProposals.clear();
        nms(proposals, nmsProposals);

        std::vector<bm::NetOutputObject> faceRects;
        faceRects.clear();
        std::vector<bmcv_rect_t> bm_rects;
    #if PLD
        std::cout<<"nmsProposals size: "<<nmsProposals.size()<<std::endl;
    #endif
        for (size_t i = 0; i < nmsProposals.size(); ++i) {
            bm::NetOutputObject rect = nmsProposals[i];
            if (rect.score >= 0.5 && i < 16){
                faceRects.push_back(rect);
                bmcv_rect_t bm_rect{rect.x1, rect.y1, rect.x2 - rect.x1, rect.y2 - rect.y1};
                bm_rects.push_back(bm_rect);
            #if PLD
                std::cout<<"face_detector rect: "<<rect.x1<<" "<<rect.y1<<" "<<rect.x2<<" "<<rect.y2<<std::endl;
            #endif
            }
        }
        #if DRAW_DETECTOR
        if(bm_rects.size() > 0){
            bmcv_image_draw_rectangle(bmctx_->handle(), frame_info.frames[n].original, bm_rects.size(), bm_rects.data(), 2, 255, 255, 0);
        }
            uint8_t *jpeg_data=NULL;
            size_t out_size = 0;
            int ret = bmcv_image_jpeg_enc(bmctx_->handle(), 1, &frame_info.frames[n].original, (void**)&jpeg_data, &out_size, 85);
            if (ret == BM_SUCCESS) {
                static int ii = 0;
                std::string img_file = "results/drawed_frame_" + std::to_string(ii++) + ".jpg";
                FILE *fp = fopen(img_file.c_str(), "wb");
                std::cout<<"==========================="<<std::endl;
                std::cout<<"==Drawing detector frame" << ii << "=="<<std::endl;
                std::cout<<"==========================="<<std::endl;
                fwrite(jpeg_data, out_size, 1, fp);
                fclose(fp);
            }
            free(jpeg_data);
            bm_image_destroy_allinone(&frame_info.frames[n].original);
        #endif
        frame_info.out_datums.push_back(bm::NetOutputDatum(faceRects));
        //std::cout << "Image idx=" << n << " final predict " << faceRects.size() << " bboxes" << std::endl;
    }

    return 0;
}

void FaceDetector::generate_proposal(const float *          scores,
                                        const float *          bbox_deltas,
                                        const float            scale_factor,
                                        const int              feat_factor,
                                        const int              feat_w,
                                        const int              feat_h,
                                        const int              width,
                                        const int              height,
                                        std::vector<bm::NetOutputObject> &proposals)
{
    std::vector<bm::NetOutputObject> m_proposals;
    float                 anchor_cx   = (base_size_ - 1) * 0.5;
    float                 anchor_cy   = (base_size_ - 1) * 0.5;
    int                   feat_stride = feat_stride_ * feat_factor;

    for (int s = 0; s < anchor_scales_.size(); ++s) {
        float scale = anchor_scales_[s] * scale_factor;
        for (int h = 0; h < feat_h; ++h) {
            for (int w = 0; w < feat_w; ++w) {
                int      delta_index = h * feat_w + w;
                bm::NetOutputObject facerect;
                facerect.score = scores[s * feat_w * feat_h + delta_index];
                if (facerect.score <= base_threshold_)
                    continue;
                float anchor_size = scale * base_size_;
                float bbox_x1 =
                        anchor_cx - (anchor_size - 1) * 0.5 + w * feat_stride;
                float bbox_y1 =
                        anchor_cy - (anchor_size - 1) * 0.5 + h * feat_stride;
                float bbox_x2 =
                        anchor_cx + (anchor_size - 1) * 0.5 + w * feat_stride;
                float bbox_y2 =
                        anchor_cy + (anchor_size - 1) * 0.5 + h * feat_stride;

                float bbox_w  = bbox_x2 - bbox_x1 + 1;
                float bbox_h  = bbox_y2 - bbox_y1 + 1;
                float bbox_cx = bbox_x1 + 0.5 * bbox_w;
                float bbox_cy = bbox_y1 + 0.5 * bbox_h;
                float dx =
                        bbox_deltas[(s * 4 + 0) * feat_h * feat_w + delta_index];
                float dy =
                        bbox_deltas[(s * 4 + 1) * feat_h * feat_w + delta_index];
                float dw =
                        bbox_deltas[(s * 4 + 2) * feat_h * feat_w + delta_index];
                float dh =
                        bbox_deltas[(s * 4 + 3) * feat_h * feat_w + delta_index];
                float pred_cx = dx * bbox_w + bbox_cx;
                float pred_cy = dy * bbox_h + bbox_cy;
                float pred_w  = std::exp(dw) * bbox_w;
                float pred_h  = std::exp(dh) * bbox_h;
                facerect.x1 =
                        std::max(std::min(static_cast<double>(width - 1),
                                          (pred_cx - 0.5 * pred_w) / img_x_scale_),
                                 0.0);
                facerect.y1 =
                        std::max(std::min(static_cast<double>(height - 1),
                                          (pred_cy - 0.5 * pred_h) / img_y_scale_),
                                 0.0);
                facerect.x2 =
                        std::max(std::min(static_cast<double>(width - 1),
                                          (pred_cx + 0.5 * pred_w) / img_x_scale_),
                                 0.0);
                facerect.y2 =
                        std::max(std::min(static_cast<double>(height - 1),
                                          (pred_cy + 0.5 * pred_h) / img_y_scale_),
                                 0.0);
                if ((facerect.x2 - facerect.x1 + 1 < min_size_) ||
                    (facerect.y2 - facerect.y1 + 1 < min_size_))
                    continue;
                m_proposals.push_back(facerect);
            }
        }
    }
    std::sort(m_proposals.begin(), m_proposals.end(), compareBBox);

    int keep = m_proposals.size();
    if (per_nms_topn_ < keep)
        keep = per_nms_topn_;

    if (keep > 0) {
        proposals.insert(
                proposals.end(), m_proposals.begin(), m_proposals.begin() + keep);
    }
}


void FaceDetector::nms(const std::vector<bm::NetOutputObject> &proposals,
                          std::vector<bm::NetOutputObject>&      nmsProposals)
{
    if (proposals.empty()) {
        nmsProposals.clear();
        return;
    }
    std::vector<bm::NetOutputObject> bboxes = proposals;
    std::sort(bboxes.begin(), bboxes.end(), compareBBox);

    int              select_idx = 0;
    int              num_bbox   = bboxes.size();
    std::vector<int> mask_merged(num_bbox, 0);
    bool             all_merged = false;
    while (!all_merged) {
        while (select_idx < num_bbox && 1 == mask_merged[select_idx])
            ++select_idx;

        if (select_idx == num_bbox) {
            all_merged = true;
            continue;
        }
        nmsProposals.push_back(bboxes[select_idx]);
        mask_merged[select_idx] = 1;
        bm::NetOutputObject select_bbox    = bboxes[select_idx];
        float    area1          = (select_bbox.x2 - select_bbox.x1 + 1) *
                                  (select_bbox.y2 - select_bbox.y1 + 1);
        ++select_idx;
        for (int i = select_idx; i < num_bbox; ++i) {
            if (mask_merged[i] == 1)
                continue;
            bm::NetOutputObject &bbox_i = bboxes[i];
            float     x      = std::max(select_bbox.x1, bbox_i.x1);
            float     y      = std::max(select_bbox.y1, bbox_i.y1);
            float     w      = std::min(select_bbox.x2, bbox_i.x2) - x + 1;
            float     h      = std::min(select_bbox.y2, bbox_i.y2) - y + 1;
            if (w <= 0 || h <= 0)
                continue;
            float area2 =
                    (bbox_i.x2 - bbox_i.x1 + 1) * (bbox_i.y2 - bbox_i.y1 + 1);
            float area_intersect = w * h;
            // Union method
            if (area_intersect / (area1 + area2 - area_intersect) >
                nms_threshold_)
                mask_merged[i] = 1;
        }
    }
}
