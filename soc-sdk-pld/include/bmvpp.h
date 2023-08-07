#ifndef __BM_VPP_H__
#define __BM_VPP_H__

#if defined(__cplusplus)
extern "C" {
#endif

#if defined _WIN32 && !defined(__cplusplus)
#if !defined(bool)
#define	bool	int
#endif
#if !defined(true)
#define true	1
#endif
#if !defined(false)
#define	false	0
#endif
#endif

#if !defined DECL_EXPORT
#ifdef _WIN32
    #define DECL_EXPORT __declspec(dllexport)
#else
    #define DECL_EXPORT
#endif
#endif
#define MIN_RESOLUTION_W    (8)    /*linear mode to linear mode*/
#define MIN_RESOLUTION_H    (8)    /*linear mode to linear mode*/
#define MAX_RESOLUTION_W    (4096)
#define MAX_RESOLUTION_H    (4096)
#define MAX_SCALE_RATIO    (32)

#define VPP_CROP_NUM_MAX (256)

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long int u64;
typedef signed long long int s64;

typedef enum _vpp_scale_type {
    VPP_SCALE_BILINEAR = 0,
    VPP_SCALE_NEAREST  = 1,
    VPP_SCALE_BICUBIC = 2
} vpp_scale_type;

typedef enum _img_fmt {
    FMT_Y        = 0,      /*Y only*/
    FMT_I420     = 1,      /*YUV420 Planar(I420)*/
    FMT_NV12     = 2,      /*YUV420 SemiPlanar(NV12)*/
    FMT_BGR      = 3,      /*BGR Packed 24, (low) B-G-R (high)*/
    FMT_RGB      = 4,      /*RGB Packed 24, (low) R-G-B (high)*/
    FMT_RGBP     = 5,      /*rgb 24 planar,rrrgggbbb,r g b three channels of data in three different physically contiguous memory spaces*/
    FMT_BGRP     = 6,      /*bgr 24 planar,bbbgggrrr,r g b three channels of data in three different physically contiguous memory spaces*/
    FMT_YUV444P  = 7,      /*yuv444 planar,yyyuuuvvv,y u v three channels of data in three different physically contiguous memory spaces*/
    FMT_YUV422P  = 8,      /*yuv422 planar,yyyyuuvv,y u v three channels of data in three different physically contiguous memory spaces*/
    FMT_YUV444   = 9,      /*yuv444 Packed 24,yuvyuvyuv*/
    FMT_ABGR     = 10,     /*ABGR Packed 32, bgrabgra*/
    FMT_ARGB     = 11      /*ARGB Packed 32, argbargb*/
} vpp_img_fmt;

typedef enum _csc_coe_type {
	YCbCr2RGB_BT601  = 0,
	YPbPr2RGB_BT601  = 1,
	RGB2YCbCr_BT601  = 2,
	YCbCr2RGB_BT709  = 3,
	RGB2YCbCr_BT709  = 4,
	RGB2YPbPr_BT601  = 5,
	YPbPr2RGB_BT709  = 6,
	RGB2YPbPr_BT709  = 7,
	CSC_MAX,
	CSC_USER_DEFINED
} vpp_csc_type;

typedef struct csc_matrix {
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
} vpp_csc_matrix;

typedef struct _vpp_fd_ {
    int dev_fd;/*vpp dev fd*/
    char name[7];/*if u fill in the value of vpp_dev_fd,u must fill in vpp_dev_name as bm-vpp*/
    void* handle; // handle of pcie mode
} vpp_fd_s;

typedef struct vpp_mat_s {
    int num_comp;         /*channel number of data blocks. packet data:1, yuv420sp: 2, yuv420p:3*/
    vpp_img_fmt format;   /*image data format*/
    int is_pa;            /*Judging whether to use ion memory handles or physical addresses directly, now only is 1 */
    vpp_fd_s vpp_fd;      /*vpp handle*/
    int stride;           /*stride of image*/
    int uv_stride;        /*uv stride of image*/
    int fd[4];            /*Handles pointing to ion memory*/
    void* va[4];          /*Virtual address of ion memory, now not used*/
    unsigned long long pa[4];  /*Physical address of device memory*/
    int ion_len[4];       /* Memory Length of Three Channels*/
    int axisX;            /*Image offset in x quadrant*/
    int axisY;            /*Image offset in y quadrant*/
    int cols;             /*Image width*/
    int rows;             /*Image height*/
    void* reserved;       /*reserved field, no need to fill in*/
} vpp_mat;

typedef struct vpp_rect_s {
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
} vpp_rect;

#define STRIDE_ALIGN    (64)
#define VPP_ALIGN(x, mask)  (((x) + ((mask)-1)) & ~((mask)-1))

DECL_EXPORT int vpp_basic(vpp_mat* src, vpp_rect* loca, vpp_mat* dst, int in_img_num, int* crop_num_vec, vpp_csc_type csc_type, vpp_scale_type scale_type, vpp_csc_matrix *matrix);

#if defined(__cplusplus)
}
#endif

#endif
