//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolact.h"
#include "bmutility_profile.h"
#include <algorithm>

#define USE_GEN_MASK 0

YOLACT::YOLACT(bm::BMNNContextPtr bmctx, std::string model_type):m_bmctx(bmctx)
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

YOLACT::~YOLACT()
{

}


int YOLACT::init_yolo(std::string model_type = "yolact_base"){
    int ret = 0;

    std::transform( model_type.begin(), model_type.end(), model_type.begin(), ::tolower );

    std::vector <float> rgb_mean;
    std::vector <float> rgb_std;
    float scale;
    m_alpha = std::vector <float> (3);
    m_beta = std::vector <float> (3);

    if ((model_type == "yolact_base") or (model_type == "yolact_im700") or (model_type == "yolact_resnet50")){
        m_model_type = 0;
        // use normalize
        rgb_mean = {123.68, 116.78, 103.94};
        rgb_std = {58.40, 57.12, 57.38};
        scale = 1.f;
    }
    else if (model_type == "yolact_darknet53"){
        m_model_type = 1;
        // use to_float
        rgb_mean = {0, 0, 0};
        rgb_std = {1, 1, 1};
        scale = 1.f/255;
    }
    else{
        std::cerr << "Unsupported model_type: " << model_type << std::endl;
        assert(
            (model_type == "yolact_base") or (model_type == "yolact_im700")
            or (model_type == "yolact_resnet50")
            or (model_type == "yolact_darknet53")
        );
    }

    for (int i = 0; i < rgb_mean.size(); i++){
        m_alpha[i] = scale / rgb_std[i]; 
        m_beta[i] = - rgb_mean[i] * scale / rgb_std[i];
    }

    if ((model_type == "yolact_base") or (model_type == "yolact_resnet50") or (model_type == "yolact_darknet53")){
        m_num_scales = 5;
        m_num_priors = 0;
        m_num_aspect_ratios = 3;

        m_conv_ws = {69, 35, 18, 9, 5};
        m_conv_hs = {69, 35, 18, 9, 5};
        m_aspect_ratios = {1, 0.5, 2};
        m_scales = {24, 48, 96, 192, 384};
        m_variances = {0.1, 0.2};
    }
    else if (model_type == "yolact_im700"){
        m_num_scales = 5;
        m_num_priors = 0;
        m_num_aspect_ratios = 3;

        m_conv_ws = {88, 44, 22, 11, 6};
        m_conv_hs = {88, 44, 22, 11, 6};
        m_aspect_ratios = {1, 0.5, 2};
        m_scales = {30, 61, 122, 244, 488};
        m_variances = {0.1, 0.2};
    }
    else{
        std::cerr << "Unsupported model_type: " << model_type << std::endl;
        assert(
            (model_type == "yolact_base") or (model_type == "yolact_im700")
            or (model_type == "yolact_resnet50")
            or (model_type == "yolact_darknet53")
        );
    }

    // make priors
    make_priors();

    return ret;

}

int YOLACT::get_Batch(){
    return MAX_BATCH;
}

int YOLACT::preprocess(std::vector<bm::FrameBaseInfo>& frames, std::vector<bm::FrameInfo>& frame_infos)
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

            ret = bmcv_image_vpp_convert(handle, 1, image1, &resized_imgs[i]);
            assert(BM_SUCCESS == ret);


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

        bm::BMImage::destroy_batch(resized_imgs, num);
        bm::BMImage::destroy_batch(convertto_imgs, num);

        frame_infos.push_back(finfo);
    }

    return ret;
}

int YOLACT::forward(std::vector<bm::FrameInfo>& frame_infos)
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

int YOLACT::postprocess(std::vector<bm::FrameInfo> &frame_infos)
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

