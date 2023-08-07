#ifndef __VPP_LIB_H__
#define __VPP_LIB_H__

#include "bmvpp.h"
#include <assert.h>


#ifdef PCIE_MODE
#include "bmlib_runtime.h"
#endif

#if !defined DECL_EXPORT
#ifdef _WIN32
    #define DECL_EXPORT __declspec(dllexport)
#else
    #define DECL_EXPORT
#endif
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#define VPP_OK    (0)
#define VPP_ERR    (-1)

/*color space BM1684 supported*/
#define YUV420        0
#define YOnly         1
#define RGB24         2
#define ARGB32        3
#define YUV422        4

struct vpp_cmd_n {
    int src_format;
    int src_stride;

    int src_endian;
    int src_endian_a;
    int src_plannar;

    int src_fd0;
    int src_fd1;
    int src_fd2;
    unsigned long long src_addr0;
    unsigned long long src_addr1;
    unsigned long long src_addr2;
    unsigned short src_axisX;
    unsigned short src_axisY;
    unsigned short src_cropW;
    unsigned short src_cropH;

    int dst_format;
    int dst_stride;

    int dst_endian;
    int dst_endian_a;
    int dst_plannar;

    int dst_fd0;
    int dst_fd1;
    int dst_fd2;
    unsigned long long dst_addr0;
    unsigned long long dst_addr1;
    unsigned long long dst_addr2;
    unsigned short dst_axisX;
    unsigned short dst_axisY;
    unsigned short dst_cropW;
    unsigned short dst_cropH;

    int src_csc_en;
    int hor_filter_sel;
    int ver_filter_sel;
    int scale_x_init;
    int scale_y_init;

    int csc_type;

    int mapcon_enable;
    int src_fd3;
    unsigned long long src_addr3;
    int cols;
    int rows;
    int src_uv_stride;
    int dst_uv_stride;
    vpp_csc_matrix matrix;
};

struct vpp_batch_n {
	int num;
	struct vpp_cmd_n *cmd;
};


struct vpp_batch_stack {
	int num;
	struct vpp_cmd_n  cmd[VPP_CROP_NUM_MAX];
};

typedef struct mem_layout_s{
	unsigned long long length;
	unsigned long long pa;
	void* va;
}mem_layout;

typedef struct vpp_csc_info
{
    int vpp_fd;                  /*vpp handle*/
    int csc_type;                /* csc coefficient type */
    int algorithm;               /* sampling algorithm type */
    int cols;                    /*image width*/
    int rows;                    /*image height*/
    int src_format;              /*the format of src image*/
    int dst_format;              /*the format of dst image*/
    int src_stride;              /*the stride of src image*/
    int dst_stride;              /*the stride of dst image*/
    unsigned long long src_addr0;     /*component 0 physical address of src image  */
    unsigned long long src_addr1;     /*component 1 physical address of src image  */
    unsigned long long src_addr2;     /*component 2 physical address of src image  */
    unsigned long long dst_addr0;     /*component 0 physical address of dst image  */
    unsigned long long dst_addr1;     /*component 1 physical address of dst image  */
    unsigned long long dst_addr2;     /*component 2 physical address of dst image  */
    unsigned long long reserved1;     /*reserved field */
    unsigned long long reserved2;
}vpp_csc_info;

#define VPP_UPDATE_BATCH _IOWR('v', 0x01, unsigned long)
#define VPP_UPDATE_BATCH_VIDEO _IOWR('v', 0x02, unsigned long)
#define VPP_UPDATE_BATCH_SPLIT _IOWR('v', 0x03, unsigned long)
#define VPP_UPDATE_BATCH_NON_CACHE _IOWR('v', 0x04, unsigned long)
#define VPP_UPDATE_BATCH_CROP_TEST _IOWR('v', 0x05, unsigned long)
#define VPP_GET_STATUS _IOWR('v', 0x06, unsigned long)
#define VPP_TOP_RST _IOWR('v', 0x07, unsigned long)
#define VPP_UPDATE_BATCH_VIDEO_FD_PA _IOWR('v', 0x08, unsigned long)
#define VPP_UPDATE_BATCH_FD_PA _IOWR('v', 0x09, unsigned long)

#ifndef _WINGDI_
typedef unsigned short      WORD;
typedef unsigned int        DWORD;
typedef int                 LONG;
typedef unsigned char       BYTE;

