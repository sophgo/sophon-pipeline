/* bmvpuapi API library for the BitMain Sophon SoC
 *
 * Copyright (C) 2018 Solan Shang
 * Copyright (C) 2015 Carlos Rafael Giani
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef __BMVPUAPI_ENC_H__
#define __BMVPUAPI_ENC_H__

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
#define ATTRIBUTE
#define DECL_EXPORT __declspec(dllexport)
#define DECL_IMPORT __declspec(dllimport)
#else
#define ATTRIBUTE __attribute__((deprecated))
#define DECL_EXPORT
#define DECL_IMPORT
#endif
#ifdef __cplusplus
extern "C" {
#endif

#include "bmvpuapi_common.h"


/**
 * How to use the encoder:
 *
 * Global initialization / shutdown is done by calling bmvpu_enc_load() and
 * bmvpu_enc_unload() respectively. These functions contain a reference counter,
 * so bmvpu_enc_unload() must be called as many times as bmvpu_enc_load() was,
 * or else it will not unload. Do not try to create a encoder before calling
 * bmvpu_enc_load(), as this function loads the VPU firmware. Likewise, the
 * bmvpu_enc_unload() function unloads the firmware. This firmware (un)loading
 * affects the entire process, not just the current thread.
 *
 * Typically, loading/unloading is done in two ways:
 * (1) bmvpu_enc_load() gets called in the startup phase of the process, and
 *     bmvpu_enc_unload() in the shutdown phase.
 * (2) bmvpu_enc_load() gets called every time before a encoder is to be created,
 *     and bmvpu_enc_unload() every time after a encoder was shut down.
 *
 * How to create, use, and shutdown an encoder:
 *  1. Call bmvpu_enc_get_bitstream_buffer_info(), and allocate a DMA buffer
 *     with the given size and alignment. This is the minimum required size.
 *     The buffer can be larger, but must not be smaller than the given size.
 *  2. Fill an instance of BmVpuEncOpenParams with the values specific to the
 *     input data. It is recommended to set default values by calling
 *     bmvpu_enc_set_default_open_params() and afterwards set any explicit valus.
 *  3. Call bmvpu_enc_open(), passing in a pointer to the filled BmVpuEncOpenParams
 *     instance, and the DMA buffer of the bitstream which was allocated in step 1.
 *  4. Call bmvpu_enc_get_initial_info(). The encoder's initial info contains the
 *     minimum number of framebuffers that must be allocated and registered, and the
 *     address alignment that must be used when allocating DMA memory  for these
 *     framebuffers.
 *  5. (Optional) Perform the necessary size and alignment calculations by calling
 *     bmvpu_calc_framebuffer_sizes(). Pass in the width & height of the frames that
 *     shall be encoded.
 *  6. Create an array of at least as many BmVpuFramebuffer instances as specified in
 *     min_num_rec_fb. Each instance must point to a DMA buffer that is big
 *     enough to hold a frame. If step 5 was performed, allocating as many bytes as indicated
 *     by total_size is enough. Make sure the Y,Cb,Cr offsets in each BmVpuFramebuffer instance
 *     are valid. Using the bmvpu_fill_framebuffer_params() convenience function for this is
 *     recommended. Note that these framebuffers are used for temporary internal encoding only,
 *     and will not contain input or output data.
 *  7. Call bmvpu_enc_register_framebuffers() and pass in the BmVpuFramebuffer array
 *     and the number of BmVpuFramebuffer instances allocated in step 6.
 *  8. (Optional) allocate an array of at least as many DMA buffers as specified in
 *     min_num_src_fb for the input frames. If the incoming data is already stored in DMA buffers,
 *     this step can be omitted, since the encoder can then read the data directly.
 *  9. Create an instance of BmVpuRawFrame, set its values to zero.
 * 10. Create an instance of BmVpuEncodedFrame. Set its values to zero.
 * 11. Set the framebuffer pointer of the BmVpuRawFrame's instance from step 9 to refer to the
 *     input DMA buffer (either the one allocated in step 8, or the one containing the input data if
 *     it already comes in DMA memory).
 * 12. Fill an instance of BmVpuEncParams with valid values. It is recommended to first set its
 *     values to zero by using memset(). It is essential to make sure the acquire_output_buffer() and
 *     finish_output_buffer() function pointers are set, as these are used for acquiring buffers
 *     to write encoded output data into.
 * 13. (Optional) If step 8 was performed, and therefore input data does *not* come in DMA memory,
 *     copy the pixels from the raw input frames into the DMA buffer allocated in step 8. Otherwise,
 *     if the raw input frames are already stored in DMA memory, this step can be omitted.
 * 14. Call bmvpu_enc_encode(). Pass the raw frame, the encoded frame, and the encoding param
 *     structures from steps 9, 10, and 12 to it.
 *     This function will encode data, and acquire an output buffer to write the encoded data into
 *     by using the acquire_output_buffer() function pointer set in step 12. Once it is done
 *     encoding, it will call the finish_output_buffer() function from step 12. Any handle created
 *     by acquire_output_buffer() will be copied over to the encoded data frame structure. When
 *     bmvpu_enc_encode() exits, this handle can then be used to further process the output data.
 *     It is guaranteed that once acquire_output_buffer() was called, finish_output_buffer() will
 *     be called, even if an error occurred.
 *     The BM_VPU_ENC_OUTPUT_CODE_ENCODED_FRAME_AVAILABLE output code bit will always be set
 *     unless the function returned a code other than BM_VPU_ENC_RETURN_CODE_OK.
 *     If the BM_VPU_ENC_OUTPUT_CODE_CONTAINS_HEADER bit is set, then header data has been
 *     written in the output memory block allocated in step 10. It is placed right before the
 *     actual encoded frame data. bmvpu_enc_encode() will pass over the combined size of the header
 *     and the encoded frame data to acquire_output_buffer() in this case, ensuring that the output
 *     buffers are big enough.
 * 15. Repeat steps 11 to 14 until there are no more frames to encode or an error occurs.
 * 16. After encoding is finished, close the encoder with bmvpu_enc_close().
 * 17. Deallocate framebuffer memory blocks, the input DMA buffer block, the output memory block,
 *     and the bitstream buffer memory block.
 *
 * The VPU's encoders only support the BM_VPU_COLOR_FORMAT_YUV420 format.
 */


