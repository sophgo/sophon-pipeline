//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "ppyoloe.h"
#include <algorithm>

#define USE_MULTICLASS_NMS 1

#define NUM_OUTPUTS 2
#define BBOX_DIM 4

PPYoloe::PPYoloe(bm::BMNNContextPtr bmctx, std::string model_type):m_bmctx(bmctx)
{
    // the bmodel has only one yolo network.
    auto net_name = m_bmctx->network_name(0);
    m_bmnet = std::make_shared<bm::BMNNNetwork>(m_bmctx->bmrt(), net_name);
    assert(m_bmnet != nullptr);
    //assert(m_bmnet->inputTensorNum() == 1);
    auto tensor = m_bmnet->inputTensor(0);

    //YOLOV5 input is NCHW
    m_net_h = tensor->get_shape()->dims[2];
    m_net_w = tensor->get_shape()->dims[3];

    MAX_BATCH = tensor->get_shape()->dims[0];

    init_yolo(model_type);
}

PPYoloe::~PPYoloe()
{

}

// init anchors by .json for yolov[X]
int PPYoloe::init_yolo(std::string model_type = "ppyoloe"){
    int ret = 0;

    std::transform( model_type.begin(), model_type.end(), model_type.begin(), ::tolower );

    std::vector <float> rgb_mean;
    std::vector <float> rgb_std;
    float scale;

    m_alpha = std::vector <float> (3);
    m_beta = std::vector <float> (3);

    if (model_type == "ppyoloe"){
        rgb_mean = {0.485, 0.456, 0.406};
        rgb_std = {0.229, 0.224, 0.225};
        scale = 1.f / 255;
    }
    else if (model_type == "ppyoloe_plus"){
        rgb_mean = {0.f, 0.f, 0.f};
        rgb_std = {1.f, 1.f, 1.f};
        scale = 1.f / 255;
    }
    else{
        std::cerr << "Unsupported model_type: " << model_type << std::endl;
        assert(
            (model_type == "ppyoloe") or (model_type == "ppyoloe_plus")
        );
    }

    for (int i = 0; i < rgb_mean.size(); i++){
        m_alpha[i] = scale / rgb_std[i]; 
        m_beta[i] = - rgb_mean[i] * scale / rgb_std[i];
    }

    // ppyoloe 
    int output_num = m_bmnet->outputTensorNum();
    if (output_num == NUM_OUTPUTS){
        // bboxes out: [bs, num, 4]
        // score out: [bs, num_classes, num]
        m_outputs_order.resize(NUM_OUTPUTS); // 0:bbox, 1:score
        auto tensor = m_bmnet->outputTensor(0);
        int num = tensor->get_shape()->dims[2];
        if (num == BBOX_DIM){
            m_outputs_order[0] = 0;
            m_outputs_order[1] = 1;
        }
        else {
            m_outputs_order[0] = 1;
            m_outputs_order[1] = 0;
        }

    }
    else{
        std::cerr << "Unsupported output num: " << output_num << std::endl;
                assert(output_num ==NUM_OUTPUTS);
    }

    return ret;

}

int PPYoloe::get_Batch(){
    return MAX_BATCH;
}