/*This code is refer from:
https://github.com/Tencent/ncnn/blob/master/examples/yolact.cpp
*/
void YOLACT::make_priors(){
    int p = 0;
    for (p = 0; p < m_num_scales; p++){
        m_num_priors += m_conv_ws[p] * m_conv_hs[p] * m_num_aspect_ratios;
    }
    m_priorbox.resize(4 * m_num_priors);

    //generate priorbox
	float* pb = m_priorbox.data();
    for (p = 0; p < m_num_scales; p++){
        int conv_w = m_conv_ws[p];
		int conv_h = m_conv_hs[p];
		float scale = m_scales[p];

        for (int i = 0; i < conv_h; i++)
		{
			for (int j = 0; j < conv_w; j++)
			{
				// +0.5 because priors are in center-size notation
				float cx = (j + 0.5f) / conv_w;
				float cy = (i + 0.5f) / conv_h;

				for (int k = 0; k < m_num_aspect_ratios; k++)
				{
					float ar = m_aspect_ratios[k];

					ar = sqrt(ar);

					float w = scale * ar / m_net_w;
					float h = scale / ar / m_net_h;

					// This is for backward compatability with a bug where I made everything square by accident
					// cfg.backbone.use_square_anchors:
					h = w;
					pb[0] = cx;
					pb[1] = cy;
					pb[2] = w;
					pb[3] = h;
					pb += 4;
				}
			}
		}
    }

}

inline float YOLACT::intersection_area(const bm::NetOutputObject& a, const bm::NetOutputObject& b)
{
    float x = std::max<float>(a.x1, b.x1);
    float y = std::max<float>(a.y1, b.y1);
    float w = std::min<float>(a.x2, b.x2) - x + 1;
    float h = std::min<float>(a.y2, b.y2) - y + 1;
    float area = w * h;
    return area;
}

void YOLACT::qsort_descent_inplace(
    std::vector<bm::NetOutputObject>& datas, std::vector<std::vector<float>>& masks, int left, int right
)
{
    int i = left;
    int j = right;
    float p = datas[(left + right) / 2].score;

    while (i <= j)
    {
        while (datas[i].score > p)
            i++;

        while (datas[j].score < p)
            j--;

        if (i <= j)
        {
            // swap
            std::swap(datas[i], datas[j]);
            std::swap(masks[i], masks[j]);

            i++;
            j--;
        }
    }

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if (left < j) qsort_descent_inplace(datas, masks, left, j);
        }
        #pragma omp section
        {
            if (i < right) qsort_descent_inplace(datas, masks, i, right);
        }
    }

}


void YOLACT::qsort_descent_inplace(std::vector<bm::NetOutputObject>& datas, std::vector<std::vector<float>> masks)
{
    if (datas.empty())
        return;

    qsort_descent_inplace(datas, masks, 0, static_cast<int>(datas.size() - 1));
}


void YOLACT::nms_sorted_bboxes(const std::vector<bm::NetOutputObject>& objects, 
    std::vector<int>& picked, float nms_threshold, bool agnostic = false)
{
    picked.clear();

    const int n = objects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++)
    {
        areas[i] = (objects[i].x2 - objects[i].x1 + 1) * (objects[i].y2 - objects[i].y1 + 1);
    }

    for (int i = 0; i < n; i++)
    {
        const bm::NetOutputObject& a = objects[i];

        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++)
        {
            const bm::NetOutputObject& b = objects[picked[j]];

            if (!agnostic && a.class_id != b.class_id)
                continue;

            // intersection over union
            float inter_area = intersection_area(a, b);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            //             float IoU = inter_area / union_area
            if (inter_area / union_area > nms_threshold)
                keep = 0;
        }

        if (keep)
            picked.push_back(i);
    }
}


void YOLACT::sigmoid(cv::Mat& out, int length)
{
	float* pdata = (float*)(out.data);
	int i = 0;
	for (i = 0; i < length; i++)
	{
		pdata[i] = 1.0 / (1 + expf(-pdata[i]));
	}
}



