//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "face_extract.h"
#ifndef USE_RGBP_SEPARATE
#define USE_RGBP_SEPARATE 0
#endif
#define PLD_DEBUG_DUMP_DATA 0



FaceExtract::FaceExtract(bm::BMNNContextPtr bmctx):m_bmctx(bmctx) {
#if 1
    auto net_name = "resnet";
#else
    std::string net_name = "feature_extract_bmnetc";
#endif
    m_bmnet = std::make_shared<bm::BMNNNetwork>(m_bmctx->bmrt(), net_name); //feature_extract_bmnetc

    assert(m_bmnet != nullptr);
    m_alpha_int8 = 0.0078431;
    m_beta_int8  = -1;
    m_alpha_fp32 = 0.0078431;
    m_beta_fp32  = -1;

    auto shape = m_bmnet->inputTensor(0)->get_shape();
    // for NCHW
    m_net_h = shape->dims[2];
    m_net_w = shape->dims[3];
    MAX_BATCH = shape->dims[0];

}

FaceExtract::~FaceExtract()
{

}

int FaceExtract::preprocess(std::vector<bm::FeatureFrame> &frames, std::vector<bm::FeatureFrameInfo> &of)
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

        bm::FeatureFrameInfo finfo;

        //1. Resize
        bm_image resized_imgs[MAX_BATCH];
    #if USE_RGBP_SEPARATE
        ret = bm::BMImage::create_batch(handle, m_net_h, m_net_w, FORMAT_RGBP_SEPARATE, DATA_TYPE_EXT_1N_BYTE, resized_imgs, num, 64, false, false);
    #else
        ret = bm::BMImage::create_batch(handle, m_net_h, m_net_w, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, resized_imgs, num, 64);
    #endif
        assert(BM_SUCCESS == ret);

        for(int i = 0;i < num; ++i) {
            bm_image image1;
#if USE_SOPHON_OPENCV //default: 0
            cv::Mat cvm1=frames[start_idx + i].img;
            cv::bmcv::toBMI(cvm1, &image1);
            ret = bmcv_image_vpp_convert(handle, 1, image1, &resized_imgs[i], NULL, BMCV_INTER_LINEAR);
            assert(BM_SUCCESS == ret);
#else
            int width = frames[start_idx + i].img.cols;
            int height = frames[start_idx + i].img.rows;
            int frameSize = width * height;
            int strides[1]={(int)frames[start_idx + i].img.step};
            ret = bm::BMImage::create_batch(handle, height, width, FORMAT_BGR_PACKED, DATA_TYPE_EXT_1N_BYTE, &image1, 1);
            assert(ret == 0);
            auto frame = frames[start_idx + i].img;
            void *buffers[4]={0};
            buffers[0] = frame.data;
            ret = bm_image_copy_host_to_device(image1, buffers);
            assert(ret == 0);
        #if PLD_DEBUG_DUMP_DATA
            std::cout<<"cv::mat data:==========="<<std::endl;
            int count = 0;
            for (int row = 0; row < height; ++row) {
                for (int col = 0; col < width; ++col) {
                    uchar pixelValue = frame.at<uchar>(row, col);
                    std::cout << static_cast<int>(pixelValue) << " ";
                    ++count;
                    if (count == 100)
                        break;
                }
                if (count == 100)
                    break;
            }
            
            std::cout<<std::endl<<"========================="<<std::endl;
            std::cout<<"image1:data:============="<<std::endl;
            uchar *buffers_image1[4]={0};
            buffers_image1[0] = new uchar[strides[0]*height];
            ret = bm_image_copy_device_to_host(image1, (void**)buffers_image1);
            assert(ret == 0);
            for(int kk = 0; kk < 100; kk++){
                std::cout<<(int)buffers_image1[0][kk]<<" ";
            }
            delete [] buffers_image1[0];
            std::cout<<std::endl<<"========================="<<std::endl;
        #endif
        #if 1 //PLD
            // std::cout<<"convert packed to plannar"<<std::endl;
            bm_image image_planar;
            assert(0 == bm::BMImage::create_batch(handle, height, width, FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, &image_planar, 1, 64));
            assert(0 == bmcv_image_storage_convert(handle, 1, &image1, &image_planar));
            // std::cout<<"this is face_extract.cpp:100 bmcv_image_vpp_convert"<<std::endl;
            ret = bmcv_image_vpp_convert(handle, 1, image_planar, &resized_imgs[i], NULL, BMCV_INTER_LINEAR);   
            //ret = bmcv_image_vpp_convert(handle, 1, image1, &resized_imgs[i], NULL, BMCV_INTER_LINEAR);   
            assert(ret == 0);
            bm_image_destroy_allinone(&image_planar);
        #endif
        #if PLD_DEBUG_DUMP_DATA
            std::cout<<"resized_:data:============="<<std::endl;
            bm_image_dump_size(resized_imgs[i],100);
            std::cout<<std::endl<<"========================="<<std::endl;
        #endif
#endif
            finfo.frames.push_back(frames[start_idx + i]);
            bm_image_destroy_allinone(&image1);
        }

        //2. Convert to
        bm_image convertto_imgs[MAX_BATCH];
        float alpha, beta;

        bm_image_data_format_ext img_type;
        auto tensor = m_bmnet->inputTensor(0);
        if (tensor->get_dtype() == BM_INT8) {
            alpha            = m_alpha_int8 * tensor->get_scale();
            beta             = m_beta_int8 * tensor->get_scale();
            img_type = DATA_TYPE_EXT_1N_BYTE_SIGNED;
        }else{
            alpha            = m_alpha_fp32;
            beta             = m_alpha_fp32;
            img_type = DATA_TYPE_EXT_FLOAT32;
        }

    #if USE_RGBP_SEPARATE
        ret = bm::BMImage::create_batch(handle, m_net_h, m_net_w, FORMAT_RGBP_SEPARATE, img_type, convertto_imgs, num, 1, false, false);
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
        convert_to_attr.beta_0  = beta;
        convert_to_attr.beta_1  = beta;
        convert_to_attr.beta_2  = beta;

    #if PLD
        std::cout<<"This is face_extract.cpp:138: bmcv_image_convert_to. "<<std::endl;
    #endif
        ret = bmcv_image_convert_to(m_bmctx->handle(), num, convert_to_attr, resized_imgs, convertto_imgs);
        assert(ret == 0);
    #if PLD
        std::cout<<"convert_to successed!"<<std::endl;
        std::cout<<"bm_image_detach_contiguous_mem"<<std::endl;
    #endif 
    #if A2_SDK
        ret = bm_image_detach_contiguous_mem(num, convertto_imgs);
    #else
        ret = bm_image_dettach_contiguous_mem(num, convertto_imgs);
    #endif
        assert(ret == 0);
        finfo.input_tensors.push_back(input_tensor);
    #if PLD
        std::cout<<"face_extract: input_tensor:"<<std::endl;
        std::cout<<"========================="<<std::endl;
        bm::BMNNTensorPtr tensor_ = std::make_shared<bm::BMNNTensor>(m_bmctx->handle(), "extractor_in", 1.0,
                                                                &input_tensor);
        float* input_cpu_data = tensor_->get_cpu_data();
        for(int kk = 0; kk < 20; kk++){
            std::cout<<*(input_cpu_data+kk)<<" ";
        }
        std::cout<<std::endl<<"========================="<<std::endl;
    #endif

        bm::BMImage::destroy_batch(resized_imgs, num);
        bm::BMImage::destroy_batch(convertto_imgs, num);

        of.push_back(finfo);
    }
    return ret;
}

