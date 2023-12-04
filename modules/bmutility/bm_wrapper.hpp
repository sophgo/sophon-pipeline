//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

/*
 * This is a wrapper header of BMruntime & BMCV, aiming to simplify user's program.
 */

#ifndef _BM_WRAPPER_HPP_
#define _BM_WRAPPER_HPP_

#include "bmruntime_interface.h"
#if A2_SDK
extern "C" {
    #include "bmcv_api_ext.h"
}
#else
#include "bmcv_api_ext.h"
#endif
#include "bmutility_image.h"
#include "bmlib_runtime.h"
#include <sys/time.h>
#include <iostream>
#include <vector>

static int bm_image_destroy_allinone(bm_image *image){
  int ret = 0;
  #if A2_SDK
    ret = bm_image_destroy(image);
  #else
    ret = bm_image_destroy(*image);
  #endif
  assert(ret == BM_SUCCESS);
  return ret;
}

/* Define this macro in advance to enable following APIs */
#ifdef USE_OPENCV
#include <opencv2/opencv.hpp>

/**
 * @name    bm_image_from_mat
 * @brief   Convert opencv a Mat object to a BMCV bm_image object
 * @ingroup bmruntime
 *
 * @param [in]           bm_handle   the low level device handle
 * @param [in]           in          a read-only OPENCV mat object
 * @param [out]          out         an uninitialized BMCV bm_image object
 * @retval BM_SUCCESS    change success.
 * @retval other values  change failed.
 */

static inline bm_status_t bm_image_from_mat (bm_handle_t       &bm_handle,
                                      cv::Mat           &in,
                                      bm_image          &out) {
  bm_status_t ret =  cv::bmcv::toBMI(in, &out, true);
  if (ret != BM_SUCCESS) {
    std::cout << "Error! bm_image_from_mat: " << ret << std::endl;
  }
  return ret;
}

/**
 * @name    bm_image_from_mat
 * @brief   Convert opencv Mat object to BMCV bm_image object
 * @ingroup bmruntime
 *
 * @param [in]           bm_handle   the low level device handle
 * @param [in]           in          a read-only OPENCV mat vector
 * @param [out]          out         an uninitialized BMCV bm_image vector
 * @retval BM_SUCCESS    change success.
 * @retval other values  change failed.
 */
static inline bm_status_t bm_image_from_mat (bm_handle_t           &bm_handle,
                                             std::vector<cv::Mat>  &in,
				                                     std::vector<bm_image> &out) {

  /* sanity check */
  if (in.empty()) {
    std::cout << "bm_image_from_mat: input empty!!" << std::endl;
    return BM_ERR_PARAM;
  }

  if (!out.empty()) {
    out.clear();
  }

  /* convert mat to bm_image one by one */
  for (size_t i = 0; i < in.size(); i++) {
    bm_image tmp;
    bm_image_from_mat (bm_handle, in[i], tmp);
    out.push_back (tmp);
  }

  return BM_SUCCESS       ;
}

#endif // USE_OPENCV

/* Define USE_FFMPEG macro in advance to enable following APIs */
#ifdef USE_FFMPEG

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>

typedef struct{
        bm_image *bmImg;
        uint8_t* buf0;
        uint8_t* buf1;
        uint8_t* buf2;
}transcode_t;

static int map_bmformat_to_avformat(int bmformat)
{
    int format;
    switch(bmformat){
        case FORMAT_YUV420P: format = AV_PIX_FMT_YUV420P; break;
        case FORMAT_YUV422P: format = AV_PIX_FMT_YUV422P; break;
        case FORMAT_YUV444P: format = AV_PIX_FMT_YUV444P; break;
        case FORMAT_NV12:    format = AV_PIX_FMT_NV12; break;
        case FORMAT_NV16:    format = AV_PIX_FMT_NV16; break;
        case FORMAT_GRAY:    format = AV_PIX_FMT_GRAY8; break;
        case FORMAT_RGBP_SEPARATE: format = AV_PIX_FMT_GBRP; break;
        default: printf("unsupported image format %d\n", bmformat); return -1;
    }
    return format;
}

static inline int map_avformat_to_bmformat(int avformat)
{
    int format;
    switch(avformat){
        case AV_PIX_FMT_YUV420P: format = FORMAT_YUV420P; break;
        case AV_PIX_FMT_YUV422P: format = FORMAT_YUV422P; break;
        case AV_PIX_FMT_YUV444P: format = FORMAT_YUV444P; break;
        case AV_PIX_FMT_NV12:    format = FORMAT_NV12; break;
        case AV_PIX_FMT_NV16:    format = FORMAT_NV16; break;
        case AV_PIX_FMT_GRAY8:   format = FORMAT_GRAY; break;
        case AV_PIX_FMT_GBRP:    format = FORMAT_RGBP_SEPARATE; break;
        default: printf("unsupported av_pix_format %d\n", avformat); return -1;
    }

    return format;
}

