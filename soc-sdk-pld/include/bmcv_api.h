#ifndef BMCV_API_H
#define BMCV_API_H
#include "bmlib_runtime.h"
#include "bmcv_api_ext.h"

#if defined (__cplusplus)
extern "C" {
#endif

/*
 * bmcv api for compitable with bm1682(SC3).
 */

#define MAX_bm_image_CHANNEL 4
typedef enum bmcv_data_format_{
    DATA_TYPE_FLOAT = 0,
    DATA_TYPE_BYTE = 4
}bmcv_data_format;

typedef enum bmcv_color_space_{
    COLOR_YUV,
    COLOR_YCbCr,
    COLOR_RGB
}bmcv_color_space;

typedef enum bm_image_format_{
    YUV420P,
    NV12,
    NV21,
    RGB,
    BGR,
    RGB_PACKED,
    BGR_PACKED,
    BGR4N,
    RGB4N
}bm_image_format;

typedef struct bm_image_t{
    bmcv_color_space color_space;
    bmcv_data_format data_format;
    bm_image_format image_format;
    int              image_width;
    int              image_height;
    bm_device_mem_t  data[MAX_bm_image_CHANNEL];
    int              stride[MAX_bm_image_CHANNEL];
}bmcv_image;

bm_status_t bm_img_yuv2bgr(
    bm_handle_t      handle,
    //input
    bmcv_image       input,
    //output
    bmcv_image       output);

bm_status_t bmcv_img_crop(
    bm_handle_t      handle,
    //input
    bmcv_image       input,
    int              channels,
    int              top,
    int              left,
    bmcv_image       output);

bm_status_t  bmcv_img_bgrsplit(
        bm_handle_t handle,
        bmcv_image input,
    bmcv_image output);

bm_status_t  bmcv_img_transpose(
        bm_handle_t handle,
        bmcv_image  input,
    bmcv_image  output);

/**
* \brief Image Scale.
*
* \param handle - bmdnn handle return from bm_init
* \param input  - input bmcv_image
* \param n      - Batch size of image
* \param do_crop  - crop on src image, resize on cropped image
* \param top      - start point most top from original image
* \param left     - start point most left from original image
* \param height   - crop image height
* \param width    - crop image width
* \param input_data_format  - input_data_format: 4 for float, 1 for unsigned char(int8)
* \param output_data_format - input_data_format: 4 for float, 1 for unsigned char(int8)
* \param pixel_weight_bias  - do dst_pixel = dst_pixel * weight + bias
* \param weight_b           - weight for channel b
* \param bias_b             - bias for channel b
* \param weight_g           - weight for channel g
* \param bias_g             - bias for channel g
* \param weight_r           - weight for channel r
* \param bias_r             - bias for channel r
* \param output             - output bmcv_image
*
* \return
* \ref BM_ATOMIC_SUCCESS
*/

bm_status_t bmcv_img_scale(
        bm_handle_t handle,
    bmcv_image  input,
    int n,
        int do_crop, int top, int left, int height, int width,
        unsigned char stretch, unsigned char padding_b, unsigned char padding_g, unsigned char padding_r,
        int pixel_weight_bias,
        float weight_b, float bias_b,
        float weight_g, float bias_g,
        float weight_r, float bias_r,
    bmcv_image  output);

bm_status_t bmcv_gemm(
    bm_handle_t      handle,
    bool                is_A_trans,
    bool                is_B_trans,
    int                 M,
    int                 N,
    int                 K,
    float               alpha,
    bm_device_mem_t     A,
    int                 lda,
    bm_device_mem_t     B,
    int                 ldb,
    float               beta,
    bm_device_mem_t     C,
    int                 ldc);

bm_status_t bm_image_to_bmcv_image(bm_image *src, bmcv_image *dst);
#if defined (__cplusplus)
}
#endif

#endif /* BMCV_API_H */
