#ifndef BMCV_API_EXT_H
#define BMCV_API_EXT_H

#include "bmlib_runtime.h"
#ifdef _WIN32
#ifndef NULL
  #ifdef __cplusplus
    #define NULL 0
  #else
    #define NULL ((void *)0)
  #endif
#endif
#define DECL_EXPORT __declspec(dllexport)
#define DECL_IMPORT __declspec(dllimport)
#else
#define DECL_EXPORT
#define DECL_IMPORT
#endif

/*
 * bmcv api with the new interface.
 */

#ifndef WIN32
typedef struct {
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
} __attribute__((packed)) face_rect_t;
#else
#pragma pack(push, 1)
typedef struct {
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
} face_rect_t;
#pragma pack(pop)
#endif

#define MIN_PROPOSAL_NUM (1)
#define MAX_PROPOSAL_NUM (40000) //(65535)

#ifndef WIN32
typedef struct nms_proposal {
    int          size;
    face_rect_t  face_rect[MAX_PROPOSAL_NUM];
    int          capacity;
    face_rect_t *begin;
    face_rect_t *end;
} __attribute__((packed)) nms_proposal_t;
#else
#pragma pack(push, 1)
typedef struct nms_proposal {
    int          size;
    face_rect_t  face_rect[MAX_PROPOSAL_NUM];
    int          capacity;
    face_rect_t *begin;
    face_rect_t *end;
} nms_proposal_t;
#pragma pack(pop)
#endif

#define MAX_RECT_NUM (8 * 1024)
#ifndef WIN32
typedef struct {
    face_rect_t  face_rect[MAX_RECT_NUM];
    int          size;
    int          capacity;
    face_rect_t *begin;
    face_rect_t *end;
} __attribute__((packed)) m_proposal_t;
#else
#pragma pack(push, 1)
typedef struct {
    face_rect_t  face_rect[MAX_RECT_NUM];
    int          size;
    int          capacity;
    face_rect_t *begin;
    face_rect_t *end;
} m_proposal_t;
#pragma pack(pop)
#endif

typedef enum bmcv_heap_id_ {
    BMCV_HEAP0_ID = 0,
    BMCV_HEAP1_ID = 1,
    BMCV_HEAP_ANY
} bmcv_heap_id;

// BMCV_IMAGE_FOR_IN and BMCV_IMAGE_FOR_OUT may be deprecated in future version.
// We recommend not use this.
#define BMCV_IMAGE_FOR_IN BMCV_HEAP1_ID
#define BMCV_IMAGE_FOR_OUT BMCV_HEAP0_ID

// typedef unsigned char u8;
// typedef unsigned short u16;
// typedef unsigned int u32;
// typedef unsigned long long u64;

// typedef signed char  s8;
// typedef signed short s16;
// typedef signed int   s32;
// typedef signed long long int s64;

typedef enum bm_image_data_format_ext_ {
    DATA_TYPE_EXT_FLOAT32,
    DATA_TYPE_EXT_1N_BYTE,
    DATA_TYPE_EXT_4N_BYTE,
    DATA_TYPE_EXT_1N_BYTE_SIGNED,
    DATA_TYPE_EXT_4N_BYTE_SIGNED,
    DATA_TYPE_EXT_FP16,
    DATA_TYPE_EXT_BF16,
} bm_image_data_format_ext;

typedef enum bm_image_format_ext_ {
    FORMAT_YUV420P,
    FORMAT_YUV422P,
    FORMAT_YUV444P,
    FORMAT_NV12,
    FORMAT_NV21,
    FORMAT_NV16,
    FORMAT_NV61,
    FORMAT_NV24,
    FORMAT_RGB_PLANAR,
    FORMAT_BGR_PLANAR,
    FORMAT_RGB_PACKED,
    FORMAT_BGR_PACKED,
    FORMAT_RGBP_SEPARATE,
    FORMAT_BGRP_SEPARATE,
    FORMAT_GRAY,
    FORMAT_COMPRESSED,
    FORMAT_HSV_PLANAR,
    FORMAT_ARGB_PACKED,
    FORMAT_ABGR_PACKED,
    FORMAT_YUV444_PACKED,
    FORMAT_YVU444_PACKED,
    FORMAT_YUV422_YUYV,
    FORMAT_YUV422_YVYU,
    FORMAT_YUV422_UYVY,
    FORMAT_YUV422_VYUY,
    FORMAT_RGBYP_PLANAR,
    FORMAT_HSV180_PACKED,
    FORMAT_HSV256_PACKED,
} bm_image_format_ext;

