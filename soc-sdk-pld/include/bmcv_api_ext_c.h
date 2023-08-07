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

#if defined(__cplusplus)
extern "C" {
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

typedef enum {
    LINEAR_WEIGHTING = 0,
    GAUSSIAN_WEIGHTING,
    MAX_WEIGHTING_TYPE
} weighting_method_e;

typedef enum bmcv_heap_id_ {
    BMCV_HEAP0_ID = 0,
    BMCV_HEAP1_ID = 1,
    BMCV_HEAP_ANY
} bmcv_heap_id;

// BMCV_IMAGE_FOR_IN and BMCV_IMAGE_FOR_OUT may be deprecated in future version.
// We recommend not use this.
#define BMCV_IMAGE_FOR_IN BMCV_HEAP1_ID
#define BMCV_IMAGE_FOR_OUT BMCV_HEAP0_ID

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

typedef enum bmcv_resize_algorithm_ {
    BMCV_INTER_NEAREST = 0,
    BMCV_INTER_LINEAR  = 1,
    BMCV_INTER_BICUBIC = 2
} bmcv_resize_algorithm;

typedef enum bm_cv_nms_alg_ {
    HARD_NMS = 0,
    SOFT_NMS,
    ADAPTIVE_NMS,
    SSD_NMS,
    MAX_NMS_TYPE
} bm_cv_nms_alg_e;

struct bm_image_private;

typedef struct bm_image_s {
    int                      width;
    int                      height;
    bm_image_format_ext      image_format;
    bm_image_data_format_ext data_type;
    struct bm_image_private *       image_private;
}bm_image;

typedef struct bmcv_rect {
    int start_x;
    int start_y;
    int crop_w;
    int crop_h;
} bmcv_rect_t;

#ifndef WIN32
typedef struct yolov7_info{
  int scale;
  int *orig_image_shape;
  int model_h;
  int model_w;
} __attribute__((packed)) yolov7_info_t;
#else
#pragma pack(push, 1)
typedef struct yolov7_info{
  int scale;
  int *orig_image_shape;
  int model_h;
  int model_w;
} yolov7_info_t;
#pragma pack(pop)
#endif

typedef struct bmcv_copy_to_atrr_s {
    int           start_x;
    int           start_y;
    unsigned char padding_r;
    unsigned char padding_g;
    unsigned char padding_b;
    int           if_padding;
} bmcv_copy_to_atrr_t;

typedef struct bmcv_padding_atrr_s {
    unsigned int  dst_crop_stx;
    unsigned int  dst_crop_sty;
    unsigned int  dst_crop_w;
    unsigned int  dst_crop_h;
    unsigned char padding_r;
    unsigned char padding_g;
    unsigned char padding_b;
    int           if_memset;
} bmcv_padding_atrr_t;

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

typedef struct {
    int x;
    int y;
} bmcv_point_t;

typedef struct {
    float x;
    float y;
} bmcv_point2f_t;

typedef struct {
    int type;   // 1: maxCount   2: eps   3: both
    int max_count;
    double epsilon;
} bmcv_term_criteria_t;

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} bmcv_color_t;

typedef struct {
    int csc_coe00;
    int csc_coe01;
    int csc_coe02;
    int csc_add0;
    int csc_coe10;
    int csc_coe11;
    int csc_coe12;
    int csc_add1;
    int csc_coe20;
    int csc_coe21;
    int csc_coe22;
    int csc_add2;
} csc_matrix_t;

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

typedef enum {
    BM_THRESH_BINARY = 0,
    BM_THRESH_BINARY_INV,
    BM_THRESH_TRUNC,
    BM_THRESH_TOZERO,
    BM_THRESH_TOZERO_INV,
    BM_THRESH_TYPE_MAX
} bm_thresh_type_t;

typedef enum {
    BM_MORPH_RECT,
    BM_MORPH_CROSS,
    BM_MORPH_ELLIPSE
} bmcv_morph_shape_t;

// const char *bm_get_bmcv_version();

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
                            bm_image *        image,
                            int *                    stride);

/** bm_image_destroy
 * @brief Destroy bm_image and free the corresponding system memory and device
 * memory.
 * @param [in] image                     The bm_image structure ready to
 * destroy. If bm_image_destroy return BM_SUCCESS, image is destroy successfully
 * and the corresponding system memory and device memory are freed.
 */
DECL_EXPORT bm_status_t bm_image_destroy(bm_image image);

/** bm_image_get_handle
 * @brief return the device handle, this handle is exactly the first parameter
 * when bm_image_create called.
 * @param [in] image                                   The bm_image structure
 *  @param [return] bm_handle_t          The device handle where bm_image bind
 * to. If image is not created by bm_image_create, this function would return
 * NULL.
 */
DECL_EXPORT bm_handle_t bm_image_get_handle(bm_image *image);

/** bm_image_write_to_bmp
 * @brief dump this bm_image to .bmp file.
 * @param [in] image                 The bm_image structure you would like to
 * dump
 *  @param [in] filename           path and filename for the creating bmp file,
 * it's better end with ".bmp" If bm_image_write_to_bmp return BM_SUCCESS, a
 * .bmp file is create in the path filename point to.
 */