int PPYoloe::preprocess(std::vector<bm::FrameBaseInfo>& frames, std::vector<bm::FrameInfo>& frame_infos)
{
    int ret = 0;
    bm_handle_t handle = m_bmctx->handle();

    // Check input
    int total = frames.size();
    int left = (total%MAX_BATCH == 0 ? MAX_BATCH: total%MAX_BATCH);
    int batch_num = total%MAX_BATCH==0 ? total/MAX_BATCH: (total/MAX_BATCH + 1);
    for(int batch_idx = 0; batch_idx < batch_num; ++ batch_idx) {
        int num = MAX_BATCH;
        int start_idx = batch_idx*MAX_BATCH;
        if (batch_idx == batch_num-1) {
            // last one
            num = left;
        }

        std::vector <float> scale_factor(MAX_BATCH * 2, 1.f);

        auto scaleTensorPtr = m_bmnet->inputTensor(1);
        bm_tensor_t scale_tensor = *scaleTensorPtr->bm_tensor();

        bm::FrameInfo finfo;
        //1. Resize
        bm_image resized_imgs[MAX_BATCH];
        ret = bm::BMImage::create_batch(handle, m_net_h, m_net_w, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, resized_imgs, num, 64);
        assert(BM_SUCCESS == ret);

        for(int i = 0;i < num; ++i) {
            bm_image image1;
            bm::BMImage::from_avframe(handle, frames[start_idx + i].avframe, image1, true);

            ret = bmcv_image_vpp_convert(handle, 1, image1, &resized_imgs[i]);
            assert(BM_SUCCESS == ret);

            // scale_factor
            scale_factor[i*2] = (float)m_net_h / image1.height * scaleTensorPtr->get_scale();
            scale_factor[i*2 + 1] = (float)m_net_w / image1.width * scaleTensorPtr->get_scale();

            // convert data to jpeg
            uint8_t *jpeg_data=NULL;
            size_t out_size = 0;
#if USE_QTGUI
            bmcv_image_jpeg_enc(handle, 1, &image1, (void**)&jpeg_data, &out_size);
#endif
            frames[start_idx + i].jpeg_data = std::make_shared<bm::Data>(jpeg_data, out_size);
            frames[start_idx + i].height= image1.height;
            frames[start_idx + i].width = image1.width;
            av_frame_unref(frames[start_idx + i].avframe);
            av_frame_free(&frames[start_idx + i].avframe);

            finfo.frames.push_back(frames[start_idx+i]);
            bm_image_destroy(image1);
        }

        //2. Convert to
        bm_image convertto_imgs[MAX_BATCH];
        bm_image_data_format_ext img_type;
        auto inputTensorPtr = m_bmnet->inputTensor(0);
        if (inputTensorPtr->get_dtype() == BM_INT8) {
            img_type = DATA_TYPE_EXT_1N_BYTE_SIGNED;
        }else{
            img_type = DATA_TYPE_EXT_FLOAT32;
        }

        ret = bm::BMImage::create_batch(handle, m_net_h, m_net_w, FORMAT_RGB_PLANAR, img_type, convertto_imgs, num, 1, false, true);
        assert(BM_SUCCESS == ret);

        bm_tensor_t input_tensor = *inputTensorPtr->bm_tensor();
        bm::bm_tensor_reshape_NCHW(handle, &input_tensor, num, 3, m_net_h, m_net_w);

        ret = bm_image_attach_contiguous_mem(num, convertto_imgs, input_tensor.device_mem);
        assert(BM_SUCCESS == ret);

        bmcv_convert_to_attr convert_to_attr;
        convert_to_attr.alpha_0 = m_alpha[0] * inputTensorPtr->get_scale();
        convert_to_attr.alpha_1 = m_alpha[1] * inputTensorPtr->get_scale();
        convert_to_attr.alpha_2 = m_alpha[2] * inputTensorPtr->get_scale();
        convert_to_attr.beta_0  = m_beta[0] * inputTensorPtr->get_scale();
        convert_to_attr.beta_1  = m_beta[1] * inputTensorPtr->get_scale();
        convert_to_attr.beta_2  = m_beta[0] * inputTensorPtr->get_scale();

        ret = bmcv_image_convert_to(m_bmctx->handle(), num, convert_to_attr, resized_imgs, convertto_imgs);
        assert(ret == 0);

        bm_image_dettach_contiguous_mem(num, convertto_imgs);

        finfo.input_tensors.push_back(input_tensor);

        // feed factor_scale
        bm_malloc_device_byte(m_bmctx->handle(), &scale_tensor.device_mem, scale_factor.size() * sizeof(float));
        bm_memcpy_s2d_partial(
            m_bmctx->handle(), 
            scale_tensor.device_mem, 
            (void*)scale_factor.data(), 
            scale_factor.size() * sizeof(float)
        );

        finfo.input_tensors.push_back(scale_tensor);

        bm::BMImage::destroy_batch(resized_imgs, num);
        bm::BMImage::destroy_batch(convertto_imgs, num);

        frame_infos.push_back(finfo);
    }

    return ret;
}

int PPYoloe::forward(std::vector<bm::FrameInfo>& frame_infos)
{
    int ret = 0;
    for(int b = 0; b < frame_infos.size(); ++b) {
        for (int i = 0; i < m_bmnet->outputTensorNum(); ++i) {
            bm_tensor_t tensor;
            frame_infos[b].output_tensors.push_back(tensor);
        }

#if DUMP_FILE
        bm::BMImage::dump_dev_memory(bmctx_->handle(), frame_infos[b].input_tensors[0].device_mem, "convertto",
                frame_infos[b].frames.size(), m_net_h, m_net_w, false, false);
#endif
        ret = m_bmnet->forward(frame_infos[b].input_tensors.data(), frame_infos[b].input_tensors.size(),
                              frame_infos[b].output_tensors.data(), frame_infos[b].output_tensors.size());

        assert(BM_SUCCESS == ret);
    }

    return ret;
}