/* Encoder return codes. With the exception of BM_VPU_ENC_RETURN_CODE_OK, these
 * should be considered hard errors, and the encoder should be closed when they
 * are returned. */
typedef enum
{
    /* Operation finished successfully. */
    BM_VPU_ENC_RETURN_CODE_OK = 0,
    /* General return code for when an error occurs. This is used as a catch-all
     * for when the other error return codes do not match the error. */
    BM_VPU_ENC_RETURN_CODE_ERROR,
    /* Input parameters were invalid. */
    BM_VPU_ENC_RETURN_CODE_INVALID_PARAMS,
    /* VPU encoder handle is invalid. This is an internal error, and most likely
     * a bug in the library. Please report such errors. */
    BM_VPU_ENC_RETURN_CODE_INVALID_HANDLE,
    /* Framebuffer information is invalid. Typically happens when the BmVpuFramebuffer
     * structures that get passed to bmvpu_enc_register_framebuffers() contain
     * invalid values. */
    BM_VPU_ENC_RETURN_CODE_INVALID_FRAMEBUFFER,
    /* Registering framebuffers for encoding failed because not enough framebuffers
     * were given to the bmvpu_enc_register_framebuffers() function. */
    BM_VPU_ENC_RETURN_CODE_INSUFFICIENT_FRAMEBUFFERS,
    /* A stride value (for example one of the stride values of a framebuffer) is invalid. */
    BM_VPU_ENC_RETURN_CODE_INVALID_STRIDE,
    /* A function was called at an inappropriate time. */
    BM_VPU_ENC_RETURN_CODE_WRONG_CALL_SEQUENCE,
    /* The operation timed out. */
    BM_VPU_ENC_RETURN_CODE_TIMEOUT,
    /* The encoding end. */
    BM_VPU_ENC_RETURN_CODE_END
} BmVpuEncReturnCodes;