bm_status_t bm_image_write_to_bmp(bm_image    image,
                                             const char *filename);

DECL_EXPORT bm_status_t bm_image_copy_host_to_device(bm_image image,
                                                    void *   buffers[]);
DECL_EXPORT bm_status_t bm_image_copy_device_to_host(bm_image image,
                                                     void *   buffers[]);

DECL_EXPORT bm_status_t bm_image_attach(bm_image image, bm_device_mem_t *device_memory);
DECL_EXPORT bm_status_t bm_image_detach(bm_image);
DECL_EXPORT bool        bm_image_is_attached(bm_image);
DECL_EXPORT int         bm_image_get_plane_num(bm_image);
DECL_EXPORT bm_status_t bm_image_get_stride(bm_image image, int *stride);
DECL_EXPORT bm_status_t bm_image_get_format_info(bm_image *            image,
                                     struct bm_image_format_info *info);

DECL_EXPORT bm_status_t bm_image_alloc_dev_mem(bm_image image,
                                              int      heap_id);
DECL_EXPORT bm_status_t bm_image_alloc_dev_mem_heap_mask(bm_image image, int heap_mask);
DECL_EXPORT bm_status_t bm_image_get_byte_size(bm_image image, int *size);
DECL_EXPORT bm_status_t bm_image_get_device_mem(bm_image image, bm_device_mem_t *mem);

DECL_EXPORT bm_status_t bm_image_alloc_contiguous_mem(int       image_num,
                                          bm_image *images,
                                          int       heap_id);
DECL_EXPORT bm_status_t bm_image_alloc_contiguous_mem_heap_mask(int       image_num,
                                                    bm_image *images,
                                                    int       heap_mask);
DECL_EXPORT bm_status_t bm_image_free_contiguous_mem(int image_num, bm_image *images);
DECL_EXPORT bm_status_t bm_image_attach_contiguous_mem(int             image_num,
                                           bm_image *      images,
                                           bm_device_mem_t dmem);
DECL_EXPORT bm_status_t bm_image_dettach_contiguous_mem(int image_num, bm_image *images);

DECL_EXPORT bm_status_t bm_image_get_contiguous_device_mem(int              image_num,
                                               bm_image *       images,
                                               bm_device_mem_t *mem);

DECL_EXPORT bm_status_t bmcv_image_yuv2bgr_ext(bm_handle_t handle,
                                   int         image_num,
                                   bm_image *  input,
                                   bm_image *  output);

DECL_EXPORT bm_status_t bmcv_image_yuv2hsv(bm_handle_t handle,
                               bmcv_rect_t rect,
                               bm_image    input,
                               bm_image    output);

DECL_EXPORT bm_status_t bmcv_image_storage_convert(bm_handle_t handle,
                                       int         image_num,
                                       bm_image *  input,
                                       bm_image *  output);

DECL_EXPORT bm_status_t bmcv_image_storage_convert_with_csctype(bm_handle_t handle,
                                                    int         image_num,
                                                    bm_image *  input,
                                                    bm_image *  output,
                                                    csc_type_t  csc_type);
DECL_EXPORT bm_status_t bmcv_image_copy_to(bm_handle_t         handle,
                               bmcv_copy_to_atrr_t copy_to_attr,
                               bm_image            input,
                               bm_image            output);
DECL_EXPORT bm_status_t bmcv_image_crop(bm_handle_t         handle,
                            int                 crop_num,
                            bmcv_rect_t *       rects,
                            bm_image            input,
                            bm_image *          output);
DECL_EXPORT bm_status_t bmcv_image_split(bm_handle_t         handle,
                             bm_image            input,
                             bm_image *          output);

typedef struct bmcv_affine_matrix_s {
    float m[6];
} bmcv_affine_matrix;

typedef struct bmcv_affine_image_matrix_s {
    bmcv_affine_matrix *matrix;
    int                 matrix_num;
} bmcv_affine_image_matrix;

typedef struct bmcv_perspective_matrix_s {
    float m[9];
} bmcv_perspective_matrix;

typedef struct bmcv_perspective_image_matrix_s {
    bmcv_perspective_matrix *matrix;
    int                      matrix_num;
} bmcv_perspective_image_matrix;

typedef struct bmcv_perspective_coordinate_s {
    int x[4];
    int y[4];
} bmcv_perspective_coordinate;

typedef struct bmcv_perspective_image_coordinate_s {
    bmcv_perspective_coordinate *coordinate;
    int                         coordinate_num;
} bmcv_perspective_image_coordinate;

