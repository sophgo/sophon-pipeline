//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolov5s.h"
#include <algorithm>
#include <unistd.h>
#define USE_ASPECT_RATIO 1
#define USE_MULTICLASS_NMS 1
#define BM1684_CHIPID_BIT_MASK (0X1 << 1)
#define BM1686_CHIPID_BIT_MASK (0X1 << 2)

YoloV5::YoloV5(bm::BMNNContextPtr bmctx, std::string tpu_kernel_module_path, 
               std::string model_type):m_bmctx(bmctx), tpu_kernel_module_path(tpu_kernel_module_path)
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

    init_yolo(model_type);
}

YoloV5::~YoloV5()
{

}

// init anchors by .json for yolov[X]
int YoloV5::init_yolo(std::string model_type = "yolov5s"){
    int ret = 0;

    std::transform( model_type.begin(), model_type.end(), model_type.begin(), ::tolower );

    if (
        (bm::start_with(model_type, "yolov5")) || ((bm::start_with(model_type, "yolov7")))
        || (bm::start_with(model_type, "yolov6"))
    ){
        m_model_type = 0;
    }
    else if (bm::start_with(model_type, "yolov8")){
        // anchor free
        m_model_type = 1;
    }
    else{
        // add your self-define model_type here
        // and add implementation of post-processing, such as yolov3, yolov4
    }

    // yolov5s is the default 
    // "yolov5n", "yolov5s", "yolov5m", "yolov5l", "yolo5x"
    // "yolov7-tiny", "yolov7-tiny-silu"
    std::vector<std::vector<std::vector<float>>> anchors{{{10, 13}, {16, 30}, {33, 23}},
                                                         {{30, 61}, {62, 45}, {59, 119}},
                                                         {{116, 90}, {156, 198}, {373, 326}}};

    if ((model_type == "yolov7") || (model_type == "yolov7x")){
        anchors = {{{12, 16}, {19, 36}, {40, 28}},
                   {{36, 75}, {76, 55}, {72, 146}},
                   {{142, 110}, {192, 243}, {459, 401}}};

    }
    else if (model_type == "yolov7-w6"){
        // without test: 
        // "yolov7-E6", "yolov7-D6", "yolov7-E6E"
        // "yolov5n6", "yolov5s6", "yolov5m6", "yolov5l6", "yolov5x6"

        anchors = {{{19, 27}, {44, 40}, {38, 94}},
                   {{96, 68}, {86, 152}, {180, 137}},
                   {{140, 301}, {303, 264}, {238, 542}},
                   {{436, 615}, {739, 380}, {925, 792}}};

    }
    else{
        // add your self-define anchors here
    }
                                                         
    m_anchors = anchors;

#if USE_TPUKERNEL
    // 6.tpukernel postprocess
    bm_misc_info misc_info;
    bm_get_misc_info(m_bmctx->handle(), &misc_info);
    if(BM1686_CHIPID_BIT_MASK == misc_info.chipid_bit_mask && m_bmnet->outputTensor(0)->get_shape()->num_dims == 4){
        if(m_net_h < 32 || m_net_w < 32 || m_net_h % 32 != 0 || m_net_w % 32 != 0 || m_net_h > 2048 || m_net_w > 2048 || (m_net_h + m_net_w) > 3072){
            std::cerr << "Unsupported shape for tpukernel postprocession!" << std::endl;
            exit(1);
        }
        if(access(tpu_kernel_module_path.c_str(), F_OK) != 0){
            std::cerr << "Kernel module not exists." << std::endl;
            exit(1);
        }
        tpu_kernel_module_t tpu_module;
        tpu_module = tpu_kernel_load_module_file(m_bmctx->handle(), tpu_kernel_module_path.c_str());  
        func_id = tpu_kernel_get_function(m_bmctx->handle(), tpu_module, "tpu_kernel_api_yolov5_detect_out");
        std::cout << "Using tpu_kernel yolo postprocession, kernel funtion id: " << func_id << std::endl;
    }else if(m_bmnet->outputTensor(0)->get_shape()->num_dims == 3 || m_bmnet->outputTensor(0)->get_shape()->num_dims == 5){
        std::cout << "Using cpu yolo postprocession." << std::endl;
    }else if(BM1686_CHIPID_BIT_MASK != misc_info.chipid_bit_mask && m_bmnet->outputTensor(0)->get_shape()->num_dims == 4){
        std::cerr << "tpu_kernel yolo postprocession only support BM1684X!" << std::endl;
        exit(1);
    }else{
        std::cerr << "Invalid BModel Format!" << std::endl;
        exit(1);
    }
#else // USE_TPUKERNEL undefined in cmakelist
    if (m_bmnet->outputTensor(0)->get_shape()->num_dims == 4){
        std::cerr << "It seems you are using the tpukernel format bmodel, but you should follow these steps first." << std::endl
                  << "1. Please turn on the USE_TPU_KERNEL option in sophon-pipeline/CMakeLists.txt." << std::endl
                  << "2. Install libsophon with version >= 0.4.6 on your environment." << std::endl;
        exit(1);
    }
#endif

    return ret;
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
        //1. Resize
        bm_image resized_imgs[MAX_BATCH];
        ret = bm::BMImage::create_batch(handle, m_net_h, m_net_w, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, resized_imgs, num, 64);
        assert(BM_SUCCESS == ret);

        for(int i = 0;i < num; ++i) {
            bm_image image1;
            bm::BMImage::from_avframe(handle, frames[start_idx + i].avframe, image1, true);
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
            assert(BM_SUCCESS == ret);
#endif

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

    return ret;
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

    return ret;
}

int YoloV5::postprocess(std::vector<bm::FrameInfo> &frame_infos)
{
    for(int i=0;i < frame_infos.size(); ++i) {

        // Free AVFrames
        auto frame_info = frame_infos[i];

        // extract face detection
    #if USE_TPUKERNEL
        if(func_id != -1){
            extract_yolobox_tpukernel(frame_info);
        }else{
            extract_yolobox_cpu(frame_info);
        }
    #else
        extract_yolobox_cpu(frame_info);
    #endif

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
    std::vector<cv::Rect> bbox_vec;
    auto& images = frameInfo.frames;
    for(int batch_idx = 0; batch_idx < (int)images.size(); ++ batch_idx)
    {
        yolobox_vec.clear();
        auto& frame = images[batch_idx];
        float ratio_x = (float)m_net_w / frame.width;
        float ratio_y = (float)m_net_h / frame.height;
        int tx1 = 0, ty1 = 0;
        
#if USE_ASPECT_RATIO
        bool isAlignWidth = false;
        float ratio = get_aspect_scaled_ratio(frame.width, frame.height, m_net_w, m_net_h, &isAlignWidth);

        if (isAlignWidth) {
            ty1 = (int)((m_net_h - (int)(frame.height*ratio)) / 2);
        }else{
            tx1 = (int)((m_net_w - (int)(frame.width*ratio)) / 2);
        }
        ratio_x = ratio_y = ratio;
#endif

        int output_num = m_bmnet->outputTensorNum();


        if (m_model_type == 0){
            if (output_num == 1) {
                // 1 output: [bs, box_num, num_classes + 5]
                bm::BMNNTensor output_tensor(m_bmctx->handle(), "", m_bmnet->get_output_scale(0), &frameInfo.output_tensors[0]);
                int box_num = output_tensor.get_shape()->dims[1];
                int nout = output_tensor.get_shape()->dims[2];
                m_class_num = nout - 5;
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

                            float centerX = (ptr[0]+1 - tx1)/ratio_x -1;
                            float centerY = (ptr[1]+1 - ty1)/ratio_y -1;
                            float width = (ptr[2]+0.5) /ratio_x;
                            float height = (ptr[3]+0.5) /ratio_y;
                            box.x1  = int(centerX - width  / 2);
                            box.y1  = int(centerY - height / 2);
                            box.x2  = box.x1 + width;
                            box.y2  = box.y1 + height;
                            box.class_id = class_id;
                            yolobox_vec.push_back(box);
                        }
                    }
                }
            } else if ((output_num > 1) && (output_num <= 4)){
                // 3 output: [bs, num_out, num_feats, num_feats, num_classes + 5]
                for(int tidx = 0; tidx < output_num; ++tidx) {
                    bm::BMNNTensor output_tensor(m_bmctx->handle(), "", m_bmnet->get_output_scale(tidx), &frameInfo.output_tensors[tidx]);
                    int feat_h = output_tensor.get_shape()->dims[2];
                    int feat_w = output_tensor.get_shape()->dims[3];
                    int nout = output_tensor.get_shape()->dims[4];
                    m_class_num = nout - 5;
                    int area = feat_h * feat_w;
                    float *output_data = (float*)output_tensor.get_cpu_data() + batch_idx*3*area*nout;
                    for (int anchor_idx = 0; anchor_idx < m_anchors[tidx].size(); anchor_idx++)
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
                                    centerX = (centerX - tx1)/ratio_x-1;
                                    centerY = (centerY - ty1)/ratio_y-1;
                                    float width   = pow((sigmoid(ptr[2]) * 2), 2) * m_anchors[tidx][anchor_idx][0] / ratio_x; //w
                                    float height  = pow((sigmoid(ptr[3]) * 2), 2) * m_anchors[tidx][anchor_idx][1] / ratio_y; //h
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
                            ptr += nout;
                        }
                    }
                } // end of tidx
            } else {
                std::cerr << "Unsupported yolo ouput layer num: " << output_num << std::endl;
                assert(output_num <= 4);
            }
        }
        else{
            // yolov8, 1 output: [bs, num_classes + 4, box_num]
            bm::BMNNTensor output_tensor(m_bmctx->handle(), "", m_bmnet->get_output_scale(0), &frameInfo.output_tensors[0]);
            int nout = output_tensor.get_shape()->dims[1];
            int box_num = output_tensor.get_shape()->dims[2];
            m_class_num = nout - 4;
            float *output_data = (float*)output_tensor.get_cpu_data() + batch_idx*box_num*nout;
            float *cls_conf = output_data + 4*box_num;
            for (int i = 0; i < box_num; i++) {
                    
                float max_value = 0.0;
                int max_index = 0;
                for (int j = 0; j < m_class_num; j++){
                    float cur_value = cls_conf[i + j*box_num];
                    if (cur_value > max_value) {
                        max_value = cur_value;
                        max_index = j;
                    }
                }
                
                if (max_value >= m_obj_thres){
                    bm::NetOutputObject box;
                    box.score = max_value;
                    // todo here
                    float centerX = (output_data[i + 0*box_num] + 1 - tx1) / ratio_x - 1;
                    float centerY = (output_data[i + 1*box_num] + 1 - ty1) / ratio_y - 1;
                    float width = (output_data[i + 2*box_num] + 0.5) /ratio_x;
                    float height = (output_data[i + 3*box_num] + 0.5) /ratio_y;

                    box.x1  = int(centerX - width  / 2);
                    box.y1  = int(centerY - height / 2);
                    box.x2  = box.x1 + width;
                    box.y2  = box.y1 + height;
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

#if USE_TPUKERNEL
void YoloV5::extract_yolobox_tpukernel(bm::FrameInfo& frameInfo){
    std::vector<bm::NetOutputObject> yolobox_vec;
    std::vector<cv::Rect> bbox_vec;
    auto& images = frameInfo.frames;

    int output_num = m_bmnet->outputTensorNum();
    if(output_num != 3){
        std::cout << "output_num must be 3 !" << std::endl;
    }
    // init tpu_kernel args    
    tpu_kernel_api_yolov5NMS_t api[(int)images.size()];

    // allocate device memory
    bm_handle_t handle = m_bmctx->handle();
    bm_device_mem_t in_dev_mem[output_num];
    for(int i = 0; i < output_num; i++){
        in_dev_mem[i] = frameInfo.output_tensors[i].device_mem;
    }
    bm_device_mem_t out_dev_mem[(int)images.size()];
    bm_device_mem_t detect_num_mem[(int)images.size()];
    float* output_tensor[(int)images.size()];
    int32_t detect_num[(int)images.size()];
    int out_len_max = 200 * 7;
    int batch_num = 1; // 4b has bug, now only for 1b.
    for(int i = 0; i < (int)images.size(); i++){
        output_tensor[i] = new float[out_len_max];
        for (int j = 0; j < output_num; j++) {
            api[i].bottom_addr[j] = bm_mem_get_device_addr(in_dev_mem[j]) + i * in_dev_mem[j].size / MAX_BATCH;
        }
        auto ret = bm_malloc_device_byte(handle, &out_dev_mem[i], out_len_max * sizeof(float));
        assert(BM_SUCCESS == ret);
        ret = bm_malloc_device_byte(handle, &detect_num_mem[i], batch_num * sizeof(int32_t));
        assert(BM_SUCCESS == ret);
        api[i].top_addr = bm_mem_get_device_addr(out_dev_mem[i]);
        api[i].detected_num_addr = bm_mem_get_device_addr(detect_num_mem[i]);

        // config
        api[i].input_num = output_num;
        api[i].batch_num = batch_num;
        for (int j = 0; j < output_num; ++j) {
            api[i].hw_shape[j][0] = frameInfo.output_tensors[j].shape.dims[2];
            api[i].hw_shape[j][1] = frameInfo.output_tensors[j].shape.dims[3];
        }
        api[i].num_classes = frameInfo.output_tensors[0].shape.dims[1] / output_num - 5;
        api[i].num_boxes = m_anchors[0].size();
        api[i].keep_top_k = 200;
        api[i].nms_threshold = MAX(0.1, m_nms_thres);
        api[i].confidence_threshold = MAX(0.1, m_obj_thres);
        auto it=api[i].bias;
        for (const auto& subvector2 : m_anchors) {
            for (const auto& subvector1 : subvector2) {
                it = copy(subvector1.begin(), subvector1.end(), it);
            }
        }
        for (int j = 0; j < output_num; j++) 
            api[i].anchor_scale[j] = m_net_h / frameInfo.output_tensors[j].shape.dims[2];
        api[i].clip_box = 1;
    }

    for(int i = 0; i < (int)images.size(); ++ i)
    {
        yolobox_vec.clear();
        auto& frame = images[i];
        float ratio_x = (float)m_net_w / frame.width;
        float ratio_y = (float)m_net_h / frame.height;
        int tx1 = 0, ty1 = 0;
        
    #if USE_ASPECT_RATIO
        bool isAlignWidth = false;
        float ratio = get_aspect_scaled_ratio(frame.width, frame.height, m_net_w, m_net_h, &isAlignWidth);

        if (isAlignWidth) {
            ty1 = (int)((m_net_h - (int)(frame.height*ratio)) / 2);
        }else{
            tx1 = (int)((m_net_w - (int)(frame.width*ratio)) / 2);
        }
        ratio_x = ratio_y = ratio;
    #endif
        tpu_kernel_launch(m_bmctx->handle(), func_id, &api[i], sizeof(api[i]));
        bm_thread_sync(m_bmctx->handle());
        bm_memcpy_d2s_partial_offset(m_bmctx->handle(), (void*)(detect_num + i), detect_num_mem[i], api[i].batch_num * sizeof(int32_t), 0);
        if (detect_num[i] > 0) {
	    bm_memcpy_d2s_partial_offset(m_bmctx->handle(), (void*)output_tensor[i], out_dev_mem[i], detect_num[i] * 7 * sizeof(float), 0);  
	}
	for (int bid = 0; bid < detect_num[i]; bid++) {
            bm::NetOutputObject temp_bbox;
            temp_bbox.class_id = *(output_tensor[i] + 7 * bid + 1);
            if (temp_bbox.class_id == -1) {
                continue;
            }
            temp_bbox.score = *(output_tensor[i] + 7 * bid + 2);
            float centerX = (*(output_tensor[i] + 7 * bid + 3) + 1 - tx1) / ratio - 1;
            float centerY = (*(output_tensor[i] + 7 * bid + 4) + 1 - ty1) / ratio - 1;
            auto width = (*(output_tensor[i] + 7 * bid + 5) + 0.5) / ratio;
            auto height = (*(output_tensor[i] + 7 * bid + 6) + 0.5) / ratio;

            temp_bbox.x1  = int(centerX - width  / 2);
            temp_bbox.y1  = int(centerY - height / 2);
            temp_bbox.x2  = temp_bbox.x1 + width;
            temp_bbox.y2  = temp_bbox.y1 + height;                                   
            
            yolobox_vec.push_back(temp_bbox);  // 0
        }
        bm::NetOutputDatum datum(yolobox_vec);
        frameInfo.out_datums.push_back(datum);

        delete [] output_tensor[i];
        bm_free_device(m_bmctx->handle(), out_dev_mem[i]);
        bm_free_device(m_bmctx->handle(), detect_num_mem[i]);
    }

}
#endif