/* Encoder output codes. These can be bitwise OR combined, so check
 * for their presence in the output_codes bitmask returned by
 * bmvpu_enc_encode() by using a bitwise AND. */
typedef enum
{
    /* Input data was used. If this code is not present, then the encoder
     * didn't use it yet, so give it to the encoder again until this
     * code is set or an error is returned. */
    BM_VPU_ENC_OUTPUT_CODE_INPUT_USED                 = (1UL << 0),
    /* A fully encoded frame is now available. The encoded_frame argument
     * passed to bmvpu_enc_encode() contains information about this frame. */
    BM_VPU_ENC_OUTPUT_CODE_ENCODED_FRAME_AVAILABLE    = (1UL << 1),
    /* The data in the encoded frame also contains header information
     * like SPS/PSS for h.264. Headers are always placed at the beginning
     * of the encoded data, and this code is never present if the
     * BM_VPU_ENC_OUTPUT_CODE_ENCODED_FRAME_AVAILABLE isn't set. */
    BM_VPU_ENC_OUTPUT_CODE_CONTAINS_HEADER            = (1UL << 2)
} BmVpuEncOutputCodes;


typedef enum
{
    BM_VPU_ENC_HEADER_DATA_TYPE_VPS_RBSP = 0,
    BM_VPU_ENC_HEADER_DATA_TYPE_SPS_RBSP,
    BM_VPU_ENC_HEADER_DATA_TYPE_PPS_RBSP
} BmVpuEncHeaderDataTypes;

enum {
    BM_LINEAR_FRAME_MAP      = 0, /* Linear frame map type */
    BM_COMPRESSED_FRAME_MAP  = 10 /* Compressed frame map type */
};

/* h.264 parameters for the new encoder instance. */
typedef struct
{
    /* enables 8x8 intra prediction and 8x8 transform.
     * Default value is 1. */
    int enable_transform8x8;
} BmVpuEncH264Params;

/* h.265 parameters for the new encoder instance. */
typedef struct
{
    /* Enable temporal motion vector prediction.
     * Default value is 1. */
    int enable_tmvp;

    /* Enable Wave-front Parralel Processing for linear buffer mode.
     * Default value is 0. */
    int enable_wpp;

    /* If set to 1, SAO is enabled. If set to 0, it is disabled.
     * Default value is 1. */
    int enable_sao;

    /* Enable strong intra smoothing for intra blocks to prevent artifacts
     * in regions with few AC coefficients.
     * Default value is 1.  */
    int enable_strong_intra_smoothing;

    /* Enable transform skip for an intra CU.
     * Default value is 0. */
    int enable_intra_trans_skip;

    /* Enable intra NxN PUs.
     * Default value is 1. */
    int enable_intraNxN;
} BmVpuEncH265Params;