typedef struct bmcv_resize_s {
    int start_x;
    int start_y;
    int in_width;
    int in_height;
    int out_width;
    int out_height;
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

typedef struct bmcv_convert_to_attr_s {
    float alpha_0;
    float beta_0;
    float alpha_1;
    float beta_1;
    float alpha_2;
    float beta_2;
} bmcv_convert_to_attr;

/**
 * @brief Do warp affine operation with the transform matrix.
 *        For 1N mode, only support 4 images.
 *        For 4N mode, only support 1 images.
 * @param [in] handle        The bm handle which return by bm_dev_request.
 * @param [in] image_num    The really input image number, should be less than
 *or equal to 4.
 * @param [in] matrix        The input transform matrix and matrix number for
 *each image.
 * @param [in] input        The input bm image, could be 1N or 4N.
 *                for each image. And do any operation if matrix[n] is nullptr.
 * @param [out]            The output image, could be 1N or 4N.
 *                If setting to 1N, the output image number should have summary
 *of matrix_num[n]. If setting to 4N, the output image number should have
 *summary of ROUNDUP(matrix_num[n], 4)/4
 */
DECL_EXPORT bm_status_t bmcv_image_warp_affine(
        bm_handle_t              handle,
        int                      image_num,
        bmcv_affine_image_matrix matrix[4],
        bm_image *               input,
        bm_image *               output,
        int                      use_bilinear);

DECL_EXPORT bm_status_t bmcv_image_warp_affine_similar_to_opencv(
        bm_handle_t              handle,
        int                      image_num,
        bmcv_affine_image_matrix matrix[4],
        bm_image *               input,
        bm_image *               output,
        int                      use_bilinear);

DECL_EXPORT bm_status_t bmcv_image_warp_perspective(
        bm_handle_t                   handle,
        int                           image_num,
        bmcv_perspective_image_matrix matrix[4],
        bm_image *                    input,
        bm_image *                    output,
        int                           use_bilinear);

DECL_EXPORT bm_status_t bmcv_image_warp_perspective_with_coordinate(
        bm_handle_t                       handle,
        int                               image_num,
        bmcv_perspective_image_coordinate coordinate[4],
        bm_image *                        input,
        bm_image *                        output,
        int                               use_bilinear);

DECL_EXPORT bm_status_t bmcv_image_warp_perspective_similar_to_opencv(
    bm_handle_t                       handle,
    int                               image_num,
    bmcv_perspective_image_matrix     matrix[4],
    bm_image *                        input,
    bm_image *                        output,
    int                               use_bilinear);

DECL_EXPORT bm_status_t bmcv_image_resize(
        bm_handle_t          handle,
        int                  input_num,
        bmcv_resize_image    resize_attr[],
        bm_image *           input,
        bm_image *           output);

DECL_EXPORT bm_status_t bmcv_hamming_distance(bm_handle_t handle,
                                  bm_device_mem_t input1,
                                  bm_device_mem_t input2,
                                  bm_device_mem_t output,
                                  int bits_len,
                                  int input1_num,
                                  int input2_num);

DECL_EXPORT bm_status_t bmcv_image_yuv_resize(
                              bm_handle_t       handle,
                              int               input_num,
                              bm_image *        input,
                              bm_image *        output);

DECL_EXPORT bm_status_t bmcv_image_convert_to(
        bm_handle_t          handle,
        int                  input_num,
        bmcv_convert_to_attr convert_to_attr,
        bm_image *           input,
        bm_image *           output);

bm_status_t bmcv_width_align(
        bm_handle_t handle,
        bm_image    input,
        bm_image    output);

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

DECL_EXPORT bm_status_t bmcv_nms(
        bm_handle_t     handle,
        bm_device_mem_t input_proposal_addr,
        int             proposal_size,
        float           nms_threshold,
        bm_device_mem_t output_proposal_addr);

DECL_EXPORT bm_status_t bmcv_nms_ext(
        bm_handle_t     handle,
        bm_device_mem_t input_proposal_addr,
        int             proposal_size,
        float           nms_threshold,
        bm_device_mem_t output_proposal_addr,
        int             topk,
        float           score_threshold,
        int             nms_alg,
        float           sigma,
        int             weighting_method,
        float         * densities,
        float           eta);

#define BMCV_YOLOV3_DETECT_OUT_MAX_NUM 200
DECL_EXPORT bm_status_t bmcv_nms_yolov3(bm_handle_t      handle,
        //input
        int              input_num,
        bm_device_mem_t  bottom[3],
        int              batch_num,
        int              hw[3][2],
        int              num_classes,
        int              num_boxes,
        int              mask_group_size,
        float            nms_threshold,
        float            confidence_threshold,
        int              keep_top_k,
        float            bias[18],
        float            anchor_scale[3],
        float            mask[9],
        bm_device_mem_t  output,
        int              yolov5_flag,
        int              len_per_batch);

DECL_EXPORT bm_status_t bmcv_nms_yolo(
        bm_handle_t handle,
        int input_num,
        bm_device_mem_t bottom[3],
        int batch_num,
        int hw_shape[3][2],
        int num_classes,
        int num_boxes,
        int mask_group_size,
        float nms_threshold,
        float confidence_threshold,
        int keep_top_k,
        float bias[18],
        float anchor_scale[3],
        float mask[9],
        bm_device_mem_t output,
        int yolov5_flag,
        int len_per_batch,
        void *ext);

DECL_EXPORT bm_status_t bmcv_image_draw_rectangle(
        bm_handle_t   handle,
        bm_image      image,
        int           rect_num,
        bmcv_rect_t * rect,
        int           line_width,
        unsigned char r,
        unsigned char g,
        unsigned char b);

DECL_EXPORT bm_status_t bmcv_image_fill_rectangle(
        bm_handle_t   handle,
        bm_image      image,
        int           rect_num,
        bmcv_rect_t * rect,
        unsigned char r,
        unsigned char g,
        unsigned char b);

DECL_EXPORT bm_status_t bmcv_sort(
        bm_handle_t     handle,
        bm_device_mem_t src_index_addr,
        bm_device_mem_t src_data_addr,
        int             data_cnt,
        bm_device_mem_t dst_index_addr,
        bm_device_mem_t dst_data_addr,
        int             sort_cnt,
        int             order,
        bool            index_enable,
        bool            auto_index);
/**
 * @brief: calculate topk for each db, return BM_SUCCESS if succeed.
 * @param handle           [in]: the device handle.
 * @param src_data_addr    [in]: device addr information of input_data.
 * @param src_index_addr   [in]: device addr information of input_index, set it if src_index_valid.
 * @param dst_data_addr    [out]: device addr information of output_data
 * @param dst_index_addr   [in]: device addr information of output_index.
 * @param buffer_addr      [in]: device addr information of buffer.
 *        if (max(per_batch_cnt) <= 10000 || (max(per_batch_cnt) <=200000 && topk <=64))
 *          buffer_size = (batch * k + max_cnt) * sizeof(float) * 2
 *        else
 *          buffer_size = ceil(max(per_per_batch_cnt)/4000000) * 3*sizeof(float)*(k>10000? 2*k : 10000)
 * @param src_index_valid  [in]: if true, use src_index, otherwise gen index auto.
 * @param k                [in]: k value
 * @param batch            [in]: batch numeber
 * @param per_batch_cnt    [in]: data_number of per_batch
 * @param src_batch_stride [in]: distance between two batches
 * @param descending       [in]: descending or ascending.
 */
DECL_EXPORT bm_status_t bmcv_batch_topk(
        bm_handle_t     handle,
        bm_device_mem_t src_data_addr,
        bm_device_mem_t src_index_addr,
        bm_device_mem_t dst_data_addr,
        bm_device_mem_t dst_index_addr,
        bm_device_mem_t buffer_addr,
        bool            src_index_valid,
        int             k,
        int             batch,
        int *           per_batch_cnt,
        bool            same_batch_cnt,
        int             src_batch_stride,
        bool            descending);

bm_status_t bmcv_feature_match_normalized(
        bm_handle_t     handle,
        bm_device_mem_t input_data_global_addr,
        bm_device_mem_t db_data_global_addr,
        bm_device_mem_t db_feature_global_addr,
        bm_device_mem_t output_similarity_global_addr,
        bm_device_mem_t output_index_global_addr,
        int             batch_size,
        int             feature_size,
        int             db_size);

DECL_EXPORT bm_status_t bmcv_feature_match(
        bm_handle_t     handle,
        bm_device_mem_t input_data_global_addr,
        bm_device_mem_t db_data_global_addr,
        bm_device_mem_t output_sorted_similarity_global_addr,
        bm_device_mem_t output_sorted_index_global_addr,
        int             batch_size,
        int             feature_size,
        int             db_size,
        int             sort_cnt,
        int             rshiftbits);

DECL_EXPORT bm_status_t bmcv_base64_enc(
        bm_handle_t     handle,
        bm_device_mem_t src,
        bm_device_mem_t dst,
        unsigned long   len[2]);

DECL_EXPORT bm_status_t bmcv_base64_dec(
        bm_handle_t     handle,
        bm_device_mem_t src,
        bm_device_mem_t dst,
        unsigned long   len[2]);

/**
 * @brief: calculate inner product distance between query vectors and database vectors, output the top K IP-values and the corresponding indices, return BM_SUCCESS if succeed.
 * @param handle                               [in]: the device handle.
 * @param input_data_global_addr               [in]: device addr information of the query matrix.
 * @param db_data_global_addr                  [in]: device addr information of the database matrix.
 * @param buffer_global_addr                   [in]: inner product values stored in the buffer.
 * @param output_sorted_similarity_global_addr [out]: the IP-values matrix.
 * @param output_sorted_index_global_addr      [out]: the result indices matrix.
 * @param vec_dims          [in]: vector dimension.
 * @param query_vecs_num    [in]: the num of query vectors.
 * @param database_vecs_num [in]: the num of database vectors.
 * @param sort_cnt          [in]: get top sort_cnt values.
 * @param is_transpose      [in]: db_matrix 0: NO_TRNAS; 1: TRANS.
 * @param input_dtype       [in]: DT_FP32 / DT_INT8.
 * @param output_dtype      [in]: DT_FP32 / DT_INT32.
 */
DECL_EXPORT bm_status_t bmcv_faiss_indexflatIP(
        bm_handle_t     handle,
        bm_device_mem_t input_data_global_addr,
        bm_device_mem_t db_data_global_addr,
        bm_device_mem_t buffer_global_addr,
        bm_device_mem_t output_sorted_similarity_global_addr,
        bm_device_mem_t output_sorted_index_global_addr,
        int             vec_dims,
        int             query_vecs_num,
        int             database_vecs_num,
        int             sort_cnt,
        int             is_transpose,
        int             input_dtype,
        int             output_dtype);

/**
 * @brief: calculate squared L2 distance between query vectors and database vectors, output the top K L2sqr-values and the corresponding indices, return BM_SUCCESS if succeed.
 * @param handle                               [in]: the device handle.
 * @param input_data_global_addr               [in]: device addr information of the query matrix.
 * @param db_data_global_addr                  [in]: device addr information of the database matrix.
 * @param query_L2norm_global_addr             [in]: device addr information of the query norm_L2sqr vector.
 * @param db_L2norm_global_addr                [in]: device addr information of the database norm_L2sqr vector.
 * @param buffer_global_addr                   [in]: squared L2 values stored in the buffer.
 * @param output_sorted_similarity_global_addr [out]: the L2sqr-values matrix.
 * @param output_sorted_index_global_addr      [out]: the result indices matrix.
 * @param vec_dims          [in]: vector dimension.
 * @param query_vecs_num    [in]: the num of query vectors.
 * @param database_vecs_num [in]: the num of database vectors.
 * @param sort_cnt          [in]: get top sort_cnt values.
 * @param is_transpose      [in]: db_matrix 0: NO_TRNAS; 1: TRANS.
 * @param input_dtype       [in]: DT_FP32.
 * @param output_dtype      [in]: DT_FP32.
 */
DECL_EXPORT bm_status_t bmcv_faiss_indexflatL2(
        bm_handle_t     handle,
        bm_device_mem_t input_data_global_addr,
        bm_device_mem_t db_data_global_addr,
        bm_device_mem_t query_L2norm_global_addr,
        bm_device_mem_t db_L2norm_global_addr,
        bm_device_mem_t buffer_global_addr,
        bm_device_mem_t output_sorted_similarity_global_addr,
        bm_device_mem_t output_sorted_index_global_addr,
        int             vec_dims,
        int             query_vecs_num,
        int             database_vecs_num,
        int             sort_cnt,
        int             is_transpose,
        int             input_dtype,
        int             output_dtype);

/**
 * @brief: PQ Asymmetric Distance Computation, output the topK distance and label of x and q(ny), return BM_SUCCESS if succeed.
 * @param handle                         [in]: the device handle.
 * @param centroids_input_dev            [in]: device addr information of the centroids.
 * @param nxquery_input_dev              [in]: device addr information of the query.
 * @param nycodes_input_dev,             [in]: PQcodes of database.
 * @param distance_output_dev            [out]: output topK distance
 * @param index_output_dev               [out]: output topK label
 * @param vec_dims              [in]: vector dimension.
 * @param slice_num             [in]: the num of sliced vector.
 * @param centroids_num         [in]: the num of centroids num.
 * @param database_num          [in]: the num of database vectors.
 * @param query_num             [in]: the num of query vectors.
 * @param sort_cnt              [in]: get top sort_cnt values.
 * @param IP_metric             [in]: metrics 0:L2_matric; 1:IP_matric.
 */
DECL_EXPORT bm_status_t bmcv_faiss_indexPQ_ADC(
        bm_handle_t     handle,
        bm_device_mem_t centroids_input_dev,
        bm_device_mem_t nxquery_input_dev,
        bm_device_mem_t nycodes_input_dev,
        bm_device_mem_t distance_output_dev,
        bm_device_mem_t index_output_dev,
        int             vec_dims,
        int             slice_num,
        int             centroids_num,
        int             database_num,
        int             query_num,
        int             sort_cnt,
        int             IP_metric);

/**
 * @brief: PQ Symmetric Distance Computation, output the topK distance and label of q(x) and q(ny), return BM_SUCCESS if succeed.
 * @param handle                         [in]: the device handle.
 * @param sdc_table_input_dev            [in]: device addr information of the sdc_table.
 * @param nxcodes_input_dev,             [in]: PQcodes of query.
 * @param nycodes_input_dev,             [in]: PQcodes of database.
 * @param distance_output_dev            [out]: output topK distance.
 * @param index_output_dev               [out]: output topK label.
 * @param slice_num             [in]: the num of sliced vector.
 * @param centroids_num         [in]: the num of centroids num.
 * @param database_num          [in]: the num of database vectors.
 * @param query_num             [in]: the num of query vectors.
 * @param sort_cnt              [in]: get top sort_cnt values.
 * @param IP_metric             [in]: metrics 0:L2_matric; 1:IP_matric.
 */
DECL_EXPORT bm_status_t bmcv_faiss_indexPQ_SDC(
        bm_handle_t     handle,
        bm_device_mem_t sdc_table_input_dev,
        bm_device_mem_t nxcodes_input_dev,
        bm_device_mem_t nycodes_input_dev,
        bm_device_mem_t distance_output_dev,
        bm_device_mem_t index_output_dev,
        int             slice_num,
        int             centroids_num,
        int             database_num,
        int             query_num,
        int             sort_cnt,
        int             IP_metric);

/**
 * @brief: encode D-dims vectors into m*int8 PQcodes , return BM_SUCCESS if succeed.
 * @param handle                         [in]: the device handle.
 * @param vector_input_dev               [in]: device addr information of the D-dims vectors
 * @param centroids_input_dev            [in]: device addr information of the centroids.
 * @param buffer_table_dev,              [in]: distance table stored in the buffer,size=nv*m*ksub*dtype.
 * @param codes_output_dev               [out]: output PQcodes, size = nv * m * int8.
 * @param encode_vecs_num   [in]: the num of input vectors.
 * @param vec_dims          [in]: vector dimension.
 * @param slice_num         [in]: the num of sliced vector.
 * @param centroids_num     [in]: the num of centroids.
 */
DECL_EXPORT bm_status_t bmcv_faiss_indexPQ_encode(
        bm_handle_t     handle,
        bm_device_mem_t vector_input_dev,
        bm_device_mem_t centroids_input_dev,
        bm_device_mem_t buffer_table_dev,
        bm_device_mem_t codes_output_dev,
        int             encode_vec_num,
        int             vec_dims,
        int             slice_num,
        int             centroids_num,
        int             IP_metric);

DECL_EXPORT bm_status_t bmcv_debug_savedata(bm_image image, const char *name);

DECL_EXPORT bm_status_t bmcv_image_transpose(bm_handle_t handle,
                                 bm_image input,
                                 bm_image output);

DECL_EXPORT bm_status_t bmcv_matmul(
        bm_handle_t      handle,
        int              M,
        int              N,
        int              K,
        bm_device_mem_t  A,
        bm_device_mem_t  B,
        bm_device_mem_t  C,
        int              A_sign,  // 1: signed 0: unsigned
        int              B_sign,
        int              rshift_bit,
        int              result_type,  // 0:8bit 1:int16 2:fp32
        bool             is_B_trans,
        float            alpha,
        float            beta);

DECL_EXPORT bm_status_t bmcv_matmul_transpose_opt(
        bm_handle_t      handle,
        int              M,
        int              N,
        int              K,
        bm_device_mem_t  A,
        bm_device_mem_t  B,
        bm_device_mem_t  C,
        int              A_sign,  // 1: signed 0: unsigned
        int              B_sign);

DECL_EXPORT bm_status_t bmcv_gemm_ext(
        bm_handle_t      handle,
        bool                is_A_trans,
        bool                is_B_trans,
        int                 M,
        int                 N,
        int                 K,
        float               alpha,
        bm_device_mem_t     A,
        bm_device_mem_t     B,
        float               beta,
        bm_device_mem_t     C,
        bm_device_mem_t     Y,
        bm_image_data_format_ext in_dtype,
        bm_image_data_format_ext out_dtype);

DECL_EXPORT bm_status_t bmcv_image_sobel(
        bm_handle_t handle,
        bm_image input,
        bm_image output,
        int dx,
        int dy,
        int ksize,
        float scale,
        float delta);

DECL_EXPORT bm_status_t  bmcv_image_laplacian(
        bm_handle_t handle,
        bm_image input,
        bm_image output,
        unsigned int ksize);

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

DECL_EXPORT bm_status_t bmcv_image_fusion(
        bm_handle_t handle,
        bm_image input1,
        bm_image input2,
        bm_image output,
        unsigned char thresh,
        unsigned char max_value,
        bm_thresh_type_t type,
        int kw,
        int kh,
        bm_device_mem_t kmem);

DECL_EXPORT bm_status_t bmcv_calc_hist(bm_handle_t handle,
                           bm_device_mem_t input,
                           bm_device_mem_t output,
                           int C,
                           int H,
                           int W,
                           const int *channels,
                           int dims,
                           const int *histSizes,
                           const float *ranges,
                           int inputDtype);

DECL_EXPORT bm_status_t bmcv_calc_hist_with_weight(bm_handle_t handle,
                                       bm_device_mem_t input,
                                       bm_device_mem_t output,
                                       const float *weight,
                                       int C,
                                       int H,
                                       int W,
                                       const int *channels,
                                       int dims,
                                       const int *histSizes,
                                       const float *ranges,
                                       int inputDtype);

DECL_EXPORT bm_status_t bmcv_distance(bm_handle_t handle,
                          bm_device_mem_t input,
                          bm_device_mem_t output,
                          int dim,
                          const float *pnt,
                          int len);

DECL_EXPORT bm_status_t bmcv_distance_ext(bm_handle_t handle,
                          bm_device_mem_t input,
                          bm_device_mem_t output,
                          int dim,
                          bm_device_mem_t pnt,
                          int len,
                          int dtyte);

DECL_EXPORT bm_status_t bmcv_fft_1d_create_plan(bm_handle_t handle,
                                    int batch,
                                    int len,
                                    bool forward,
                                    void *plan);
DECL_EXPORT bm_status_t bmcv_fft_2d_create_plan(bm_handle_t handle,
                                    int M,
                                    int N,
                                    bool forward,
                                    void *plan);
DECL_EXPORT bm_status_t bmcv_fft_execute(bm_handle_t handle,
                             bm_device_mem_t inputReal,
                             bm_device_mem_t inputImag,
                             bm_device_mem_t outputReal,
                             bm_device_mem_t outputImag,
                             const void *plan);
DECL_EXPORT bm_status_t bmcv_fft_execute_real_input(
                             bm_handle_t handle,
                             bm_device_mem_t inputReal,
                             bm_device_mem_t outputReal,
                             bm_device_mem_t outputImag,
                             const void *plan);

DECL_EXPORT void bmcv_fft_destroy_plan(bm_handle_t handle, void *plan);

// mode = 0 for min only, 1 for max only, 2 for both
DECL_EXPORT bm_status_t bmcv_min_max(bm_handle_t handle,
                         bm_device_mem_t input,
                         float *minVal,
                         float *maxVal,
                         int len);

DECL_EXPORT bm_status_t bmcv_cmulp(bm_handle_t handle,
                       bm_device_mem_t inputReal,
                       bm_device_mem_t inputImag,
                       bm_device_mem_t pointReal,
                       bm_device_mem_t pointImag,
                       bm_device_mem_t outputReal,
                       bm_device_mem_t outputImag,
                       int batch,
                       int len);

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

DECL_EXPORT bm_status_t bmcv_image_absdiff(
        bm_handle_t handle,
        bm_image input1,
        bm_image input2,
        bm_image output);

DECL_EXPORT bm_status_t bmcv_image_threshold(
        bm_handle_t handle,
        bm_image input,
        bm_image output,
        unsigned char thresh,
        unsigned char max_value,
        bm_thresh_type_t type);

DECL_EXPORT bm_status_t bmcv_image_gaussian_blur(
        bm_handle_t handle,
        bm_image input,
        bm_image output,
        int kw,
        int kh,
        float sigmaX,
        float sigmaY);

DECL_EXPORT bm_status_t bmcv_image_canny(
        bm_handle_t handle,
        bm_image input,
        bm_image output,
        float threshold1,
        float threshold2,
        int aperture_size,
        bool l2gradient);

DECL_EXPORT bm_status_t bmcv_image_draw_lines(
        bm_handle_t handle,
        bm_image img,
        const bmcv_point_t* start,
        const bmcv_point_t* end,
        int line_num,
        bmcv_color_t color,
        int thickness);

DECL_EXPORT bm_status_t bmcv_image_put_text(
        bm_handle_t handle,
        bm_image image,
        const char* text,
        bmcv_point_t org,
        bmcv_color_t color,
        float fontScale,
        int thickness);

DECL_EXPORT bm_device_mem_t bmcv_get_structuring_element(
        bm_handle_t handle,
        bmcv_morph_shape_t shape,
        int kw,
        int kh
        );

DECL_EXPORT bm_status_t bmcv_image_erode(
        bm_handle_t handle,
        bm_image src,
        bm_image dst,
        int kw,
        int kh,
        bm_device_mem_t kmem
        );

DECL_EXPORT bm_status_t bmcv_image_dilate(
        bm_handle_t handle,
        bm_image src,
        bm_image dst,
        int kw,
        int kh,
        bm_device_mem_t kmem
        );

bm_status_t bmcv_image_pyramid_down(
        bm_handle_t handle,
        bm_image input,
        bm_image output);

DECL_EXPORT bm_status_t bmcv_image_lkpyramid_create_plan(
        bm_handle_t handle,
        void* plan,
        int width,
        int height,
        int winW,
        int winH,
        int maxLevel);

DECL_EXPORT bm_status_t bmcv_image_lkpyramid_execute(
        bm_handle_t handle,
        void* plan,
        bm_image prevImg,
        bm_image nextImg,
        int ptsNum,
        bmcv_point2f_t* prevPts,
        bmcv_point2f_t* nextPts,
        bool* status,
        bmcv_term_criteria_t criteria);

DECL_EXPORT void bmcv_image_lkpyramid_destroy_plan(
        bm_handle_t handle,
        void* plan);

/**
 * @brief bmcv_dct_coeff
 * @param handle
 * @param H
 * @param W
 * @param hcoeff_output: HxH
 * @param wcoeff_output: WxW
 * @param is_inversed: 0-dct, 1-idct
 * @return
 */
DECL_EXPORT bm_status_t bmcv_dct_coeff(
        bm_handle_t handle,
        int H,
        int W,
        bm_device_mem_t hcoeff_output,
        bm_device_mem_t wcoeff_output,
        bool is_inversed
        );

/**
 * @brief bmcv_image_dct_with_coeff which can reuse dct coeff for optimization, output=hcoeff[HxH]*input[HxW]*wcoeff[WxW]
 * @param handle
 * @param input: image
 * @param hcoeff: output from bmcv_dct_coeff
 * @param wcoeff: output from bmcv_dct_coeff
 * @param output: dct output
 * @return
 */
DECL_EXPORT bm_status_t bmcv_image_dct_with_coeff(
        bm_handle_t handle,
        bm_image input,
        bm_device_mem_t hcoeff,
        bm_device_mem_t wcoeff,
        bm_image output
        );

/**
 * @brief bmcv_image_dct: recalculate dct coeff every time, that will cost more time
 * @param handle
 * @param input
 * @param output
 * @return
 */
DECL_EXPORT bm_status_t bmcv_image_dct(
        bm_handle_t handle,
        bm_image input,
        bm_image output,
        bool is_inversed
        );

DECL_EXPORT bm_status_t bmcv_open_cpu_process(bm_handle_t handle);
DECL_EXPORT bm_status_t bmcv_close_cpu_process(bm_handle_t handle);

//#ifndef USING_CMODEL
DECL_EXPORT bm_status_t bmcv_image_vpp_basic(bm_handle_t           handle,
                                 int                   in_img_num,
                                 bm_image*             input,
                                 bm_image*             output,
                                 int*                  crop_num_vec,
                                 bmcv_rect_t*          crop_rect,
                                 bmcv_padding_atrr_t*  padding_attr,
                                 bmcv_resize_algorithm algorithm,
                                 csc_type_t            csc_type,
                                 csc_matrix_t*         matrix);

DECL_EXPORT bm_status_t bmcv_image_vpp_convert(
    bm_handle_t           handle,
    int                   output_num,
    bm_image              input,
    bm_image *            output,
    bmcv_rect_t *         crop_rect,
    bmcv_resize_algorithm algorithm);

DECL_EXPORT bm_status_t bmcv_image_vpp_csc_matrix_convert(bm_handle_t           handle,
                                              int                   output_num,
                                              bm_image              input,
                                              bm_image *            output,
                                              csc_type_t            csc,
                                              csc_matrix_t *        matrix,
                                              bmcv_resize_algorithm algorithm,
                                              bmcv_rect_t *         crop_rect);

DECL_EXPORT bm_status_t bmcv_image_vpp_convert_padding(
    bm_handle_t           handle,
    int                   output_num,
    bm_image              input,
    bm_image *            output,
    bmcv_padding_atrr_t * padding_attr,
    bmcv_rect_t *         crop_rect,
    bmcv_resize_algorithm algorithm);

DECL_EXPORT bm_status_t bmcv_image_vpp_stitch(
    bm_handle_t          handle,
    int                  input_num,
    bm_image*            input,
    bm_image             output,
    bmcv_rect_t*         dst_crop_rect,
    bmcv_rect_t*         src_crop_rect,
    bmcv_resize_algorithm algorithm);
//#endif

/**
 * Legacy functions
 */

typedef bmcv_affine_image_matrix bmcv_warp_image_matrix;
typedef bmcv_affine_matrix bmcv_warp_matrix;

#ifdef __linux__
bm_status_t bmcv_image_warp(
        bm_handle_t            handle,
        int                    image_num,
        bmcv_warp_image_matrix matrix[4],
        bm_image *             input,
        bm_image *output) __attribute__((deprecated));
bm_status_t bm_image_dev_mem_alloc(bm_image image,
                                   int heap_id) __attribute__((deprecated));
#endif

DECL_EXPORT bm_status_t bmcv_image_warp_perspective_similar_to_opencv(
    bm_handle_t                       handle,
    int                               image_num,
    bmcv_perspective_image_matrix     matrix[4],
    bm_image *                        input,
    bm_image *                        output,
    int                               use_bilinear);

DECL_EXPORT bm_status_t bm1684x_vpp_fill_rectangle(
  bm_handle_t          handle,
  int                  input_num,
  bm_image *           input,
  bm_image *           output,
  bmcv_rect_t*         input_crop_rect,
  unsigned char        r,
  unsigned char        g,
  unsigned char        b);

DECL_EXPORT bm_status_t bm1684x_vpp_cmodel_csc_resize_convert_to(
  bm_handle_t             handle,
  int                     frame_number,
  bm_image*               input,
  bm_image*               output,
  bmcv_rect_t*            input_crop_rect,
  bmcv_padding_atrr_t*    padding_attr,
  bmcv_resize_algorithm   algorithm,
  csc_type_t              csc_type,
  csc_matrix_t*           matrix,
  bmcv_convert_to_attr*   convert_to_attr);

DECL_EXPORT bm_status_t bm1684x_vpp_cmodel_border(
  bm_handle_t             handle,
  int                     rect_num,
  bm_image*               input,
  bm_image*               output,
  bmcv_rect_t*            rect,
  int                     line_width,
  unsigned char           r,
  unsigned char           g,
  unsigned char           b);

DECL_EXPORT bm_status_t bmcv_image_mosaic(
  bm_handle_t           handle,
  int                   mosaic_num,
  bm_image              input,
  bmcv_rect_t *         mosaic_rect,
  int                   is_expand);

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
  bm_handle_t           handle,
  bm_image              image,
  bm_device_mem_t       bitmap_mem,
  int                   bitmap_num,
  int                   bitmap_type,
  int                   pitch,
  bmcv_rect_t *         rects,
  bmcv_color_t          color);

DECL_EXPORT bm_status_t bmcv_image_csc_convert_to(
  bm_handle_t             handle,
  int                     img_num,
  bm_image*               input,
  bm_image*               output,
  int*                    crop_num_vec,
  bmcv_rect_t*            crop_rect,
  bmcv_padding_atrr_t*    padding_attr,
  bmcv_resize_algorithm   algorithm,
  csc_type_t              csc_type,
  csc_matrix_t*           matrix,
  bmcv_convert_to_attr*   convert_to_attr);

DECL_EXPORT bm_status_t bmcv_image_vpp_basic_v2(
  bm_handle_t             handle,
  int                     img_num,
  bm_image*               input,
  bm_image*               output,
  int*                    crop_num_vec,
  bmcv_rect_t*            crop_rect,
  bmcv_padding_atrr_t*    padding_attr,
  bmcv_resize_algorithm   algorithm,
  csc_type_t              csc_type,
  csc_matrix_t*           matrix,
  bmcv_convert_to_attr*   convert_to_attr);

#if defined(__cplusplus)
}
#endif

#endif /* BMCV_API_EXT_H */