typedef enum csc_type {
    CSC_YCbCr2RGB_BT601 = 0,
    CSC_YPbPr2RGB_BT601,
    CSC_RGB2YCbCr_BT601,
    CSC_YCbCr2RGB_BT709,
    CSC_RGB2YCbCr_BT709,
    CSC_RGB2YPbPr_BT601,
    CSC_YPbPr2RGB_BT709,
    CSC_RGB2YPbPr_BT709,
    CSC_USER_DEFINED_MATRIX = 1000,
    CSC_MAX_ENUM
} csc_type_t;

struct bm_image_private;

typedef struct bm_image{
    int                       width;
    int                       height;
    bm_image_format_ext       image_format;
    bm_image_data_format_ext  data_type;
    struct bm_image_private  *image_private;
} bm_image;

typedef struct bm_image_tensor{
    bm_image image;
    int      image_c;
    int      image_n;
} bm_image_tensor;

typedef struct bm_image_format_info {
    int                      plane_nb;
    bm_device_mem_t          plane_data[8];
    int                      stride[8];
    int                      width;
    int                      height;
    bm_image_format_ext      image_format;
    bm_image_data_format_ext data_type;
    bool                     default_stride;
} bm_image_format_info_t;

typedef struct bmcv_convert_to_attr_s {
    float alpha_0;
    float beta_0;
    float alpha_1;
    float beta_1;
    float alpha_2;
    float beta_2;
} bmcv_convert_to_attr;

typedef struct bmcv_rect {
    unsigned int start_x;
    unsigned int start_y;
    unsigned int crop_w;
    unsigned int crop_h;
} bmcv_rect_t;

typedef enum bmcv_resize_algorithm_ {
    BMCV_INTER_NEAREST = 0,
    BMCV_INTER_LINEAR  = 1,
    BMCV_INTER_BICUBIC = 2
} bmcv_resize_algorithm;

typedef struct bmcv_padding_atrr_s {
    unsigned int  dst_crop_stx;
    unsigned int  dst_crop_sty;
    unsigned int  dst_crop_w;
    unsigned int  dst_crop_h;
    unsigned char padding_r;
    unsigned char padding_g;
    unsigned char padding_b;
    int           if_memset;
} bmcv_padding_attr_t;

typedef struct {
    unsigned short csc_coe00;
    unsigned short csc_coe01;
    unsigned short csc_coe02;
    unsigned char csc_add0;
    unsigned short csc_coe10;
    unsigned short csc_coe11;
    unsigned short csc_coe12;
    unsigned char csc_add1;
    unsigned short csc_coe20;
    unsigned short csc_coe21;
    unsigned short csc_coe22;
    unsigned char csc_add2;
} csc_matrix_t;

typedef struct bmcv_resize_s {
    int start_x;
    int start_y;
    int in_width;
    int in_height;
} bmcv_resize_t;

typedef struct bmcv_resize_image_s {
    bmcv_resize_t *resize_img_attr;
    int            roi_num;
    unsigned char  stretch_fit;
    unsigned char  padding_b;
    unsigned char  padding_g;
    unsigned char  padding_r;
    unsigned int   interpolation;
} bmcv_resize_image;

typedef struct bmcv_copy_to_atrr_s {
    int           start_x;
    int           start_y;
    unsigned char padding_r;
    unsigned char padding_g;
    unsigned char padding_b;
    int           if_padding;
} bmcv_copy_to_atrr_t;

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} bmcv_color_t;