/* Structure used together with bmvpu_enc_open() */
typedef struct
{
    /* Format encoded data to produce. */
    BmVpuCodecFormat codec_format;

    /* Color format to use for incoming frames.
     * Video codec formats only allow for the two formats
     * BM_VPU_COLOR_FORMAT_YUV420 and BM_VPU_COLOR_FORMAT_YUV400 (the second
     * one is supported by using YUV420 and dummy U and V planes internally).
     * See the BmVpuColorFormat documentation for an explanation how
     * the chroma_interleave value can affec the pixel format that is used. */
    BmVpuColorFormat color_format;

    /* Width and height of the incoming frames, in pixels. These
     * do not have to be aligned to any boundaries. */
    uint32_t frame_width;
    uint32_t frame_height;

    /* Time base, given as a rational number. */
    /* numerator/denominator */
    uint32_t timebase_num;
    uint32_t timebase_den;

    /* Frame rate, given as a rational number. */
    /* numerator/denominator */
    uint32_t fps_num;
    uint32_t fps_den;

    /* Bitrate in bps. If this is set to 0, rate control is disabled, and
     * constant quality mode is active instead.
     * Default value is 100000. */
    int64_t bitrate;

    /* Size of the vbv buffer, in bits. This is only used if rate control is active
     * (= the bitrate in BmVpuEncOpenParams is nonzero).
     * 0 means the buffer size constraints are not checked for.
     * Default value is 0. */
    uint64_t vbv_buffer_size;

    /* the quantization parameter for constant quality mode */
    int cqp;

    /* 0 : Custom mode,
     * 1 : recommended encoder parameters (slow encoding speed, highest picture quality)
     * 2 : Boost mode (normal encoding speed, normal picture quality),
     * 3 : Fast mode (high encoding speed, low picture quality) */
    int enc_mode;

    /* The number of merge candidates in RDO(1 or 2).
     *  1: improve encoding performance.
     *  2: improve quality of encoded picture;
     *  Default value is 2. */
    int max_num_merge;

    /* If set to 1, constrained intra prediction is enabled.
     * If set to 0, it is disabled.
     * Default value is 0. */
    int enable_constrained_intra_prediction;

    /* Enable weighted prediction
     * Default value is 1. */
    int enable_wp;

    /* If set to 1, the deblocking filter is disabled.
     * If set to 0, it remains enabled.
     * Default value is 0. */
    int disable_deblocking;
    /* Alpha/Tc offset for the deblocking filter.
     * Default value is 0. */
    int offset_tc;
    /* Beta offset for the deblocking filter.
     * Default value is 0. */
    int offset_beta;
    /* Enable filtering cross slice boundaries in in-loop debelocking.
     * Default value is 0. */
    int enable_cross_slice_boundary;

    /* Enable Noise Reduction
     * Default value is 1.*/
    int enable_nr;

    /* Additional codec format specific parameters. */
    union
    {
        BmVpuEncH264Params h264_params;
        BmVpuEncH265Params h265_params;
    };

    /* If this is 1, then Cb and Cr are interleaved in one shared chroma
     * plane, otherwise they are separated in their own planes.
     * See the BmVpuColorFormat documentation for the consequences of this. */
    int chroma_interleave;

    /* only used for PCIE mode. For SOC mode, this must be 0.
     * Default value is 0. */
    int soc_idx;

    /* A GOP structure preset option
     * 1: all I,          all Intra, gopsize = 1
     * 2: I-P-P,          consecutive P, cyclic gopsize = 1
     * 3: I-B-B-B,        consecutive B, cyclic gopsize = 1
     * 4: I-B-P-B-P,      gopsize = 2
     * 5: I-B-B-B-P,      gopsize = 4
     * 6: I-P-P-P-P,      consecutive P, cyclic gopsize = 4
     * 7: I-B-B-B-B,      consecutive B, cyclic gopsize = 4
     * 8: Random Access, I-B-B-B-B-B-B-B-B, cyclic gopsize = 8
     * Low delay cases are 1, 2, 3, 6, 7.
     * Default value is 5. */
    int gop_preset;

    // TODO
    /* A period of intra picture in GOP size.
     * Default value is 28. */
    int intra_period;

    /* Enable background detection.
     * Default value is 0 */
    int bg_detection;

    /* Enable MB-level/CU-level rate control.
     * Default value is 1 */
    int mb_rc;

    /* maximum delta QP for rate control
     * Default value is 5 */
    int delta_qp;

    /* minimum QP for rate control
     * Default value is 8 */
    int min_qp;

    /* maximum QP for rate control
     * Default value is 51 */
    int max_qp;

    /* roi encoding flag
     * Default value is 0 */
    int roi_enable;
} BmVpuEncOpenParams;

/* Initial encoding information, produced by the encoder. This structure is
 * essential to actually begin encoding, since it contains all of the
 * necessary information to create and register enough framebuffers. */
