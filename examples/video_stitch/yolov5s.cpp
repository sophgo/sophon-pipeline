//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolov5s.h"
#define USE_ASPECT_RATIO 1
#define USE_MULTICLASS_NMS 1

YoloV5::YoloV5(bm::BMNNContextPtr bmctx, int start_chan, int chan_num, 
    int class_num):m_bmctx(bmctx), m_class_num(class_num)
{
    // the bmodel has only one yolo network.
    auto net_name = m_bmctx->network_name(0);
    m_bmnet = std::make_shared<bm::BMNNNetwork>(m_bmctx->bmrt(), net_name);
    assert(m_bmnet != nullptr);
    assert(m_bmnet->inputTensorNum() == 1);
    auto tensor = m_bmnet->inputTensor(0);

    //YOLOV5 input is NCHW
    m_net_h = tensor->get_shape()->dims[2];
    m_net_w = tensor->get_shape()->dims[3];
    MAX_BATCH = tensor->get_shape()->dims[0];

    for (int i = start_chan; i < start_chan + chan_num; ++i) {
        m_trackerPerChanel.insert(std::make_pair(i, bm::BMTracker::create()));
    }
    
}

YoloV5::~YoloV5()
{

}

int YoloV5::get_Batch(){
    return MAX_BATCH;
}

int YoloV5::preprocess(std::vector<bm::FrameBaseInfo>& frames, std::vector<bm::FrameInfo>& frame_infos)
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

        bm::FrameInfo finfo;
        finfo.handle = handle;
        //1. Resize
        bm_image resized_imgs[MAX_BATCH];
        ret = bm::BMImage::create_batch(handle, m_net_h, m_net_w, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, resized_imgs, num, 64);
        assert(BM_SUCCESS == ret);

        for(int i = 0;i < num; ++i) {
            bm_image image1;
           // bm::BMImage::from_avframe(handle, frames[start_idx + i].avframe, image1, false);
            image1 =  frames[start_idx + i].original;
#if USE_ASPECT_RATIO
            bool isAlignWidth = false;
            float ratio = get_aspect_scaled_ratio(image1.width, image1.height, m_net_w, m_net_h, &isAlignWidth);
            bmcv_padding_atrr_t padding_attr;
            memset(&padding_attr, 0, sizeof(padding_attr));

            padding_attr.padding_b = 114;
            padding_attr.padding_g = 114;
            padding_attr.padding_r = 114;
            padding_attr.if_memset = 1;
            if (isAlignWidth) {
            padding_attr.dst_crop_h = image1.height*ratio;
            padding_attr.dst_crop_w = m_net_w;

            int ty1 = (int)((m_net_h - padding_attr.dst_crop_h) / 2);
            padding_attr.dst_crop_sty = ty1;
            padding_attr.dst_crop_stx = 0;
            }else{
            padding_attr.dst_crop_h = m_net_h;
            padding_attr.dst_crop_w = image1.width*ratio;

            int tx1 = (int)((m_net_w - padding_attr.dst_crop_w) / 2);
            padding_attr.dst_crop_sty = 0;
            padding_attr.dst_crop_stx = tx1;
            }

            bmcv_rect_t crop_rect{0, 0, image1.width, image1.height};
            ret = bmcv_image_vpp_convert_padding(handle, 1, image1, &resized_imgs[i],
                &padding_attr, &crop_rect);
#else
            ret = bmcv_image_vpp_convert(handle, 1, image1, &resized_imgs[i]);
#endif
            assert(BM_SUCCESS == ret);

            frames[start_idx + i].height = frames[start_idx + i].original.height;
            frames[start_idx + i].width  = frames[start_idx + i].original.width;
            finfo.frames.push_back(frames[start_idx+i]);
            //bm_image_destroy(image1);
        }

        //2. Convert to
        bm_image convertto_imgs[MAX_BATCH];
        float alpha, beta;

        bm_image_data_format_ext img_type = DATA_TYPE_EXT_FLOAT32;
        auto inputTensorPtr = m_bmnet->inputTensor(0);
        if (inputTensorPtr->get_dtype() == BM_INT8) {
            img_type = DATA_TYPE_EXT_1N_BYTE_SIGNED;
            alpha            = inputTensorPtr->get_scale() * 1.0 / 255;
            beta             = 0.0;
            img_type = (DATA_TYPE_EXT_1N_BYTE_SIGNED);
        }else{
            alpha            = 1.0/255;
            beta             = 0.0;
            img_type = DATA_TYPE_EXT_FLOAT32;
        }

        ret = bm::BMImage::create_batch(handle, m_net_h, m_net_w, FORMAT_RGB_PLANAR, img_type, convertto_imgs, num, 1, false, true);
        assert(BM_SUCCESS == ret);

        bm_tensor_t input_tensor = *inputTensorPtr->bm_tensor();
        bm::bm_tensor_reshape_NCHW(handle, &input_tensor, num, 3, m_net_h, m_net_w);

        ret = bm_image_attach_contiguous_mem(num, convertto_imgs, input_tensor.device_mem);
        assert(BM_SUCCESS == ret);

        bmcv_convert_to_attr convert_to_attr;
        convert_to_attr.alpha_0 = alpha;
        convert_to_attr.alpha_1 = alpha;
        convert_to_attr.alpha_2 = alpha;
        convert_to_attr.beta_0  = beta;
        convert_to_attr.beta_1  = beta;
        convert_to_attr.beta_2  = beta;

        ret = bmcv_image_convert_to(m_bmctx->handle(), num, convert_to_attr, resized_imgs, convertto_imgs);
        assert(ret == 0);

        bm_image_dettach_contiguous_mem(num, convertto_imgs);

        finfo.input_tensors.push_back(input_tensor);

        bm::BMImage::destroy_batch(resized_imgs, num);
        bm::BMImage::destroy_batch(convertto_imgs, num);

        frame_infos.push_back(finfo);
    }
    return 0;
}