static inline void bmBufferDeviceMemFree(void *opaque, uint8_t *data)
{
    if(opaque == NULL){
        printf("parameter error\n");
    }
    transcode_t *testTranscoed = (transcode_t *)opaque;
    av_freep(&testTranscoed->buf0);
    testTranscoed->buf0 = NULL;

    int ret =  0;
    ret = bm_image_destroy_allinone(testTranscoed->bmImg);
    if(testTranscoed->bmImg){
        free(testTranscoed->bmImg);
        testTranscoed->bmImg =NULL;
    }
    if(ret != 0)
        printf("bm_image destroy failed\n");
    free(testTranscoed);
    testTranscoed = NULL;
    return ;
}

static inline void bmBufferDeviceMemFree2(void *opaque, uint8_t *data)
{
    return ;
}

/**
 * @name    bm_image_to_avframe
 * @brief   Convert bmcv bm_image object to ffmpeg a avframe object
 * @ingroup bmruntime
 *
 * @param [in]           bm_handle   the low level device handle
 * @param [in]           in          a read-only  BMCV bm_image object.
 * @param [out]          out         a output avframe
                         just support YUV420P ,NV12 format.
 * @retval BM_SUCCESS    convert success.
 * @retval other values  convert failed.
 */
static inline bm_status_t bm_image_to_avframe(bm_handle_t &bm_handle,bm_image *in,AVFrame *out){
    transcode_t *ImgOut  = NULL;
    ImgOut = (transcode_t *)malloc(sizeof(transcode_t));
    ImgOut->bmImg = in;
    bm_image_format_info_t image_info;
    int idx       = 0;
    int plane     = 0;
    if(in == NULL || out == NULL){
        free(ImgOut);
        return BM_ERR_FAILURE;
    }

    if(ImgOut->bmImg->image_format == FORMAT_NV12){
        plane = 2;
    }
    else if(ImgOut->bmImg->image_format == FORMAT_YUV420P){
        plane = 3;
    }
    else{
        free(ImgOut);
        free(in);
        return BM_ERR_FAILURE;
    }

    out->format = (AVPixelFormat)map_bmformat_to_avformat(ImgOut->bmImg->image_format);
    out->height = ImgOut->bmImg->height;
    out->width = ImgOut->bmImg->width;

    if(ImgOut->bmImg->width > 0 && ImgOut->bmImg->height > 0
        && ImgOut->bmImg->height * ImgOut->bmImg->width <= 8192*4096) {
        ImgOut->buf0 = (uint8_t*)av_malloc(ImgOut->bmImg->height * ImgOut->bmImg->width * 3 / 2);
        ImgOut->buf1 = ImgOut->buf0 + (unsigned int)(ImgOut->bmImg->height * ImgOut->bmImg->width);
        if(plane == 3){
            ImgOut->buf2 = ImgOut->buf0 + (unsigned int)(ImgOut->bmImg->height * ImgOut->bmImg->width * 5 / 4);
        }
    }

    out->buf[0] = av_buffer_create(ImgOut->buf0,ImgOut->bmImg->width * ImgOut->bmImg->height,
        bmBufferDeviceMemFree,ImgOut,AV_BUFFER_FLAG_READONLY);
    out->buf[1] = av_buffer_create(ImgOut->buf1,ImgOut->bmImg->width * ImgOut->bmImg->height / 2 /2 ,
        bmBufferDeviceMemFree2,NULL,AV_BUFFER_FLAG_READONLY);
    out->data[0] = ImgOut->buf0;
    out->data[1] = ImgOut->buf0;

    if(plane == 3){
        out->buf[2] = av_buffer_create(ImgOut->buf2,ImgOut->bmImg->width * ImgOut->bmImg->height / 2 /2 ,
            bmBufferDeviceMemFree2,NULL,AV_BUFFER_FLAG_READONLY);
        out->data[2] = ImgOut->buf0;
    }

    if(plane == 3 && !out->buf[2]){
        av_buffer_unref(&out->buf[0]);
        av_buffer_unref(&out->buf[1]);
        av_buffer_unref(&out->buf[2]);
        free(ImgOut);
        free(in);
        return BM_ERR_FAILURE;
    }
    else if(plane == 2 && !out->buf[1]){
        av_buffer_unref(&out->buf[0]);
        av_buffer_unref(&out->buf[1]);
        free(ImgOut);
        free(in);
        return BM_ERR_FAILURE;
    }

    bm_device_mem_t mem_tmp[3];
    if(bm_image_get_device_mem(*(ImgOut->bmImg),mem_tmp) != BM_SUCCESS ){
        free(ImgOut);
        free(in);
        return BM_ERR_FAILURE;
    }
    if(bm_image_get_format_info(ImgOut->bmImg, &image_info) != BM_SUCCESS ){
        free(ImgOut);
        free(in);
        return BM_ERR_FAILURE;
    }
    for (idx=0; idx< plane; idx++) {
        out->data[4+idx]     = (uint8_t *)mem_tmp[idx].u.device.device_addr;
        out->linesize[idx]   = image_info.stride[idx];
        out->linesize[4+idx] = image_info.stride[idx];
    }
    return BM_SUCCESS;
}