#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER {
    WORD          bfType;
    DWORD         bfSize;
    WORD          bfReserved1;
    WORD          bfReserved2;
    DWORD         bfOffBits;
}BITMAPFILEHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagBITMAPINFOHEADER{
    DWORD         biSize;
    LONG          biWidth;
    LONG          biHeight;
    WORD          biPlanes;
    WORD          biBitCount;
    DWORD         biCompression;
    DWORD         biSizeImage;
    LONG          biXPelsPerMeter;
    LONG          biYPelsPerMeter;
    DWORD         biClrUsed;
    DWORD         biClrImportant;
}BITMAPINFOHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct tagRGBQUAD{
    BYTE rgbRed;
    BYTE rgbGreen;
    BYTE rgbBlue;
    BYTE rgbReserved;
}RGBQUAD;
#pragma pack(pop)
#endif

#define VPP_MSG
#define VPP_MASK_ERR     0x1
#define VPP_MASK_WARN    0x2
#define VPP_MASK_INFO    0x4
#define VPP_MASK_DBG     0x8
#define VPP_MASK_TRACE   0x100

extern int vpp_level;

#ifdef VPP_MSG
#define VppErr(msg, ... )   if (vpp_level & VPP_MASK_ERR)   { printf("[ERR] %s = %d, " msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#define VppWarn(msg, ... )  if (vpp_level & VPP_MASK_WARN)  { printf("[WARN] %s = %d, " msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#define VppInfo(msg, ...)   if (vpp_level & VPP_MASK_INFO)  { printf("[INFO] %s = %d, " msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#define VppDbg(msg, ...)    if (vpp_level & VPP_MASK_DBG)   { printf("[DBG] %s = %d, " msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#define VppTrace(msg, ...)  if (vpp_level & VPP_MASK_TRACE) { printf("[TRACE] %s = %d, " msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#else
#define VppErr(msg, ... )   if (vpp_level & VPP_MASK_ERR)   { printf("[ERR] %s = %d, " msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); abort();}
#define VppWarn(msg, ...)
#define VppInfo(msg, ...)
#define VppDbg(msg, ...)
#define VppTrace(msg, ...)
#endif

#define VppPrint            printf
// #define VppAssert(cond)     do { if (!(cond)) {printf("[vppassert]:%s<%d> : %s\n", __FILE__, __LINE__, #cond); abort();}} while (0)
DECL_EXPORT void vpp_init_lib(void);

#ifdef PCIE_MODE
#define DDR_CH        (1)
DECL_EXPORT int vpp_creat_host_and_device_mem(bm_device_mem_t *dev_buffer_src, vpp_mat *mat, int format, int in_w, int in_h);
DECL_EXPORT int vpp_read_file_pcie(vpp_mat *mat, bm_device_mem_t *dev_buffer_src, char *file_name);
#endif

DECL_EXPORT int fbd_matrix(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, char csc_type, int scale_type, struct csc_matrix *matrix);
DECL_EXPORT int vpp_misc_matrix(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, char csc_type, int scale_type, struct csc_matrix *matrix);
DECL_EXPORT int vpp_misc_cmodel(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, char csc_type, int scale_type);
DECL_EXPORT int vpp_misc(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, char csc_type, int scale_type);
DECL_EXPORT int vpp_crop_csc_single_ctype(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, char csc_type);
DECL_EXPORT int vpp_crop_csc_multi_ctype(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, char csc_type);
DECL_EXPORT int vpp_crop_csc_single(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num);
DECL_EXPORT int vpp_crop_csc_multi(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num);
DECL_EXPORT int vpp_resize_crop_single(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num);
DECL_EXPORT int vpp_resize_crop_multi(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num);
DECL_EXPORT int vpp_resize_crop_single_stype(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, int scale_type);
DECL_EXPORT int vpp_resize_crop_multi_stype(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, int scale_type);
DECL_EXPORT int vpp_resize_csc(struct vpp_mat_s* src, struct vpp_mat_s* dst);
DECL_EXPORT int vpp_resize_csc_ctype(struct vpp_mat_s* src, struct vpp_mat_s* dst, char csc_type);
DECL_EXPORT int vpp_resize_csc_stype(struct vpp_mat_s* src, struct vpp_mat_s* dst, int scale_type);
DECL_EXPORT int vpp_resize_csc_ctype_stype(struct vpp_mat_s* src, struct vpp_mat_s* dst, char csc_type, int scale_type);
DECL_EXPORT int vpp_resize(struct vpp_mat_s* src, struct vpp_mat_s* dst);
DECL_EXPORT int vpp_resize_stype(struct vpp_mat_s* src, struct vpp_mat_s* dst, int scale_type);
DECL_EXPORT int vpp_csc(struct vpp_mat_s* src, struct vpp_mat_s* dst);
DECL_EXPORT int vpp_csc_ctype(struct vpp_mat_s* src, struct vpp_mat_s* dst, char csc_type);
DECL_EXPORT int vpp_split(struct vpp_mat_s* src, struct vpp_mat_s* dst);
DECL_EXPORT int vpp_border(struct vpp_mat_s* src, struct vpp_mat_s* dst, int top, int bottom, int left, int right);
DECL_EXPORT int vpp_crop_multi(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num);
DECL_EXPORT int vpp_crop_single(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num);
DECL_EXPORT int vpp_csc_single(vpp_csc_info * vpp_info);