/** bm_image_create
 * @brief Create and fill bm_image structure
 * @param [in] handle                     The bm handle which return by
 * bm_dev_request.
 * @param [in] img_h                      The height or rows of the creating
 * image.
 * @param [in] img_w                     The width or cols of the creating
 * image.
 * @param [in] image_format      The image_format of the creating image,
 *  please choose one from bm_image_format_ext enum.
 * @param [in] data_type               The data_type of the creating image,
 *  be caution that not all combinations between image_format and data_type
 *  are supported.
 * @param [in] stride                        the stride array for planes, each
 * number in array means corresponding plane pitch stride in bytes. The plane
 * size is determinated by image_format. If this array is null, we may use
 * default value.
 *  @param [out] image                   The filled bm_image structure.
 *  For example, we need create a 480x480 NV12 format image, we know that NV12
 * format has 2 planes, we need pitch stride is 256 aligned(just for example) so
 * the pitch stride for the first plane is 512, so as the same for the second
 * plane.
 * The call may as following
 * bm_image res;
 *  int stride[] = {512, 512};
 *  bm_image_create(handle, 480, 480, FORMAT_NV12, DATA_TYPE_EXT_1N_BYTE, &res,
 * stride); If bm_image_create return BM_SUCCESS, res is created successfully.
 */
DECL_EXPORT bm_status_t bm_image_create(bm_handle_t              handle,
                                        int                      img_h,
                                        int                      img_w,
                                        bm_image_format_ext      image_format,
                                        bm_image_data_format_ext data_type,
                                        bm_image *               image,
                                        int *                    stride);

/** bm_image_destroy
 * @brief Destroy bm_image and free the corresponding system memory and device
 * memory.
 * @param [in] image                     The bm_image structure ready to
 * destroy. If bm_image_destroy return BM_SUCCESS, image is destroy successfully
 * and the corresponding system memory and device memory are freed.
 */
DECL_EXPORT bm_status_t bm_image_destroy(bm_image *image);
DECL_EXPORT bm_handle_t bm_image_get_handle(bm_image *image);
DECL_EXPORT bm_status_t bm_image_copy_host_to_device(bm_image image, void *buffers[]);
DECL_EXPORT bm_status_t bm_image_copy_device_to_host(bm_image image, void *buffers[]);
DECL_EXPORT bm_status_t bm_image_attach(bm_image image, bm_device_mem_t *device_memory);
DECL_EXPORT bm_status_t bm_image_detach(bm_image image);
DECL_EXPORT bool        bm_image_is_attached(bm_image);
DECL_EXPORT int         bm_image_get_plane_num(bm_image);
DECL_EXPORT bm_status_t bm_image_get_stride(bm_image image, int *stride);
DECL_EXPORT bm_status_t bm_image_get_format_info(bm_image *image, bm_image_format_info_t *info);
DECL_EXPORT bm_status_t bm_image_alloc_dev_mem(bm_image image, int heap_id);
DECL_EXPORT bm_status_t bm_image_alloc_dev_mem_heap_mask(bm_image image, int heap_mask);
DECL_EXPORT bm_status_t bm_image_get_byte_size(bm_image image, int *size);
DECL_EXPORT bm_status_t bm_image_get_device_mem(bm_image image, bm_device_mem_t *mem);
DECL_EXPORT bm_status_t bm_image_alloc_contiguous_mem(int image_num, bm_image *images, int heap_id);
DECL_EXPORT bm_status_t bm_image_alloc_contiguous_mem_heap_mask(int image_num, bm_image *images, int heap_mask);
DECL_EXPORT bm_status_t bm_image_free_contiguous_mem(int image_num, bm_image *images);
DECL_EXPORT bm_status_t bm_image_attach_contiguous_mem(int image_num, bm_image *images, bm_device_mem_t dmem);
DECL_EXPORT bm_status_t bm_image_detach_contiguous_mem(int image_num, bm_image *images);
DECL_EXPORT bm_status_t bm_image_get_contiguous_device_mem(int image_num, bm_image *images, bm_device_mem_t *mem);
DECL_EXPORT bm_status_t bmcv_width_align(bm_handle_t handle, bm_image input, bm_image output);