/**
 * @name    bm_image_from_frame
 * @brief   Convert ffmpeg a avframe object to a BMCV bm_image object
 * @ingroup bmruntime
 *
 * @param [in]           bm_handle   the low level device handle
 * @param [in]           in          a read-only avframe
 * @param [out]          out         an uninitialized BMCV bm_image object.
                         if avframe is compressed format,you need use
                         bm_image_destroy function to free out parameter until you
                         no longer useing it.
 * @retval BM_SUCCESS    change success.
 * @retval other values  change failed.
 */

static inline bm_status_t bm_image_from_frame (bm_handle_t       &bm_handle,
                                               AVFrame           &in,
                                               bm_image          &out) {
  if (in.format != AV_PIX_FMT_NV12) {
    std::cout << "format donot support" << std::endl;
    return BM_NOT_SUPPORTED;
  }

  if (in.channel_layout == 101) { /* COMPRESSED NV12 FORMAT */
    /* sanity check */
    if ((0 == in.height) || (0 == in.width) || \
	(0 == in.linesize[4]) || (0 == in.linesize[5]) || (0 == in.linesize[6]) || (0 == in.linesize[7]) || \
	(0 == in.data[4]) || (0 == in.data[5]) || (0 == in.data[6]) || (0 == in.data[7])) {
      std::cout << "bm_image_from_frame: get yuv failed!!" << std::endl;
      return BM_ERR_PARAM;
    }
    bm_image cmp_bmimg;
    bm_image_create (bm_handle,
		     in.height,
		     in.width,
		     FORMAT_COMPRESSED,
		     DATA_TYPE_EXT_1N_BYTE,
		     &cmp_bmimg,
         NULL);

    /* calculate physical address of avframe */
    bm_device_mem_t input_addr[4];
    int size = in.height * in.linesize[4];
    input_addr[0] = bm_mem_from_device((unsigned long long)in.data[6], size);
    size = (in.height / 2) * in.linesize[5];
    input_addr[1] = bm_mem_from_device((unsigned long long)in.data[4], size);
    size = in.linesize[6];
    input_addr[2] = bm_mem_from_device((unsigned long long)in.data[7], size);
    size = in.linesize[7];
    input_addr[3] = bm_mem_from_device((unsigned long long)in.data[5], size);
    bm_image_attach(cmp_bmimg, input_addr);

    bm_image_create (bm_handle,
		   in.height,
		   in.width,
		   FORMAT_YUV420P,
		   DATA_TYPE_EXT_1N_BYTE,
		   &out,
       NULL);

    bm_image_alloc_dev_mem(out, BMCV_HEAP_ANY);
    bmcv_rect_t crop_rect = {0, 0, in.width, in.height};
    bmcv_image_vpp_convert(bm_handle, 1, cmp_bmimg, &out, &crop_rect, BMCV_INTER_LINEAR);
    bm_image_destroy_allinone(&cmp_bmimg);
  } else { /* UNCOMPRESSED NV12 FORMAT */
    /* sanity check */
    if ((0 == in.height) || (0 == in.width) || \
	(0 == in.linesize[4]) || (0 == in.linesize[5]) || \
	(0 == in.data[4]) || (0 == in.data[5])) {
      std::cout << "bm_image_from_frame: get yuv failed!!" << std::endl;
      return BM_ERR_PARAM;
    }

    /* create bm_image with YUV-nv12 format */
    int stride[2];
    stride[0] = in.linesize[4];
    stride[1] = in.linesize[5];
    bm_image_create (bm_handle,
		     in.height,
		     in.width,
		     FORMAT_NV12,
		     DATA_TYPE_EXT_1N_BYTE,
		     &out,
		     stride);

    /* calculate physical address of yuv mat */
    bm_device_mem_t input_addr[2];
    int size = in.height * stride[0];
    input_addr[0] = bm_mem_from_device((unsigned long long)in.data[4], size);
    size = in.height * stride[1];
    input_addr[1] = bm_mem_from_device((unsigned long long)in.data[5], size);

    /* attach memory from mat to bm_image */
    bm_image_attach(out, input_addr);
  }

  return BM_SUCCESS;
}

/**
 * @name    bm_image_from_frame
 * @brief   Convert ffmpeg avframe  to BMCV bm_image object
 * @ingroup bmruntime
 *
 * @param [in]           bm_handle   the low level device handle
 * @param [in]           in          a read-only ffmpeg avframe vector
 * @param [out]          out         an uninitialized BMCV bm_image vector
 * @retval BM_SUCCESS    change success.
 * @retval other values  change failed.
 */