typedef struct
{
    /* Caller must register at least this many framebuffers for reconstruction */
    uint32_t min_num_rec_fb;

    /* Caller must register at least this many framebuffers for source(GOP) */
    uint32_t min_num_src_fb;

    /* Physical framebuffer addresses must be aligned to this value. */
    uint32_t framebuffer_alignment;

    /* frame buffer size for reconstruction */
    BmVpuFbInfo rec_fb;

    /* frame buffer size for source */
    BmVpuFbInfo src_fb;

} BmVpuEncInitialInfo;


/* Function pointer used during encoding for acquiring output buffers.
 * See bmvpu_enc_encode() for details about the encoding process.
 * context is the value of output_buffer_context specified in
 * BmVpuEncParams. size is the size of the block to acquire, in bytes.
 * acquired_handle is an output value; the function can set this to a
 * handle that corresponds to the acquired buffer. For example, in
 * libav/FFmpeg, this handle could be a pointer to an AVBuffer. In
 * GStreamer, this could be a pointer to a GstBuffer. The value of
 * *acquired_handle will later be copied to the acquired_handle value
 * of BmVpuEncodedFrame.
 * The return value is a pointer to a memory-mapped region of the
 * output buffer, or NULL if acquiring failed.
 * This function is only used by bmvpu_enc_encode(). */
typedef void* (*BmVpuEncAcquireOutputBuffer)(void *context, size_t size, void **acquired_handle);
/* Function pointer used during encoding for notifying that the encoder
 * is done with the output buffer. This is *not* a function for freeing
 * allocated buffers; instead, it makes it possible to release, unmap etc.
 * context is the value of output_buffer_context specified in
 * BmVpuEncParams. acquired_handle equals the value of *acquired_handle in
 * BmVpuEncAcquireOutputBuffer. */
typedef void (*BmVpuEncFinishOutputBuffer)(void *context, void *acquired_handle);


/* Custom map options in H.265/HEVC encoder. */
typedef struct {
    /* Set an average QP of ROI map. */
    int roiAvgQp;

    /* Enable ROI map. */
    int customRoiMapEnable;
    /* Enable custom lambda map. */
    int customLambdaMapEnable;
    /* Force CTU to be encoded with intra or to be skipped.  */
    int customModeMapEnable;
    /* Force all coefficients to be zero after TQ or not for each CTU (to be dropped).*/
    int customCoefDropEnable;

    /**
     * It indicates the start buffer address of custom map.
     * Each custom CTU map takes 8 bytes and holds mode,
     * coefficient drop flag, QPs, and lambdas like the below illustration.
     * image::../figure/wave520_ctumap.svg["Format of custom Map", width=300]
     */
    bm_pa_t addrCustomMap;
} BmCustomMapOpt;


typedef struct
{
    /* If set to 1, the VPU ignores the given source frame, and
     * instead generates a "skipped frame". If such a frame is
     * reconstructed, it is a duplicate of the preceding frame.
     * This skipped frame is encoded as a P frame.
     * 0 disables skipped frame generation. Default value is 0. */
    int skip_frame;

    /* Functions for acquiring and finishing output buffers. See the
     * typedef documentations above for details about how these
     * functions should behave, and the bmvpu_enc_encode()
     * documentation for how they are used. */
    BmVpuEncAcquireOutputBuffer acquire_output_buffer;
    BmVpuEncFinishOutputBuffer  finish_output_buffer;

    /* User supplied value that will be passed to the functions */
    void *output_buffer_context;
    int  customMapOptUsedIndex;
    BmCustomMapOpt* customMapOpt;

    bm_device_mem_t** roi_dma_buffer;
} BmVpuEncParams;