bm_status_t bm_image_tensor_get_device_mem(bm_image_tensor  image_tensor,
                                           bm_device_mem_t *mem);
bool bm_image_tensor_is_attached(bm_image_tensor image_tensor);

bm_status_t bm_image_tensor_destroy(bm_image_tensor image_tensor);

bm_status_t bm_image_tensor_alloc_dev_mem(bm_image_tensor image_tensor,
                                          int             heap_id);

bm_status_t concat_images_to_tensor(bm_handle_t      handle,
                                    int              image_num,
                                    bm_image *       images,
                                    bm_image_tensor *image_tensor);

DECL_EXPORT void algorithm_to_str(bmcv_resize_algorithm algorithm, char* res);

DECL_EXPORT void format_to_str(bm_image_format_ext format, char* res);

DECL_EXPORT int is_csc_yuv_or_rgb(bm_image_format_ext format);

DECL_EXPORT bm_status_t bmcv_image_convert_to(
    bm_handle_t          handle,
    int                  input_num,
    bmcv_convert_to_attr convert_to_attr,
    bm_image *           input,
    bm_image *           output);


DECL_EXPORT bm_status_t bmcv_image_vpp_convert(
    bm_handle_t           handle,
    int                   output_num,
    bm_image              input,
    bm_image *            output,
    bmcv_rect_t *         crop_rect,
    bmcv_resize_algorithm algorithm);

DECL_EXPORT bm_status_t bmcv_image_vpp_basic(
    bm_handle_t           handle,
    int                   in_img_num,
    bm_image*             input,
    bm_image*             output,
    int*                  crop_num_vec,
    bmcv_rect_t*          crop_rect,
    bmcv_padding_attr_t*  padding_attr,
    bmcv_resize_algorithm algorithm,
    csc_type_t            csc_type,
    csc_matrix_t*         matrix);

DECL_EXPORT bm_status_t bmcv_image_vpp_csc_matrix_convert(
    bm_handle_t           handle,
    int                   output_num,
    bm_image              input,
    bm_image*             output,
    csc_type_t            csc,
    csc_matrix_t*         matrix,
    bmcv_resize_algorithm algorithm,
    bmcv_rect_t*          crop_rect);

DECL_EXPORT bm_status_t bmcv_image_storage_convert(
    bm_handle_t      handle,
    int              image_num,
    bm_image*        input_,
    bm_image*        output_);

DECL_EXPORT bm_status_t bmcv_image_storage_convert_with_csctype(
    bm_handle_t      handle,
    int              image_num,
    bm_image*        input_,
    bm_image*        output_,
    csc_type_t       csc_type);

DECL_EXPORT bm_status_t bmcv_image_vpp_convert_padding(
    bm_handle_t             handle,
    int                     output_num,
    bm_image                input,
    bm_image*               output,
    bmcv_padding_attr_t*    padding_attr,
    bmcv_rect_t*            crop_rect,
    bmcv_resize_algorithm   algorithm);

DECL_EXPORT bm_status_t bmcv_image_draw_rectangle(
    bm_handle_t   handle,
    bm_image      image,
    int           rect_num,
    bmcv_rect_t * rects,
    int           line_width,
    unsigned char r,
    unsigned char g,
    unsigned char b);

DECL_EXPORT bm_status_t bmcv_image_csc_convert_to(
    bm_handle_t             handle,
    int                     img_num,
    bm_image*               input,
    bm_image*               output,
    int*                    crop_num_vec,
    bmcv_rect_t*            crop_rect,
    bmcv_padding_attr_t*    padding_attr,
    bmcv_resize_algorithm   algorithm,
    csc_type_t              csc_type,
    csc_matrix_t*           matrix,
    bmcv_convert_to_attr*   convert_to_attr);