static inline bm_status_t bm_image_from_frame (bm_handle_t           &bm_handle,
                                               std::vector<AVFrame>  &in,
                                               std::vector<bm_image> &out) {

  /* sanity check */
  if (in.empty()) {
    std::cout << "bm_image_from_mat: input empty!!" << std::endl;
    return BM_ERR_PARAM;
  }

  if (!out.empty()) {
    out.clear();
  }

  /* convert avframe to bm_image one by one */
  for (size_t i = 0; i < in.size(); i++) {
    bm_image tmp;
    bm_image_from_frame (bm_handle, in[i], tmp);
    out.push_back (tmp);
  }

  return BM_SUCCESS;
}

//Copy from ff_avframe_convert
#ifndef USEING_MEM_HEAP1
#define USEING_MEM_HEAP1 2
#endif

static int avframe_to_bm_image(bm_handle_t &bm_handle,AVFrame &in, bm_image &out){

    int plane                 = 0;
    int data_five_denominator = -1;
    int data_six_denominator  = -1;
    static int mem_flags = USEING_MEM_HEAP1;


    switch(in.format){
    case AV_PIX_FMT_GRAY8:
        plane = 1;
        data_five_denominator = -1;
        data_six_denominator = -1;
        break;
    case AV_PIX_FMT_YUV420P:
        plane = 3;
        data_five_denominator = 4;
        data_six_denominator = 4;
        break;
    case AV_PIX_FMT_NV12:
        plane = 2;
        data_five_denominator = 2;
        data_six_denominator = -1;
        break;
    case AV_PIX_FMT_YUV422P:
        plane = 3;
        data_five_denominator = 2;
        data_six_denominator = 2;
        break;
    case AV_PIX_FMT_NV16:
        plane = 2;
        data_five_denominator = 2;
        data_six_denominator = -1;
        break;
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_GBRP:
        plane = 3;
        data_five_denominator = 1;
        data_six_denominator = 1;
        break;
    default:
        printf("unsupported format, only gray,nv12,yuv420p,nv16,yuv422p horizontal,yuv444p,rgbp supported\n");
        break;
    }

    if (in.channel_layout == 101) {/* COMPRESSED NV12 FORMAT */
        if ((0 == in.height) || (0 == in.width) || \
         (0 == in.linesize[4]) || (0 == in.linesize[5]) || (0 == in.linesize[6]) || (0 == in.linesize[7]) || \
         (0 == in.data[4]) || (0 == in.data[5]) || (0 == in.data[6]) || (0 == in.data[7])) {
          printf("bm_image_from_frame: get yuv failed!!");
          return BM_ERR_PARAM;
        }
        bm_image cmp_bmimg;
        bm_image_create (bm_handle,
                  in.height,
                  in.width,
                  FORMAT_COMPRESSED,
                  DATA_TYPE_EXT_1N_BYTE,
                  &cmp_bmimg,
                  NULL
                  );

        bm_device_mem_t input_addr[4];
        int size = in.height * in.linesize[4];
        input_addr[0] = bm_mem_from_device((unsigned long long)in.data[6], size);
        size = (in.height / 2) * in.linesize[5];
        input_addr[1] = bm_mem_from_device((unsigned long long)in.data[4], size);
        size = in.linesize[6];
        input_addr[2] = bm_mem_from_device((unsigned long long)in.data[7], size);
        size = in.linesize[7];
        input_addr[3] = bm_mem_from_device((unsigned long long)in.data[5], size);
        bm_image_attach(cmp_bmimg, input_addr);
        bm_image_create (bm_handle,
                in.height,
                in.width,
                FORMAT_YUV420P,
                DATA_TYPE_EXT_1N_BYTE,
                &out,
                NULL
                );
        //bm_image_dev_mem_alloc(out);
        if(mem_flags == USEING_MEM_HEAP1 && bm_image_alloc_dev_mem_heap_mask(out,USEING_MEM_HEAP1) != BM_SUCCESS){
            printf("bmcv allocate mem failed!!!");
        }

        bmcv_rect_t crop_rect = {0, 0, in.width, in.height};
        bmcv_image_vpp_convert(bm_handle, 1, cmp_bmimg, &out, &crop_rect,BMCV_INTER_LINEAR);
        bm_image_destroy(&cmp_bmimg);
    }
    else {
        int stride[3];
        bm_image_format_ext bm_format;
        bm_device_mem_t input_addr[3] = {0};
        if(plane == 1){
            if ((0 == in.height) || (0 == in.width) ||(0 == in.linesize[4]) || (0 == in.data[4])) {
                return BM_ERR_PARAM;
            }
            stride[0] = in.linesize[4];
        }
        else if (plane == 2){
            if ((0 == in.height) || (0 == in.width) || \
                (0 == in.linesize[4]) || (0 == in.linesize[5]) || \
                (0 == in.data[4]) || (0 == in.data[5])) {
                return BM_ERR_PARAM;
            }

            stride[0] = in.linesize[4];
            stride[1] = in.linesize[5];
        }
        else if(plane == 3){
            if ((0 == in.height) || (0 == in.width) || \
                (0 == in.linesize[4]) || (0 == in.linesize[5]) || (0 == in.linesize[6]) || \
                (0 == in.data[4]) || (0 == in.data[5]) || (0 == in.data[6])) {
              return BM_ERR_PARAM;
            }

            stride[0] = in.linesize[4];
            stride[1] = in.linesize[5];
            stride[2] = in.linesize[6];
        }

        bm_format = (bm_image_format_ext)map_avformat_to_bmformat(in.format);
        bm_image_create (bm_handle,
                  in.height,
                  in.width,
                  bm_format,
                  DATA_TYPE_EXT_1N_BYTE,
                  &out,
                  stride
                  );

        int size = in.height * stride[0];
        input_addr[0] = bm_mem_from_device((unsigned long long)in.data[4], size);
        if(data_five_denominator != -1 ){
            size = in.height * stride[1] / data_five_denominator;
            input_addr[1] = bm_mem_from_device((unsigned long long)in.data[5], size);
        }
        if(data_six_denominator != -1){
            size = in.height * stride[2] / data_six_denominator;
            input_addr[2] = bm_mem_from_device((unsigned long long)in.data[6], size);
        }
        bm_image_attach(out, input_addr);
    }
   return BM_SUCCESS;
}