int FaceExtract::forward(std::vector<bm::FeatureFrameInfo> &frame_infos, int core_id)
{
    int ret = 0;
    for(int b = 0; b < frame_infos.size(); ++b) {
        for (int i = 0; i < m_bmnet->outputTensorNum(); ++i) {
            bm_tensor_t tensor;
            frame_infos[b].output_tensors.push_back(tensor);
        }
    #if PLD
        std::cout<<"this is face_extractor, forward."<<std::endl;
        std::cout<<"input_tensor shape:"<<std::endl;
        for(int i = 0; i < frame_infos[b].input_tensors.size(); i++){
            std::cout<<std::endl;
            for(int j = 0; j < frame_infos[b].input_tensors[i].shape.num_dims; j++){
                std::cout<<frame_infos[b].input_tensors[i].shape.dims[j]<<" ";
            }
        }
        std::cout<<std::endl;
    #endif
    //TODO: 2core bmodel cannot use this code do inference.
    #if A2_SDK && !USE_2CORE
        if(m_bmctx->get_core_id()!=-1){
            core_id = m_bmctx->get_core_id();
        }
        ret = m_bmnet->forward_core(frame_infos[b].input_tensors.data(), frame_infos[b].input_tensors.size(),
                              frame_infos[b].output_tensors.data(), frame_infos[b].output_tensors.size(), core_id);
    #else
        ret = m_bmnet->forward(frame_infos[b].input_tensors.data(), frame_infos[b].input_tensors.size(),
                              frame_infos[b].output_tensors.data(), frame_infos[b].output_tensors.size());
    #endif
        assert(BM_SUCCESS == ret);
        #if PLD
            std::cout<<"face_extractor forward success."<<std::endl;
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
    return ret;
}

int FaceExtract::postprocess(std::vector<bm::FeatureFrameInfo> &frameinfos)
{
    #if PLD
        std::cout<<"this is face_extractor: postprocess."<<std::endl;
    #endif
    for(int i=0;i < frameinfos.size(); ++i) {

        // Free AVFrames
        auto &frame_info = frameinfos[i];

        // extract face features
        extract_facefeature_cpu(frame_info);

        if (m_pfnDetectFinish != nullptr) {
            m_pfnDetectFinish(frame_info);
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

void FaceExtract::extract_facefeature_cpu(bm::FeatureFrameInfo &frame_info) {

    int frameNum = frame_info.frames.size();
    frame_info.out_datums.resize(frameNum);
    for(int frameIdx = 0; frameIdx < frameNum;++frameIdx) {
    #if 0//A2_SDK
        bm::BMNNTensorPtr output_tensor = get_output_tensor("mobilenetv20_output_flatten0_reshape0_Reshape_f32", frame_info, m_bmnet->get_output_scale(0));
    #else
        bm::BMNNTensorPtr output_tensor = get_output_tensor(m_bmnet->m_netinfo->output_names[0], frame_info, m_bmnet->get_output_scale(0));
    #endif
        const void *out_data = (const void *) output_tensor->get_cpu_data();
        auto output_shape = output_tensor->get_shape();
        int out_c = output_shape->dims[1];
        int out_n = output_shape->dims[0];

    #if PLD
        std::cout<<"face_etractor_feature: "<<std::endl;
    #endif
        for (int n = 0; n < out_n; n++) {
            const float *data = (const float *) out_data + out_c * n;
            bm::ObjectFeature features;
            features.clear();
            for (int i = 0; i < out_c; i++) {
            #if PLD
                if(i % 50 == 0)
                    std::cout<<data[i]<<" ";
            #endif
                features.push_back(data[i]);
            }
        #if PLD
            std::cout<<std::endl;
        #endif
            frame_info.out_datums[frameIdx].face_features.push_back(features);
        }
    }
}

bm::BMNNTensorPtr FaceExtract::get_output_tensor(const std::string &name, bm::FeatureFrameInfo& frame_info, float scale=1.0) {
    int output_tensor_num = frame_info.output_tensors.size();
    int idx = m_bmnet->outputName2Index(name);
    if (idx < 0 && idx > output_tensor_num-1) {
        std::cout << "ERROR:idx=" << idx << std::endl;
        assert(0);
    }
    #if PLD
        std::cout<<"FaceExtract::get_output_tensor:"<<std::endl;
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
    bm::BMNNTensorPtr tensor = std::make_shared<bm::BMNNTensor>(m_bmctx->handle(), name, scale,
                                                                &frame_info.output_tensors[idx]);
    return tensor;
}