/* BM VPU Encoder structure. */
typedef struct
{
    void* handle;
    bm_handle_t bm_handle;

    int soc_idx; /* The index of Sophon SoC.
                  * For PCIE mode, please refer to the number at /dev/bm-sophonxx.
                  * For SOC mode, set it to zero. */
    int core_idx; /* unified index for vpu encoder cores at all Sophon SoCs */

    BmVpuCodecFormat codec_format;
    BmVpuColorFormat color_format;

    uint32_t frame_width;
    uint32_t frame_height;

    uint32_t fps_n;
    uint32_t fps_d;

    int cbcr_interleave;
    int first_frame;

    int rc_enable;
    /* constant qp when rc is disabled */
    int cqp;

    /* DMA buffer allocator */
    // BmVpuDMABufferAllocator *dmabuffers_allocator; //Deprecated, now use bmlib

    /* DMA buffer for working */
    bm_device_mem_t*   work_dmabuffer;

    /* DMA buffer for bitstream */
    bm_device_mem_t* bs_dmabuffer;

    unsigned long long bs_virt_addr;
    bmvpu_phys_addr_t bs_phys_addr;

    /* DMA buffer for frame data */
    uint32_t      num_framebuffers;
    VpuFrameBuffer*   internal_framebuffers;
    BmVpuFramebuffer* framebuffers;

    /* TODO change all as the parameters of bmvpu_enc_register_framebuffers() */
    /* DMA buffer for colMV */
    bm_device_mem_t*   buffer_mv;

    /* DMA buffer for FBC luma table */
    bm_device_mem_t*   buffer_fbc_y_tbl;

    /* DMA buffer for FBC chroma table */
    bm_device_mem_t*   buffer_fbc_c_tbl;

    /* Sum-sampled DMA buffer for ME */
    bm_device_mem_t*   buffer_sub_sam;

    uint8_t* headers_rbsp;
    size_t   headers_rbsp_size;

    BmVpuEncInitialInfo initial_info;
} BmVpuEncoder;



/* Returns a human-readable description of the error code.
 * Useful for logging. */
DECL_EXPORT char const * bmvpu_enc_error_string(BmVpuEncReturnCodes code);

/* Get the unified core index for VPU encoder at a specified Sophon SoC. */
DECL_EXPORT int bmvpu_enc_get_core_idx(int soc_idx);

/* These two functions load/unload the encoder. Due to an internal reference
 * counter, it is safe to call these functions more than once. However, the
 * number of unload() calls must match the number of load() calls.
 *
 * The encoder must be loaded before doing anything else with it.
 * Similarly, the encoder must not be unloaded before all encoder activities
 * have been finished. This includes opening/decoding encoder instances. */
DECL_EXPORT int bmvpu_enc_load(int soc_idx);
DECL_EXPORT int bmvpu_enc_unload(int soc_idx);

/*
 * If a bm_handle_t on this soc already exists, return it directly,
 * otherwise return NULL. This function should be called after bmvpu_enc_load()
 * is called, otherwise it is possibily that there is not bm_handle_t on that soc.
 */
DECL_EXPORT bm_handle_t bmvpu_enc_get_bmlib_handle(int soc_idx);

/* Called before bmvpu_enc_open(), it returns the alignment and size for the
 * physical memory block necessary for the encoder's bitstream buffer.
 * The user must allocate a DMA buffer of at least this size, and its physical
 * address must be aligned according to the alignment value. */
DECL_EXPORT void bmvpu_enc_get_bitstream_buffer_info(size_t *size, uint32_t *alignment);

/* Set the fields in "open_params" to valid defaults
 * Useful if the caller wants to modify only a few fields (or none at all) */
DECL_EXPORT void bmvpu_enc_set_default_open_params(BmVpuEncOpenParams *open_params, BmVpuCodecFormat codec_format);

/* Opens a new encoder instance.
 * "open_params" and "bs_dmabuffer" must not be NULL. */
DECL_EXPORT int bmvpu_enc_open(BmVpuEncoder **encoder, BmVpuEncOpenParams *open_params,
                   bm_device_mem_t *bs_dmabuffer);

/* Closes a encoder instance.
 * Trying to close the same instance multiple times results in undefined behavior. */
DECL_EXPORT int bmvpu_enc_close(BmVpuEncoder *encoder);