static int AVFrameConvert(bm_handle_t &bmHandle,AVFrame *inPic,AVFrame *outPic,int enc_frame_height,int enc_frame_width,int enc_pix_format,int enable_mosaic = 0,int enable_watermark = 0, bm_device_mem_t* watermark = NULL){

    static int mem_flags = USEING_MEM_HEAP1;
    if(!inPic){
        return -1;
    }

    bm_image *bmImagein    = NULL;
    bm_image *bmImageout   = NULL;
    int ret                   = 0;
    //release this func
    bmImagein = (bm_image *) malloc(sizeof(bm_image));
    //release by callback
    bmImageout = (bm_image *) malloc(sizeof(bm_image));
    if(!bmImagein || !bmImageout){
        return -1;
    }
    av_frame_unref(outPic);
    if(avframe_to_bm_image(bmHandle,*inPic,*bmImagein)!= BM_SUCCESS){
        return -1;
    }

    bm_image_format_ext bmOutFormat = (bm_image_format_ext)map_avformat_to_bmformat(enc_pix_format);
    int stride_yuv = ((enc_frame_width +31) >> 5) << 5;
    int stride_bmi[3] = {0};
    if(FORMAT_NV12 == bmOutFormat){
        stride_bmi[0] = stride_bmi[1] = stride_yuv;
        stride_bmi[2] = 0;
    }else{
        stride_bmi[0] = stride_yuv;
        stride_bmi[1] = stride_bmi[2] = stride_yuv / 2;
    }
    bm_image_create(bmHandle,enc_frame_height,enc_frame_width,bmOutFormat,DATA_TYPE_EXT_1N_BYTE, bmImageout,stride_bmi);
    //vpu cannot use other heap memory(soc and pcie)

    if(mem_flags == USEING_MEM_HEAP1 && bm_image_alloc_dev_mem_heap_mask(*bmImageout,USEING_MEM_HEAP1) != BM_SUCCESS){
        printf("bmcv allocate mem failed!!!");
    }

    if(!bm_image_is_attached(*bmImageout)){
        return -1;
    }
    bmcv_rect_t crop_rect = {0, 0, inPic->width, inPic->height};
    ret = bmcv_image_vpp_convert(bmHandle, 1, *bmImagein, bmImageout ,&crop_rect,BMCV_INTER_LINEAR);
    if(ret != BM_SUCCESS){
        printf("convert error coed = %d\n",ret);
        return -1;
    }

    /* mosaic process */
    if (enable_mosaic){
        bmcv_rect_t mosaic_rect = {0, 0, bmImageout->width/2, bmImageout->height/2};
        ret = bmcv_image_mosaic(bmHandle, 1, *bmImageout, &mosaic_rect, 0);
        if(ret != BM_SUCCESS){
            printf("mosaic_process failed \n");
            return -1;
        }
    }

    /* watermark process*/
    if (enable_watermark){
        int interval = 300;
        int water_w = 120;
        int water_h = 80;
        int font_idx = 0;

        bmcv_rect_t * rect;
        bmcv_color_t color;
        color.r = 128;
        color.g = 128;
        color.b = 128;

        if(bmImageout->width >= water_w + interval && bmImageout->height >= water_h + interval){
            int font_num = ((bmImageout->width - water_w) / (water_w + interval) + 1) * ((bmImageout->height - water_h) / (water_h + interval) + 1);
            rect = (bmcv_rect_t*)malloc(sizeof(bmcv_rect_t)*font_num);
            for(int w = 0; w < ((bmImageout->width - water_w) / (water_w + interval) + 1); w++){
                for(int h = 0; h < ((bmImageout->height - water_h) / (water_h + interval) + 1); h++){
                    rect[font_idx].start_x = w * (water_w + interval);
                    rect[font_idx].start_y = h * (water_h + interval);
                    rect[font_idx].crop_w = water_w;
                    rect[font_idx].crop_h = water_h;
                    font_idx = font_idx + 1;
                }
            }
            bmcv_image_watermark_repeat_superpose(bmHandle, *bmImageout, *watermark, font_num, 0, water_w, rect, color);
            free(rect);
        }
    }

    if(bm_image_to_avframe(bmHandle,bmImageout,outPic) != 0){
        return -1;
    }
    bm_image_destroy(bmImagein);

    free(bmImagein);
    return 0;
}