int YoloV5::forward(std::vector<bm::FrameInfo>& frame_infos)
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

    return 0;
}

int YoloV5::postprocess(std::vector<bm::FrameInfo> &frame_infos)
{
    for(int i=0;i < frame_infos.size(); ++i) {

        // Free AVFrames
        auto &frame_info = frame_infos[i];

        // extract face detection
        extract_yolobox_cpu(frame_info);

        if (m_pfnDetectFinish != nullptr) {
            m_pfnDetectFinish(frame_info);
        }
        

    }
    return 0;
}

float YoloV5::sigmoid(float x)
{
    return 1.0 / (1 + expf(-x));
}

int YoloV5::argmax(float* data, int num) {
    float max_value = 0.0;
    int max_index = 0;
    for(int i = 0; i < num; ++i) {
        float sigmoid_value = sigmoid(data[i]);
        if (sigmoid_value > max_value) {
            max_value = sigmoid_value;
            max_index = i;
        }
    }

    return max_index;
}

float YoloV5::get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h, bool *pIsAligWidth)
{
    float ratio;
    float r_w = (float)dst_w / src_w;
    float r_h = (float)dst_h / src_h;
    if (r_h > r_w){
        *pIsAligWidth = true;
        ratio = r_w;
    }
    else{
        *pIsAligWidth = false;
        ratio = r_h;
    }
    return ratio;
}


void YoloV5::NMS(bm::NetOutputObjects &dets, float nmsConfidence)
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