DECL_EXPORT int fbd_csc_resize(struct vpp_mat_s* src, struct vpp_mat_s* dst);
DECL_EXPORT int fbd_csc_resize_ctype(struct vpp_mat_s* src, struct vpp_mat_s* dst, char csc_type);
DECL_EXPORT int fbd_csc_resize_stype(struct vpp_mat_s* src, struct vpp_mat_s* dst, int scale_type);
DECL_EXPORT int fbd_csc_resize_ctype_stype(struct vpp_mat_s* src, struct vpp_mat_s* dst, char csc_type, int scale_type);
DECL_EXPORT int fbd_csc_crop_multi(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num);
DECL_EXPORT int fbd_csc_crop_single(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num);
DECL_EXPORT int fbd_csc_crop_multi_ctype(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, char csc_type);
DECL_EXPORT int fbd_csc_crop_single_ctype(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, char csc_type);
DECL_EXPORT int fbd_csc_crop_multi_resize_ctype(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, char csc_type);
DECL_EXPORT int fbd_csc_crop_multi_resize_stype(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, int scale_type);
DECL_EXPORT int fbd_csc_crop_multi_resize_ctype_stype(struct vpp_mat_s* src, struct vpp_rect_s* loca, struct vpp_mat_s* dst, int crop_num, char csc_type, int scale_type);

DECL_EXPORT int i420tonv12(void * const in ,unsigned int w, unsigned int h, void * const out,unsigned int stride);
#if !defined  _WIN32
#include "vppion.h"
DECL_EXPORT int vpp_read_file(vpp_mat* mat, const ion_dev_fd_s* ion_dev_fd, char* file_name);
DECL_EXPORT void* vpp_ion_malloc(int rows, int stride, ion_para* para);
DECL_EXPORT void* vpp_ion_malloc_len(int len, ion_para* para);
DECL_EXPORT void vpp_ion_free(ion_para* para);
DECL_EXPORT void vpp_ion_free_close_devfd(ion_para* para);
DECL_EXPORT int vpp_creat_ion_mem_fd(vpp_mat* mat, int format, int in_w, int in_h, const ion_dev_fd_s* ion_dev_fd);
#endif

DECL_EXPORT int vpp_write_file(char *file_name, vpp_mat *mat);
DECL_EXPORT int vpp_bmp_bgr888(char *img_name, unsigned char *bgr_data, int cols, int rows, int stride);
DECL_EXPORT int vpp_output_mat_to_yuv420(char *file_name, vpp_mat *mat);
DECL_EXPORT int vpp_bmp_gray(char *img_name, unsigned char *bgr_data, int cols, int rows, int stride);
DECL_EXPORT void vpp_get_status(void);
DECL_EXPORT void vpp_top_rst(void);
DECL_EXPORT int vpp_creat_ion_mem(vpp_mat *mat, int format, int in_w, int in_h);
DECL_EXPORT void vpp_free_ion_mem(vpp_mat *mat);
DECL_EXPORT int output_file(char *file_name, vpp_mat *mat);
#if defined(__cplusplus)
}
#endif

#endif