/* Retrieves initial information available after calling bmvpu_enc_open().
 * Internally this also generates stream headers. */
DECL_EXPORT int bmvpu_enc_get_initial_info(BmVpuEncoder *encoder, BmVpuEncInitialInfo *info);

/* Registers the specified array of framebuffers with the encoder.
 * These framebuffers are used for temporary values during encoding.
 * The minimum valid value for "num_framebuffers" is
 * the "min_num_rec_fb" field of BmVpuEncInitialInfo. */
DECL_EXPORT int bmvpu_enc_register_framebuffers(BmVpuEncoder *encoder,
                                    BmVpuFramebuffer *framebuffers,
                                    uint32_t num_framebuffers);

/* Encodes a given raw input frame with the given encoding parameters.
 * encoded_frame is filled with information about the resulting encoded output frame.
 * The encoded frame data itself is stored in a buffer that is allocated
 * by user-supplied functions (which are set as the acquire_output_buffer and
 * finish_output_buffer function pointers in the encoding_params).
 *
 * Encoding internally works as follows:
 * first, the actual encoding operation is performed by the VPU.
 * Next, information about the encoded data is queried, particularly its size in bytes.
 * Once this size is known, acquire_output_buffer() from encoding_params is called.
 * This function must acquire a buffer that can be used to store the encoded data.
 * This buffer must be at least as large as the size of the encoded data
 * (which is given to acquire_output_buffer() as an argument). The return value of acquire_output_buffer()
 * is a pointer to the (potentially memory-mapped) region of the buffer. The encoded frame data is then
 * copied to this buffer, and finish_output_buffer() is called. This function can be used to inform the
 * caller that the encoder is done with this buffer; it now contains encoded data, and will not be modified
 * further. encoded_frame is filled with information about the encoded frame data.
 * If acquiring the buffer fails, acquire_output_buffer() returns a NULL pointer.
 *
 * NOTE: again, finish_output_buffer() is NOT a function to free the buffer; it just signals that the encoder
 * won't touch the memory inside the buffer anymore.
 *
 * acquire_output_buffer() can also pass on a handle to the acquired buffer (for example, in FFmpeg/libav,
 * this handle would be a pointer to an AVBuffer). The handle is called the "acquired_handle".
 * acquire_output_buffer() can return such a handle. This handle is copied to the encoded_frame struct's
 * acquired_handle field. This way, a more intuitive workflow can be used; if for example, acquire_output_buffer()
 * returns an AVBuffer pointer as the handle, this AVBuffer pointer ends up in the encoded_frame. Afterwards,
 * encoded_frame contains all the necessary information to process the encoded frame data.
 *
 * It is guaranteed that once the buffer was acquired, finish_output_buffer() will always be called, even if
 * an error occurs. This prevents potential memory/resource leaks if the finish_output_buffer() call somehow
 * unlocks or releases the buffer for further processing. The acquired_handle is also copied to encoded_frame
 * even if an error occurs, unless the error occurred before the acquire_output_buffer() call, in which case
 * the encoded_frame's acquired_handle field will be set to NULL.
 *
 * The other fields in encoding_params specify additional encoding parameters, which can vary from frame to
 * frame.
 * output_code is a bit mask containing information about the encoding result. The value is a bitwise OR
 * combination of the codes in BmVpuEncOutputCodes.
 *
 * None of the arguments may be NULL. */
DECL_EXPORT int bmvpu_enc_encode(BmVpuEncoder *encoder,
                     BmVpuRawFrame const *raw_frame,
                     BmVpuEncodedFrame *encoded_frame,
                     BmVpuEncParams *encoding_params,
                     uint32_t *output_code);

/**
 * Parse the parameters in string
 *
 * return value:
 *   -1, failed
 *    0, done
 */
DECL_EXPORT int bmvpu_enc_param_parse(BmVpuEncOpenParams *p, const char *name, const char *value);

#ifdef __cplusplus
}
#endif


#endif