#endif // USE_FFMPEG

/**
 * @name    bm_image_copy_buffer
 * @brief   Copy a malloc() buffer to BMCV bm_image object
 * @ingroup bmruntime
 *
 * @param [in]           in          input buffer
 * @param [in]           size        input buffer size which must be equal to bm_image's size
 * @param [out]          out         an BMCV bm_image object initialized with bm_image_create()
 * @retval BM_SUCCESS    change success.
 * @retval other values  change failed.
 */
static inline bm_status_t bm_image_copy_buffer (void *in, int size, bm_image &out) {

  /* sanity check */
  if (NULL == in) {
    std::cout << "bm_image_copy_buffer: input is NULL!!" << std::endl;
    return BM_ERR_PARAM;
  }

  /* get bm_image size */
  bm_status_t res;
  int length = 0;
  res = bm_image_get_byte_size (out , &length);
  if (BM_SUCCESS != res) {
    std::cout << "bm_image_copy_buffer: get bm_image size failed!!" << std::endl;
    return res;
  }

  /* input buffer size must be equal to bm_image's size */
  if (size != length) {
    std::cout << "bm_image_copy_buffer: input size match bm_image failed!!" << std::endl;
    return BM_ERR_PARAM;
  }

  /* copy memory from system to device */
  res = bm_image_copy_host_to_device (out, &in);
  if (BM_SUCCESS != res) {
    std::cout << "bm_image_copy_buffer: copy data to bm_image failed!!" << std::endl;
    return res;
  }

  return BM_SUCCESS;
}

/**
 * @name    bm_image_copy_buffer
 * @brief   Copy malloc buffers to BMCV bm_image objects
 * @ingroup bmruntime
 *
 * @param [in]           in          an input buffer vector
 * @param [in]           size        an input buffer size vector
 * @param [out]          out         an initialized BMCV bm_image object vector
 * @retval BM_SUCCESS    change success.
 * @retval other values  change failed.
 */
static inline bm_status_t bm_image_copy_buffer (const std::vector<void *> &in,
                                         const std::vector<int>    &size,
                                         std::vector<bm_image>     &out) {

  /* sanity check */
  if ((in.size() != size.size()) || (in.size() != out.size())) {
    std::cout << "bm_image_copy_buffer: inputs size match failed!!" << std::endl;
    return BM_ERR_PARAM;
  }

  /* copy buffer to bm_image one by one */
  for (size_t i = 0; i < in.size(); i++) {
    bm_status_t res = bm_image_copy_buffer (in[i], size[i], out[i]);
    if (!res) {
      std::cout << "bm_image_copy_buffer: change the buffer " << " i " << "failed!!" << std::endl;
      return res;
    }
  }

  return BM_SUCCESS;
}

/**
 * @name    bm_image_create_batch
 * @brief   create bm images with continuous device memory
 * @ingroup bmruntime
 *
 * @param [in]           handle       handle of low level device
 * @param [in]           img_h        image height
 * @param [in]           img_w        image width
 * @param [in]           img_format   format of image: BGR or YUV
 * @param [in]           data_type    data type of image: INT8 or FP32
 * @param [out]          image        pointer of bm image object
 * @param [in]           batch_num    batch size
 * @param [in]           stride       bm_image stride
 * @param [in]           head_id      bm_image head id
 * @retval BM_SUCCESS    change success.
 * @retval other values  change failed.
 */
static inline bm_status_t bm_image_create_batch (bm_handle_t              handle,
                                                 int                      img_h,
                                                 int                      img_w,
                                                 bm_image_format_ext      img_format,
                                                 bm_image_data_format_ext data_type,
                                                 bm_image                 *image,
                                                 int                      batch_num,
                                                 int                      *stride = NULL,
                                                 int                      heap_mask = -1) {
  bm_status_t res;
  // init images
  for (int i = 0; i < batch_num; i++) {
    if (stride != NULL)
        bm_image_create(handle, img_h, img_w, img_format, data_type, &image[i], stride);
    else
        bm_image_create(handle, img_h, img_w, img_format, data_type, &image[i], stride);
  }

  // alloc continuous memory for multi-batch
  if (-1 == heap_mask)
      res = bm_image_alloc_contiguous_mem (batch_num, image, BMCV_HEAP_ANY);
  else
      res = bm_image_alloc_contiguous_mem_heap_mask (batch_num, image, heap_mask);
  return res;
}

