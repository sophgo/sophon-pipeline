//
// Created by yuan on 8/24/21.
//

#ifndef MIDDLEWARE_SOC_BMVPPAPI_H
#define MIDDLEWARE_SOC_BMVPPAPI_H

#include <stdint.h>

#ifdef _WIN32
#define DECL_EXPORT __declspec(dllexport)
    #define DECL_IMPORT __declspec(dllimport)
#else
#define DECL_EXPORT
#define DECL_IMPORT
#endif

#define BMVPP_OK 0
#define BMVPP_ERR -1


/* Color space supported in BM1684 */
enum {
    BMVPP_FMT_GREY        = 0,    /* Y only */
    BMVPP_FMT_I420        = 1,    /* YUV420 plannar */
    BMVPP_FMT_NV12        = 2,    /* YUV420 semi-plannar */
    BMVPP_FMT_BGR         = 3,    /* packed BGR, 24 bits, (low) B-G-R (high) */
    BMVPP_FMT_RGB         = 4,    /* packed RGB, 24 bits, (low) R-G-B (high) */
    BMVPP_FMT_RGBP        = 5,    /* rgb 24 plannar with three separate planes */
    BMVPP_FMT_BGRP        = 6,    /* bgr 24 plannar with three separate planes */
    BMVPP_FMT_YUV444P     = 7,    /* yuv444 plannar with three separate planes */
    BMVPP_FMT_YUV422P     = 8,    /* yuv422 plannar with three separate planes */
    BMVPP_FMT_YUV444      = 9,    /* packed yuv444, 24 bits */
    BMVPP_FMT_ABGR        = 10,   /* packed ABGR, 32 bits, (low) B-G-R-A (high) */
    BMVPP_FMT_ARGB        = 11,   /* packed ARGB, 32 bits, (low) A-R-G-B (high) */
};

enum {
    BMVPP_RESIZE_BILINEAR  = 0,
    BMVPP_RESIZE_NEAREST   = 1,
    BMVPP_RESIZE_BICUBIC   = 2,
};

typedef struct bmvpp_mat_s {
    int      format;      /* image data format */
    int      color_range; /* 0: limited range; 1: full range */
    int      colorspace;  /* 0: bt-601, 1: bt-709 */

    int      num_comp;    /* channel number of data blocks. TODO
                           * 1-packed data; 2 - yuv420sp; 3 - yuv420p */

    uint64_t pa[4];       /* Physical address of continous memory */
    int      size[4];     /* Memory size of each channels */

    int      width;       /* image width  */
    int      height;      /* image height */
    int      stride;      /* image stride */
    int      c_stride;    /* chrominance stride */

    int      soc_idx;
    void*    reserved;     /*reserved field, no need to fill in*/
} bmvpp_mat_t;

typedef struct bmvpp_rect_s {
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
} bmvpp_rect_t;

typedef enum bmvpp_log_level_enum{
    BMVPP_LOG_ERR   = 1,
    BMVPP_LOG_WARN  = 2,
    BMVPP_LOG_INFO  = 3,
    BMVPP_LOG_DEBUG = 4,
    BMVPP_LOG_TRACE = 5,
}bmvpp_log_level_t;

DECL_EXPORT int bmvpp_open(int soc_idx);
DECL_EXPORT int bmvpp_close(int soc_idx);
DECL_EXPORT int bmvpp_get_status(int soc_idx);
DECL_EXPORT void bmvpp_set_log_level(bmvpp_log_level_t level);
DECL_EXPORT int bmvpp_log_level(void);


DECL_EXPORT int bmvpp_scale(int is_compressed, bmvpp_mat_t* src, bmvpp_rect_t* loca, bmvpp_mat_t* dst, int dst_x, int dst_y, int resize_method);
DECL_EXPORT int bmvpp_J422pToJ420p(int is_compressed, bmvpp_mat_t* src, bmvpp_rect_t* loca, bmvpp_mat_t* dst, int dst_x, int dst_y, int resize_method);

#endif //MIDDLEWARE_SOC_BMVPPAPI_H
