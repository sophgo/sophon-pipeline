/*
 * Bitmain hardware context
 *
 * Copyright (c) 2020 Solan Shang <shulin.shang@bitmain.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVUTIL_HWCONTEXT_BMCODEC_H
#define AVUTIL_HWCONTEXT_BMCODEC_H
#include "bmlib_runtime.h"
/**
 * BmCodec details.
 *
 * Allocated as AVHWDeviceContext.hwctx
 */
typedef struct AVBmCodecDeviceContext {
    int device_idx;

    /**
     * File descriptor of Sophon device.
     *
     * This is used as the device to create frames on, and may also be
     * used in some derivation and mapping operations.
     *
     * If no device is required, set to -1.
     */
    int fd; // TODO
    /* HANDLE of Sophon device.
     * if no device is required,set to NULL.
     */
    bm_handle_t handle;
    uint8_t* map_vaddr;
} AVBmCodecDeviceContext;

/**
 * This struct is allocated as AVHWFramesContext.hwctx
 */
typedef struct AVBmCodecFramesContext {
    /**
     * 0: internal allocation of decoder
     * 1: av_hwframe_get_buffer()
     */
    int   type;
    void* surfaces;
    int   nb_surfaces;
} AVBmCodecFramesContext;

typedef struct AVBmCodecFrame {
    /**
     * 0: internal allocation of decoder
     * 1: av_hwframe_get_buffer()
     */
    int               type;

    /**
     * 0: uncompressed
     * 1: compressed
     */
    int               maptype;

    /* av_hwframe_get_buffer() */
    void*             buffer;
    uint8_t*          buffer_vaddr;

    uint8_t*          data[4];
    int               linesize[4];

    int               padded;
    int               coded_width;
    int               coded_height;
    bm_handle_t       handle;
    int               reserved[5];
} AVBmCodecFrame;

typedef struct BmCodecFramesContext {
    int shift_width;
    int shift_height;
    int     codec_id;
} BmCodecFramesContext;
#endif /* AVUTIL_HWCONTEXT_BMCODEC_H */