/**
 * @name    bm_image_destroy_batch
 * @brief   destroy bm images with continuous device memory
 * @ingroup bmruntime
 *
 * @param [in]           image        pointer of bm image object
 * @param [in]           batch_num    batch size
 * @retval BM_SUCCESS    change success.
 * @retval other values  change failed.
 */
static inline bm_status_t bm_image_destroy_batch (bm_image *image, int batch_num) {
  bm_status_t res;
  // free memory
  res = bm_image_free_contiguous_mem (batch_num, image);

  // deinit bm image
  for (int i = 0; i < batch_num; i++) {
    bm_image_destroy_allinone (&image[i]);
  }

  return res;
}

/**
 * @name    bm_inference
 * @brief   a inference wrapper call supporting multi-input & multi-output
 * @ingroup bmruntime
 *
 * @param [in]    p_bmrt         the pointer of contxt
 * @param [in]    inputs         a vector of bm_images containing multi-input data
 * @param [out]   outputs        a vector of output buffer pointers
 * @param [in]    input_shapes   a vector of input shapes
 * @param [in]    net_name       a string of certain neuron network name
 *
 * @retval true    Launch success.
 * @retval false   Launch failed.
 */
static inline bool bm_inference (void                      *p_bmrt,
                                 std::vector<bm_image*>    &inputs,
                                 std::vector<void*>        &outputs,
                                 std::vector<bm_shape_t>   &input_shapes,
                                 const char                *net_name) {

  /* sanity check */
  if ((NULL == p_bmrt) || (NULL == net_name)) {
    std::cout << "bm_inference: input error!" << std::endl;
    return false;
  }

  if ((inputs.empty()) || (outputs.empty()) || (input_shapes.empty())) {
    std::cout << "bm_inference: input error!" << std::endl;
    return false;
  }

  if (inputs.size() != input_shapes.size()) {
    std::cout << "bm_inference: input error!" << std::endl;
    return false;
  }

  auto net_info = bmrt_get_network_info(p_bmrt, net_name);
  if (NULL == net_info) {
    std::cout << "ERROR: get net-info failed!" << std::endl;
    return false;
  }

  if (inputs.size() != (size_t)net_info->input_num) {
    std::cout << "ERROR: input number error!!" << std::endl;
    return false;
  }

  if (outputs.size() != (size_t)net_info->output_num) {
    std::cout << "ERROR: output number error!!" << std::endl;
    return false;
  }

  /* initialize input tensors */
  bm_tensor_t input_tensors[net_info->input_num];
  bm_tensor_t output_tensors[net_info->output_num];

  for (size_t i =0; i < inputs.size(); i++) {
    if (NULL == inputs[i]) {
      std::cout << "bm_inference: input " << i <<"NULL!" << std::endl;
      return false;
    }

    if (DATA_TYPE_EXT_FLOAT32 == inputs[i]->data_type) {
      input_tensors[i].dtype = BM_FLOAT32;
    } else if (DATA_TYPE_EXT_1N_BYTE_SIGNED == inputs[i]->data_type) {
      input_tensors[i].dtype = BM_INT8;
    } else if (DATA_TYPE_EXT_1N_BYTE == inputs[i]->data_type) {
      input_tensors[i].dtype = BM_UINT8;
    }  else {
      std::cout << "bm_inference: input type error !" << std::endl;
      return false;
    }

    input_tensors[i].shape = input_shapes[i];

    input_tensors[i].st_mode = BM_STORE_1N;

    /* attach input memory from bm_images to input tensors */
    int batch_number = input_shapes[i].dims[0];
    bm_image_get_contiguous_device_mem (batch_number, inputs[i], &input_tensors[i].device_mem);
  }

  /* do inference, unblock call */
  bm_handle_t bm_handle = (bm_handle_t)bmrt_get_bm_handle (p_bmrt);
  bool ret = bmrt_launch_tensor (p_bmrt, net_name, input_tensors, inputs.size(), output_tensors, outputs.size());
  if (!ret) {
    std::cout << "bm_inference: Failed to launch network" << net_name << "inference" << std::endl;
    return false;
  }

  /* wait for inference done */
  bm_status_t res = (bm_status_t)bm_thread_sync (bm_handle);
  if (res != BM_SUCCESS) {
    std::cout << "bm_inference: Failed to sync" << net_name << "inference" << std::endl;
    for (size_t i =0; i < outputs.size(); i++) {
      bm_free_device (bm_handle, output_tensors[i].device_mem);
    }
    return false;
  }

  /* copy device inference results to system output buffers */
  for (size_t i =0; i < outputs.size(); i++) {
    if (NULL == outputs[i]) {
      std::cout << "bm_inference: out "<< i << "is NULL!" << std::endl;
      return false;
    }

    size_t size_o = bmrt_tensor_bytesize(&output_tensors[i]);
    bm_memcpy_d2s_partial (bm_handle, outputs[i], output_tensors[i].device_mem, size_o);
    bm_free_device (bm_handle, output_tensors[i].device_mem);
  }

  return true;
}