int PPYoloe::postprocess(std::vector<bm::FrameInfo> &frame_infos)
{
    for(int i=0;i < frame_infos.size(); ++i) {

        // Free AVFrames
        auto frame_info = frame_infos[i];

        // extract face detection
        extract_yolobox_cpu(frame_info);

        if (m_pfnDetectFinish != nullptr) {
            m_pfnDetectFinish(frame_info);
        }

        for(int j = 0; j < frame_info.frames.size(); ++j) {

            auto reff = frame_info.frames[j];
            assert(reff.avpkt != nullptr);
            av_packet_unref(reff.avpkt);
            av_packet_free(&reff.avpkt);

            assert(reff.avframe == nullptr);
            av_frame_unref(reff.avframe);
            av_frame_free(&reff.avframe);
        }

        // Free Tensors
        for(auto& tensor : frame_info.input_tensors) {
            bm_free_device(m_bmctx->handle(), tensor.device_mem);
        }

        for(auto& tensor: frame_info.output_tensors) {
            bm_free_device(m_bmctx->handle(), tensor.device_mem);
        }

    }
    return 0;
}

void PPYoloe::NMS(bm::NetOutputObjects &dets, float nmsConfidence)
{
    int length = dets.size();
    int index = length - 1;

    std::sort(dets.begin(), dets.end(), [](const bm::NetOutputObject& a, const bm::NetOutputObject& b) {
        return a.score < b.score;
    });

    std::vector<float> areas(length);
    for (int i=0; i<length; i++)
    {
        areas[i] = dets[i].width() * dets[i].height();
    }

    while (index  > 0)
    {
        int i = 0;
        while (i < index)
        {
            float left    = std::max(dets[index].x1,   dets[i].x1);
            float top     = std::max(dets[index].y1,    dets[i].y1);
            float right   = std::min(dets[index].x1 + dets[index].width(),  dets[i].x1 + dets[i].width());
            float bottom  = std::min(dets[index].y1 + dets[index].height(), dets[i].y1 + dets[i].height());
            float overlap = std::max(0.0f, right - left) * std::max(0.0f, bottom - top);
            if (overlap / (areas[index] + areas[i] - overlap) > nmsConfidence)
            {
                areas.erase(areas.begin() + i);
                dets.erase(dets.begin() + i);
                index --;
            }
            else
            {
                i++;
            }
        }
        index--;
    }
}

void PPYoloe::extract_yolobox_cpu(bm::FrameInfo& frameInfo)
{
    std::vector<bm::NetOutputObject> yolobox_vec;
    std::vector<cv::Rect> bbox_vec;

    int order[2] = {0, 1};

    auto& images = frameInfo.frames;
    for(int batch_idx = 0; batch_idx < (int)images.size(); ++ batch_idx)
    {
        yolobox_vec.clear();
        auto& frame = images[batch_idx];

        int output_num = m_bmnet->outputTensorNum();

        //ppyoloe
        if (output_num == NUM_OUTPUTS){
            bm::BMNNTensor bboxes_output_tensor(
                m_bmctx->handle(), "", 
                m_bmnet->get_output_scale(m_outputs_order[0]), 
                &frameInfo.output_tensors[m_outputs_order[0]]
            );
            bm::BMNNTensor scores_output_tensor(
                m_bmctx->handle(), "", 
                m_bmnet->get_output_scale(m_outputs_order[1]), 
                &frameInfo.output_tensors[m_outputs_order[1]]
            );

            int box_num = bboxes_output_tensor.get_shape()->dims[1];
            int nout = bboxes_output_tensor.get_shape()->dims[2];
            m_class_num = scores_output_tensor.get_shape()->dims[1];

            float *bboxes_output_data = (float*)bboxes_output_tensor.get_cpu_data() + batch_idx*box_num*nout;
            float *scores_output_data = (float*)scores_output_tensor.get_cpu_data() + batch_idx*box_num*m_class_num;
            for (int i = 0; i < box_num; i++) {
                float max_value = 0.0;
                int max_index = 0;
                for (int j = 0; j < m_class_num; j++){
                    float cur_value = scores_output_data[i + j*box_num];
                    if (cur_value > max_value) {
                        max_value = cur_value;
                        max_index = j;
                    }
                }
                
                if (max_value >= m_obj_thres){
                    bm::NetOutputObject box;
                    box.score = max_value;
                    box.x1 = bboxes_output_data[i * nout + 0];
                    box.y1 = bboxes_output_data[i * nout + 1];
                    box.x2 = bboxes_output_data[i * nout + 2];
                    box.y2 = bboxes_output_data[i * nout + 3];
                    box.class_id = max_index;
                    yolobox_vec.push_back(box);
                }
            }
        }
        

#if USE_MULTICLASS_NMS
        std::vector<bm::NetOutputObjects> class_vec(m_class_num);
        for (auto& box : yolobox_vec){
        class_vec[box.class_id].push_back(box);
        }
        for (auto& cls_box : class_vec){
        NMS(cls_box, m_nms_thres);
        }
        yolobox_vec.clear();
        for (auto& cls_box : class_vec){
        yolobox_vec.insert(yolobox_vec.end(), cls_box.begin(), cls_box.end());
        }
#else
        NMS(yolobox_vec, m_nms_thres);
#endif
        bm::NetOutputDatum datum(yolobox_vec);
        frameInfo.out_datums.push_back(datum);
    }

}