static void draw_objects(const cv::Mat& bgr, const std::vector<bm::NetOutputObject>& objects, std::vector<cv::Mat> &masks)
{
    static const char* class_names[] = {"background",
                                        "person", "bicycle", "car", "motorcycle", "airplane", "bus",
                                        "train", "truck", "boat", "traffic light", "fire hydrant",
                                        "stop sign", "parking meter", "bench", "bird", "cat", "dog",
                                        "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe",
                                        "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
                                        "skis", "snowboard", "sports ball", "kite", "baseball bat",
                                        "baseball glove", "skateboard", "surfboard", "tennis racket",
                                        "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl",
                                        "banana", "apple", "sandwich", "orange", "broccoli", "carrot",
                                        "hot dog", "pizza", "donut", "cake", "chair", "couch",
                                        "potted plant", "bed", "dining table", "toilet", "tv", "laptop",
                                        "mouse", "remote", "keyboard", "cell phone", "microwave", "oven",
                                        "toaster", "sink", "refrigerator", "book", "clock", "vase",
                                        "scissors", "teddy bear", "hair drier", "toothbrush"
                                       };

    static const unsigned char colors[81][3] = {
        {56, 0, 255},
        {226, 255, 0},
        {0, 94, 255},
        {0, 37, 255},
        {0, 255, 94},
        {255, 226, 0},
        {0, 18, 255},
        {255, 151, 0},
        {170, 0, 255},
        {0, 255, 56},
        {255, 0, 75},
        {0, 75, 255},
        {0, 255, 169},
        {255, 0, 207},
        {75, 255, 0},
        {207, 0, 255},
        {37, 0, 255},
        {0, 207, 255},
        {94, 0, 255},
        {0, 255, 113},
        {255, 18, 0},
        {255, 0, 56},
        {18, 0, 255},
        {0, 255, 226},
        {170, 255, 0},
        {255, 0, 245},
        {151, 255, 0},
        {132, 255, 0},
        {75, 0, 255},
        {151, 0, 255},
        {0, 151, 255},
        {132, 0, 255},
        {0, 255, 245},
        {255, 132, 0},
        {226, 0, 255},
        {255, 37, 0},
        {207, 255, 0},
        {0, 255, 207},
        {94, 255, 0},
        {0, 226, 255},
        {56, 255, 0},
        {255, 94, 0},
        {255, 113, 0},
        {0, 132, 255},
        {255, 0, 132},
        {255, 170, 0},
        {255, 0, 188},
        {113, 255, 0},
        {245, 0, 255},
        {113, 0, 255},
        {255, 188, 0},
        {0, 113, 255},
        {255, 0, 0},
        {0, 56, 255},
        {255, 0, 113},
        {0, 255, 188},
        {255, 0, 94},
        {255, 0, 18},
        {18, 255, 0},
        {0, 255, 132},
        {0, 188, 255},
        {0, 245, 255},
        {0, 169, 255},
        {37, 255, 0},
        {255, 0, 151},
        {188, 0, 255},
        {0, 255, 37},
        {0, 255, 0},
        {255, 0, 170},
        {255, 0, 37},
        {255, 75, 0},
        {0, 0, 255},
        {255, 207, 0},
        {255, 0, 226},
        {255, 245, 0},
        {188, 255, 0},
        {0, 255, 18},
        {0, 255, 75},
        {0, 255, 151},
        {255, 56, 0},
        {245, 255, 0}
    };

    cv::Mat image = bgr;

    int color_index = 0;
    for (size_t i = 0; i < objects.size(); i++)
    {
        auto obj = objects[i];
        auto mask = masks[i];

        const unsigned char* color = colors[color_index % 81];
        color_index++;

        cv::rectangle(image, cv::Rect(obj.x1, obj.y1, obj.x2 - obj.x1 + 1, obj.y2 - obj.y1 + 1), cv::Scalar(color[0], color[1], color[2]));
        char text[256];
        sprintf(text, "%s %.2f%%", class_names[obj.class_id], obj.score * 100);

        int baseLine = 0;
        cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        
        int x = obj.x1;
        int y = obj.y1 - label_size.height - baseLine;
        if (y < 0)
            y = 0;
        if (x + label_size.width > image.cols)
            x = image.cols - label_size.width;

        cv::rectangle(image, cv::Rect(cv::Point(obj.x1, obj.y1), cv::Size(label_size.width, label_size.height + baseLine)),
                      cv::Scalar(255, 255, 255), 2);
        
        cv::putText(image, text, cv::Point(obj.x1, obj.y1 + label_size.height),//cv::Point(x, y + label_size.height),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

        // draw mask
        for (int y = 0; y < image.rows; y++)
        {
            const unsigned char* mp = mask.ptr(y);
            unsigned char* p = image.ptr(y);
            for (int x = 0; x < image.cols; x++)
            {
                if (mp[x] == 255)
                {
                    p[0] = cv::saturate_cast<unsigned char>(p[0] * 0.5 + color[0] * 0.5);
                    p[1] = cv::saturate_cast<unsigned char>(p[1] * 0.5 + color[1] * 0.5);
                    p[2] = cv::saturate_cast<unsigned char>(p[2] * 0.5 + color[2] * 0.5);
                }
                p += 3;
            }
        }
    }

    // cv::imwrite("vis.jpg", image);
    // std::cout << "vis.jpg is saved." << std::endl;

}


void YOLACT::extract_yolobox_cpu(bm::FrameInfo& frameInfo)
{
    std::vector<bm::NetOutputObject> yolactbox_vec;
    std::vector<std::vector<float>> yolact_maskdata;
    std::vector<cv::Mat> mat_masks;

    auto& images = frameInfo.frames;
    for(int batch_idx = 0; batch_idx < (int)images.size(); ++ batch_idx)
    {        
        yolactbox_vec.clear();
        
        auto& frame = images[batch_idx];
        int orgw = frame.width;
        int orgh = frame.height;
        int output_num = m_bmnet->outputTensorNum();

        // pay your attention to your bmodel out node name. output order: conf, loc, mask, proto.
        bm::BMNNTensor conf_output_tensor(m_bmctx->handle(), "conf", m_bmnet->get_output_scale(0), &frameInfo.output_tensors[0]);
        bm::BMNNTensor loc_output_tensor(m_bmctx->handle(), "loc", m_bmnet->get_output_scale(1), &frameInfo.output_tensors[1]);
        bm::BMNNTensor mask_output_tensor(m_bmctx->handle(), "mask", m_bmnet->get_output_scale(2), &frameInfo.output_tensors[2]);
        bm::BMNNTensor proto_output_tensor(m_bmctx->handle(), "proto", m_bmnet->get_output_scale(3), &frameInfo.output_tensors[3]);

        int box_num = loc_output_tensor.get_shape()->dims[1]; 
        int box_nout = loc_output_tensor.get_shape()->dims[2];
        int cls_nout = conf_output_tensor.get_shape()->dims[2];
        m_class_num = cls_nout - 1;
        int mask_nout = mask_output_tensor.get_shape()->dims[2];
        int proto_h = proto_output_tensor.get_shape()->dims[1];
        int proto_w = proto_output_tensor.get_shape()->dims[2];

        float *loc_output_data = (float*)loc_output_tensor.get_cpu_data() + batch_idx*box_num*box_nout;
        float *conf_output_data = (float*)conf_output_tensor.get_cpu_data() + batch_idx*box_num*cls_nout;
        float *mask_output_data = (float*)mask_output_tensor.get_cpu_data() + batch_idx*box_num*mask_nout;
        float *proto_output_data = (float*)proto_output_tensor.get_cpu_data() + batch_idx*box_num*box_nout;

        // get detections
        std::vector<std::vector<bm::NetOutputObject>> class_candidates;
        class_candidates.resize(cls_nout);

        // instance
        std::vector<std::vector<std::vector <float>>> maskdata_candidates;
        maskdata_candidates.resize(cls_nout);

        for (int i = 0; i < m_num_priors; i++){
            float *loc = loc_output_data + i * box_nout;
            float *conf = conf_output_data + i *  cls_nout;
            float *maskdata = mask_output_data + i * mask_nout;
            float *pb = m_priorbox.data() + i * box_nout;

            // find class id with highest score
            // start from 1 to skip background

            
            int label = 0;
            float score = 0.f;

            for (int j = 1; j < m_class_num; j++){
                float class_score = conf[j];
                if (class_score > score)
                {
                    label = j;
                    score = class_score;
                }
            }

            // ignore background or low score
            if (label == 0 || score <= m_obj_thres)
                continue;
            
            float pb_cx = pb[0];
            float pb_cy = pb[1];
            float pb_w = pb[2];
            float pb_h = pb[3];
            
            float bbox_cx = m_variances[0] * loc[0] * pb_w + pb_cx;
            float bbox_cy = m_variances[0] * loc[1] * pb_h + pb_cy;
            float bbox_w = (float)(exp(m_variances[1] * loc[2]) * pb_w);
            float bbox_h = (float)(exp(m_variances[1] * loc[3]) * pb_h);

            float obj_x1 = bbox_cx - bbox_w * 0.5f;
            float obj_y1 = bbox_cy - bbox_h * 0.5f;
            float obj_x2 = bbox_cx + bbox_w * 0.5f;
            float obj_y2 = bbox_cy + bbox_h * 0.5f;

            // clip
            obj_x1 = std::max(std::min(obj_x1 * frame.width, (float)(frame.width - 1)), 0.f);
            obj_y1 = std::max(std::min(obj_y1 * frame.height, (float)(frame.height - 1)), 0.f);
            obj_x2 = std::max(std::min(obj_x2 * frame.width, (float)(frame.width - 1)), 0.f);
            obj_y2 = std::max(std::min(obj_y2 * frame.height, (float)(frame.height - 1)), 0.f);
           
            // append object
            bm::NetOutputObject obj;
            obj.x1 = obj_x1;
            obj.y1 = obj_y1;
            obj.x2 = obj_x2;
            obj.y2 = obj_y2;
            obj.score = score;
            obj.class_id = label;

            class_candidates[label].push_back(obj);

            // append instance
            std::vector <float> obj_maskdata = std::vector<float>(maskdata, maskdata + mask_nout);
            maskdata_candidates[label].push_back(obj_maskdata);
        }

        // objects.clear() 
        yolactbox_vec.clear();
        yolact_maskdata.clear();
        
        for (int i = 0; i < (int)class_candidates.size(); i++){
            std::vector<bm::NetOutputObject>& candidates = class_candidates[i];
            std::vector<std::vector<float>> maskdata_candidates_i = maskdata_candidates[i];

            qsort_descent_inplace(candidates, maskdata_candidates_i);

            std::vector<int> picked;
            nms_sorted_bboxes(candidates, picked, m_nms_thres);

            for (int j = 0; j < (int)picked.size(); j++){
                int z = picked[j];
                yolactbox_vec.push_back(candidates[z]);

                yolact_maskdata.push_back(maskdata_candidates_i[z]);
            }
        }

        // qsort for keep_top_k
        qsort_descent_inplace(yolactbox_vec, yolact_maskdata);

        // keep_top_k
        if (m_keep_top_k < (int)yolactbox_vec.size()){
            yolactbox_vec.resize(m_keep_top_k);
            yolact_maskdata.resize(m_keep_top_k);
        }
        
# if USE_GEN_MASK
        // generate mask
        float *proto_data = proto_output_data;
        mat_masks.resize(yolactbox_vec.size());
        
        for (int i = 0; i < (int)yolactbox_vec.size(); i++)
        {
            auto obj_mask = yolact_maskdata[i];
            auto obj = yolactbox_vec[i];
            cv::Mat mask(proto_h, proto_w, CV_32FC1);
            mask = cv::Scalar(0.f);
            float* mp = (float*)mask.data;

            for (int p = 0; p < mask_nout; p++)
            {
                float coeff = obj_mask[p];
                for (int j = 0; j < proto_h; j++){
                    for (int k = 0; k < proto_w; k++){
                        mp[j*proto_w + k] += proto_data[j*proto_w*mask_nout + k*mask_nout + p] * coeff; 
                    }
                }
            }

            // ignore sigmoid for mask
            // todo: debug for mask visualization.
            this->sigmoid(mask, proto_h*proto_w);

            cv::Mat mask2;
            cv::resize(mask, mask2, cv::Size(frame.width, frame.height));
            // crop obj box and binarize
            mat_masks[i] = cv::Mat(frame.height, frame.width, CV_8UC1);
            {
                mat_masks[i] = cv::Scalar(0);

                for( int y = 0; y < frame.height; y++){
                    if (y < obj.y1 || y > obj.y2)
                        continue;

                    const float* mp2 = mask2.ptr<const float>(y);
                    unsigned char* bmp = mat_masks[i].ptr<unsigned char>(y);
                    for (int x = 0; x < frame.width; x++){
                        if (x < obj.x1 || x > obj.x2)
                            continue;

                        bmp[x] = mp2[x] > 0.5f ? 255 : 0;
                        //bmp[x] = (int)(mp2[x] * 255.f);
                    }

                }
            }
            

        }
        // draw
        cv::Mat output_mat;
        cv::bmcv::toMAT(&frame.original, output_mat, true);

        draw_objects(output_mat, yolactbox_vec, mat_masks);
#endif

        bm::NetOutputDatum datum(yolactbox_vec);
        frameInfo.out_datums.push_back(datum);

        bm_image_destroy(frame.original);
    }

}