/**
 * @name    bm_inference
 * @brief   a inference wrapper call supporting single-input & single-output
 * @ingroup bmruntime
 *
 * @param [in]    p_bmrt         the pointer of contxt
 * @param [in]    input          a pointer of bm_image containing input data
 * @param [out]   output         a pointer of output buffer
 * @param [in]    input_shape    input shape
 * @param [in]    net_name       a string of certain neuron network name
 *
 * @retval true    Launch success.
 * @retval false   Launch failed.
 */
static inline bool bm_inference (void         *p_bmrt,
                                 bm_image     *input,
                                 void         *output,
                                 bm_shape_t    input_shape,
                                 const char   *net_name) {

  /* sanity check */
  if ((NULL == p_bmrt) || (NULL == input) || (NULL == output) || (NULL == net_name)) {
    std::cout << "bm_inference: input error!" << std::endl;
    return false;
  }

  std::vector<bm_image*> inputs;
  inputs.push_back (input);

  std::vector<void*> outputs;
  outputs.push_back (output);

  std::vector<bm_shape_t> input_shapes;
  input_shapes.push_back (input_shape);

  /* inference */
  bool result = bm_inference (p_bmrt, inputs, outputs, input_shapes, net_name);

  return result;
}

/**
 * @name    bm_inference
 * @brief   a inference wrapper call supporting single-input & multi-output
 * @ingroup bmruntime
 *
 * @param [in]    p_bmrt         the pointer of contxt
 * @param [in]    input          a pointer of bm_image containing input data
 * @param [out]   outputs        a vector of output buffer pointers
 * @param [in]    input_shape    input shape
 * @param [in]    net_name       a string of certain neuron network name
 *
 * @retval true    Launch success.
 * @retval false   Launch failed.
 */
static inline bool bm_inference (void                  *p_bmrt,
                                 bm_image              *input,
                                 std::vector<void*>     outputs,
                                 bm_shape_t             input_shape,
                                 const char            *net_name) {

  /* sanity check */
  if ((NULL == p_bmrt) || (NULL == input) || (NULL == net_name) || (outputs.empty())) {
    std::cout << "bm_inference: input error!" << std::endl;
    return false;
  }

  std::vector<bm_image*> inputs;
  inputs.push_back (input);

  std::vector<bm_shape_t> input_shapes;
  input_shapes.push_back (input_shape);

  /* inference */
  bool result = bm_inference (p_bmrt, inputs, outputs, input_shapes, net_name);

  return result;
}

/**
 * @name    bm_image_dumpdata
 * @brief   Convert BMCV bm_image memory to bin
 * @ingroup bmcv
 *
 * @param [in]           in            bm_image object
 * @param [out]          output_name   a save file name ,if not exit would build it , if exit would clear it
 * @retval BM_SUCCESS    change success.
 * @retval other values  change failed.
 */
static inline bm_status_t bm_image_dumpdata (bm_image &in ,
                                      const char *output_name) {

  /* sanity check */
  if (NULL == output_name) {
    std::cout << "bm_image_dumpdata: OUT file name err!!" << std::endl;
    return BM_ERR_PARAM;
  }
  if ((0 == in.width) || (0 == in.height)) {
    std::cout << "input image err!!" << std::endl;
    return BM_ERR_PARAM;
  }

  int size = 0;
  bm_status_t ret = bm_image_get_byte_size (in , &size);
  if (ret != BM_SUCCESS) {
    std::cout << "get image size err!!" << std::endl;
    return ret;
  }

  unsigned char *data;
  data = new unsigned char[size];
  if (NULL == data) {
    std::cout << "malloc memory failed!!" << std::endl;
    return BM_ERR_PARAM;
  }
  memset (data , 0, size);

  FILE *fp;
  fp = fopen (output_name , "wb+");
  if (NULL == fp) {
    std::cout << "open file failed!!" << std::endl;
    return BM_ERR_PARAM;
  }

  ret = bm_image_copy_device_to_host (in , (void**)&data);
  if (ret != BM_SUCCESS) {
    std::cout << "copy device data err!!" << std::endl;
    return ret;
  }

  fwrite (data , size , 1 , fp);

  delete (data);
  fclose (fp);

  return BM_SUCCESS;
}

#endif // _BM_WRAPPER_HPP_