DECL_EXPORT bm_status_t bmcv_image_copy_to(
    bm_handle_t         handle,
    bmcv_copy_to_atrr_t copy_to_attr,
    bm_image            input,
    bm_image            output);

DECL_EXPORT bm_status_t bmcv_image_vpp_stitch(
    bm_handle_t           handle,
    int                   input_num,
    bm_image*             input,
    bm_image              output,
    bmcv_rect_t*          dst_crop_rect,
    bmcv_rect_t*          src_crop_rect,
    bmcv_resize_algorithm algorithm);

DECL_EXPORT bm_status_t bmcv_image_mosaic(
    bm_handle_t           handle,
    int                   mosaic_num,
    bm_image              input,
    bmcv_rect_t *         mosaic_rect,
    int                   is_expand);

DECL_EXPORT bm_status_t bmcv_image_fill_rectangle(
    bm_handle_t             handle,
    bm_image                image,
    int                     rect_num,
    bmcv_rect_t *           rects,
    unsigned char           r,
    unsigned char           g,
    unsigned char           b);

DECL_EXPORT bm_status_t bmcv_image_watermark_superpose(
	bm_handle_t           handle,
	bm_image *            image,
	bm_device_mem_t *     bitmap_mem,
	int                   bitmap_num,
	int                   bitmap_type,
	int                   pitch,
	bmcv_rect_t *         rects,
	bmcv_color_t          color);

DECL_EXPORT bm_status_t bmcv_image_watermark_repeat_superpose(
	bm_handle_t         handle,
	bm_image            image,
	bm_device_mem_t     bitmap_mem,
	int                 bitmap_num,
	int                 bitmap_type,
	int                 pitch,
	bmcv_rect_t *       rects,
	bmcv_color_t        color);

// quality_factor = 84
DECL_EXPORT bm_status_t bmcv_image_jpeg_enc(
    bm_handle_t handle,
    int         image_num,
    bm_image *  src,
    void **     p_jpeg_data,
    size_t *    out_size,
    int         quality_factor);

DECL_EXPORT bm_status_t bmcv_image_jpeg_dec(
        bm_handle_t handle,
        void **     p_jpeg_data,
        size_t *    in_size,
        int         image_num,
        bm_image *  dst);

/* bmcv ive api */
typedef struct bmcv_ive_add_attr_s {
    unsigned short param_x;
    unsigned short param_y;
} bmcv_ive_add_attr;

DECL_EXPORT bm_status_t bmcv_image_ive_add(
    bm_handle_t          handle,
    bmcv_ive_add_attr    add_attr,
    bm_image *           input1,
    bm_image *           input2,
    bm_image *           output);

DECL_EXPORT bm_status_t bmcv_image_absdiff(
    bm_handle_t handle,
    bm_image input1,
    bm_image input2,
    bm_image output);

DECL_EXPORT bm_status_t  bmcv_image_axpy(
        bm_handle_t handle,
        bm_device_mem_t tensor_A,
        bm_device_mem_t tensor_X,
        bm_device_mem_t tensor_Y,
        bm_device_mem_t tensor_F,
        int input_n,
        int input_c,
        int input_h,
        int input_w);

DECL_EXPORT bm_status_t bmcv_image_add_weighted(
    bm_handle_t handle,
    bm_image input1,
    float alpha,
    bm_image input2,
    float beta,
    float gamma,
    bm_image output);

DECL_EXPORT bm_status_t bmcv_image_bitwise_and(
        bm_handle_t handle,
        bm_image input1,
        bm_image input2,
        bm_image output);

DECL_EXPORT bm_status_t bmcv_image_bitwise_or(
        bm_handle_t handle,
        bm_image input1,
        bm_image input2,
        bm_image output);

DECL_EXPORT bm_status_t bmcv_image_bitwise_xor(
        bm_handle_t handle,
        bm_image input1,
        bm_image input2,
        bm_image output);

#endif