void YoloV5::extract_yolobox_cpu(bm::FrameInfo& frameInfo)
{
    std::vector<bm::NetOutputObject> yolobox_vec;
    auto& images = frameInfo.frames;
    for(int batch_idx = 0; batch_idx < (int)images.size(); ++ batch_idx)
    {
        yolobox_vec.clear();
        auto& frame = images[batch_idx];
        int frame_width = frame.width;
        int frame_height = frame.height;
        int tx1 = 0, ty1 = 0;
#if USE_ASPECT_RATIO
        bool isAlignWidth = false;
        float ratio = get_aspect_scaled_ratio(frame.width, frame.height, m_net_w, m_net_h, &isAlignWidth);

        if (isAlignWidth) {
            ty1 = (int)((m_net_h - (int)(frame.height*ratio)) / 2);
        }else{
            tx1 = (int)((m_net_w - (int)(frame.width*ratio)) / 2);
        }
#endif

        int output_num = m_bmnet->outputTensorNum();
        int nout = m_class_num + 5;

        if (output_num == 1) {
            bm::BMNNTensor output_tensor(m_bmctx->handle(), "", m_bmnet->get_output_scale(0), &frameInfo.output_tensors[0]);
            int box_num = output_tensor.get_shape()->dims[1];
            float *output_data = (float*)output_tensor.get_cpu_data() + batch_idx*box_num*nout;
            for (int i = 0; i < box_num; i++) {
                float *ptr = output_data + i * nout;
                float score = ptr[4];
                if (score >= m_cls_thres) {
                    int class_id = argmax(&ptr[5], m_class_num);
                    float confidence = ptr[class_id + 5];
                    if (confidence * score >= m_obj_thres) {
                        bm::NetOutputObject box;
                        box.score = confidence * score;

                        float centerX = (ptr[0]+1 - tx1)/ratio -1;
                        float centerY = (ptr[1]+1 - ty1)/ratio -1;
                        float width = (ptr[2]+0.5) / ratio;
                        float height = (ptr[3]+0.5) / ratio;
                        box.x1  = int(centerX - width  / 2);
                        box.y1  = int(centerY - height / 2);
                        box.x2  = box.x1 + width;
                        box.y2  = box.y1 + height;
                        box.class_id = class_id;
                        yolobox_vec.push_back(box);
                    }
                }
            }
        } else if (output_num == 3){
            for(int tidx = 0; tidx < output_num; ++tidx) {
                bm::BMNNTensor output_tensor(m_bmctx->handle(), "", m_bmnet->get_output_scale(tidx), &frameInfo.output_tensors[tidx]);
                int feat_h = output_tensor.get_shape()->dims[2];
                int feat_w = output_tensor.get_shape()->dims[3];
                int area = feat_h * feat_w;
                float *output_data = (float*)output_tensor.get_cpu_data() + batch_idx*3*area*nout;
                for (int anchor_idx = 0; anchor_idx < m_anchor_num; anchor_idx++)
                {
                    int feature_size = feat_h*feat_w*nout;
                    float *ptr = output_data + anchor_idx*feature_size;
                    for (int i = 0; i < area; i++) {
                        float score = sigmoid(ptr[4]);
                        if (score >= m_cls_thres) {
                            int class_id = argmax(&ptr[5], m_class_num);
                            float confidence = sigmoid(ptr[class_id + 5]);
                            if (confidence * score > m_obj_thres) {
                                float centerX = (sigmoid(ptr[0]) * 2 - 0.5 + i % feat_w) * m_net_w / feat_w;
                                float centerY = (sigmoid(ptr[1]) * 2 - 0.5 + i / feat_w) * m_net_h / feat_h; //center_y
                                centerX = (centerX - tx1)/ratio -1;
                                centerY = (centerY - ty1)/ratio -1;
                                float width   = pow((sigmoid(ptr[2]) * 2), 2) * m_anchors[tidx][anchor_idx][0] / ratio; //w
                                float height  = pow((sigmoid(ptr[3]) * 2), 2) * m_anchors[tidx][anchor_idx][1] / ratio; //h
                                bm::NetOutputObject box;
                                box.x1  = int(centerX - width  / 2);
                                box.y1  = int(centerY - height / 2);
                                box.x2  = box.x1 + width;
                                box.y2  = box.y1 + height;
                                
                                box.score = confidence * score;
                                box.class_id = class_id;

                                yolobox_vec.push_back(box);
                            }
                        }
                        ptr += (m_class_num + 5);
                    }
                }
            } // end of tidx
        } else {
            std::cerr << "Unsupported yolo ouput layer num: " << output_num << std::endl;
            assert(output_num == 1 || output_num == 3);
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
    return;

}

int YoloV5::track(std::vector<bm::FrameInfo> &frames) {
    for (int i = 0; i < frames.size(); ++i) {
        auto &frame_info = frames[i];
        for (int j = 0; j < frame_info.frames.size(); ++j) {
            // tracker
            if (frame_info.out_datums[j].obj_rects.size() > 0) {

                if (m_trackerPerChanel.count(frame_info.frames[j].chan_id) != 0) {
                    m_trackerPerChanel[frame_info.frames[j].chan_id]->update(
                        frame_info.out_datums[j].obj_rects, frame_info.out_datums[j].track_rects);
                   // std::cout << frame_info.out_datums[j].track_rects.size() << std::endl;
                } else {
                    std::cerr << "unknown channel id " << frame_info.frames[j].chan_id << " when tracking" << std::endl;
                }               
            }

//            auto reff = frame_info.frames[j];
//            assert(reff.avpkt != nullptr);
//            av_packet_unref(reff.avpkt);
//            av_packet_free(&reff.avpkt);

            //assert(reff.avframe == nullptr);
//            av_frame_unref(reff.avframe);
//            av_frame_free(&reff.avframe);
        }
        
        // Free Tensors
        for(auto& tensor : frame_info.input_tensors) {
            if (tensor.device_mem.size == 0)
                continue;
            bm_free_device(m_bmctx->handle(), tensor.device_mem);
            memset(&tensor.device_mem, 0, sizeof(tensor.device_mem));
        }

        for(auto& tensor: frame_info.output_tensors) {
            if (tensor.device_mem.size == 0)
                continue;
            bm_free_device(m_bmctx->handle(), tensor.device_mem);
            memset(&tensor.device_mem, 0, sizeof(tensor.device_mem));
        }

        if (m_nextMediaPipe) {
            m_nextMediaPipe->push_frame(frame_info);
        }
    }
    return 0;
}
