
#ifndef __BM_VPU_LIB_H__
#define __BM_VPU_LIB_H__

#include "bmvpu_types.h"

/* H.265 Profile */
#define H265_TAG_PROFILE_MAIN         1
#define H265_TAG_PROFILE_MAIN10       2
#define H265_TAG_PROFILE_STILLPICTURE 3

/* H.265 Tier */
#define H265_TIER_MAIN                0
#define H265_TIER_HIGH                1

/* H.264/H.265 Level */
#define H26X_LEVEL(_Major, _Minor)    (_Major*10+_Minor)

/* H.264 Profile */
#define H264_TAG_PROFILE_BASE         1
#define H264_TAG_PROFILE_MAIN         2
#define H264_TAG_PROFILE_HIGH         3
#define H264_TAG_PROFILE_HIGH10       4

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
#define ATTRIBUTE
#define DECL_EXPORT __declspec(dllexport)
#define DECL_IMPORT __declspec(dllimport)
#else
#define ATTRIBUTE __attribute__((deprecated))
#define DECL_EXPORT
#define DECL_IMPORT
#endif
/**
 * Error codes
 */
enum {
    VPU_RET_SUCCESS               =  0,
    VPU_RET_FAILURE,
    VPU_RET_INVALID_HANDLE,
    VPU_RET_INVALID_PARAM,
    VPU_RET_INVALID_COMMAND,
    VPU_RET_ROTATOR_OUTPUT_NOT_SET=  5, /* rotator output buffer was not allocated. */
    VPU_RET_ROTATOR_STRIDE_NOT_SET,     /* rotator stride was not provided. */
    VPU_RET_FRAME_NOT_COMPLETE,         /* frame decoding operation was not completed, the given API function call cannot be allowed.  */
    VPU_RET_INVALID_FRAME_BUFFER,       /* the given source frame buffer pointers were invalid in encoder. */
    VPU_RET_INSUFFICIENT_FRAME_BUFFERS, /* the given numbers of frame buffers were not enough for the operations of the given handle. */
    VPU_RET_INVALID_STRIDE = 10,        /* the given stride was invalid . */
    VPU_RET_WRONG_CALL_SEQUENCE,        /* the current API function call was invalid considering the allowed sequences between API functions. */
    VPU_RET_CALLED_BEFORE,              /* multiple calls of the current API function for a given instance are invalid. */
    VPU_RET_NOT_INITIALIZED,            /* Before calling any API functions, VPU_Init() should be called at the beginning.  */
    VPU_RET_USERDATA_BUF_NOT_SET,       /* no memory allocation for reporting userdata. */
    VPU_RET_MEMORY_ACCESS_VIOLATION=15, /* access violation to the protected memory. */
    VPU_RET_VPU_RESPONSE_TIMEOUT,       /* VPU response time is too long, time out. */
    VPU_RET_INSUFFICIENT_RESOURCE,      /* VPU cannot allocate memory due to lack of memory. */
    VPU_RET_NOT_FOUND_BITCODE_PATH,     /* Invalid BIT_CODE_FILE_PATH or invalid firmware size when calling VPU_InitWithBitcode() function. */
    VPU_RET_UNSUPPORTED_FEATURE,        /* HOST application uses an API option that is not supported in current hardware.  */
    VPU_RET_NOT_FOUND_VPU_DEVICE  = 20, /* HOST application uses the undefined product ID. */
    VPU_RET_CP0_EXCEPTION,              /* coprocessor exception has occurred. */
    VPU_RET_STREAM_BUF_FULL,            /* stream buffer is full in encoder. */
    VPU_RET_ACCESS_VIOLATION_HW,        /* GDI access error has occurred.
                                         * It might come from violation of write protection region or spec-out GDI read/write request. */
    VPU_RET_QUERY_FAILURE,              /* query command was not successful. */
    VPU_RET_QUEUEING_FAILURE      = 25, /* commands cannot be queued. */
    VPU_RET_VPU_STILL_RUNNING,          /* VPU cannot be flushed or closed now, because VPU is running. */
    VPU_RET_REPORT_NOT_READY,           /* report is not ready for Query(GET_RESULT) command. */
    VPU_RET_VLC_BUF_FULL,               /* VLC buffer is full in encoder. */
    VPU_RET_INVALID_SFS_INSTANCE,       /* current instance can't run sub-framesync.
                                         * already an instance was running with sub-frame sync.*/
};

enum {
    VPU_ENC_INTR_STATUS_NONE,        /* Interrupt not asserted yet */
    VPU_ENC_INTR_STATUS_FULL,        /* Need more buffer */
    VPU_ENC_INTR_STATUS_DONE,        /* Interrupt asserted */
    VPU_ENC_INTR_STATUS_LOW_LATENCY,
    VPU_ENC_INTR_STATUS_TIMEOUT,     /* Interrupt not asserted during given time. */
};

enum {
    FB_FMT_ERR = -1,
    FB_FMT_420 =  0,
    FB_FMT_422 =  1,
    FB_FMT_224 =  2,
    FB_FMT_444 =  3,
    FB_FMT_400 =  4,
};

enum {
    VPU_CODEC_AVC  =  0,
    VPU_CODEC_HEVC = 12,
};

enum {
    VPU_PIC_TYPE_I   = 0,
    VPU_PIC_TYPE_P   = 1,
    VPU_PIC_TYPE_B   = 2,
    VPU_PIC_TYPE_IDR = 5
};

/**
 * GOP structure presets.
 */
#define MAX_GOP_NUM  8
enum {
    VPU_GOP_PRESET_IDX_CUSTOM_GOP = 0,    /* User defined GOP structure */
    VPU_GOP_PRESET_IDX_ALL_I      = 1,    /* All Intra, gopsize = 1 */
    VPU_GOP_PRESET_IDX_IPP        = 2,    /* Consecutive P, cyclic gopsize = 1  */
    VPU_GOP_PRESET_IDX_IBBB       = 3,    /* Consecutive B, cyclic gopsize = 1  */
    VPU_GOP_PRESET_IDX_IBPBP      = 4,    /* cyclic gopsize = 2  */
    VPU_GOP_PRESET_IDX_IBBBP      = 5,    /* cyclic gopsize = 4  */
    VPU_GOP_PRESET_IDX_IPPPP      = 6,    /* Consecutive P, cyclic gopsize = 4 */
    VPU_GOP_PRESET_IDX_IBBBB      = 7,    /* Consecutive B, cyclic gopsize = 4 */
    VPU_GOP_PRESET_IDX_RA_IB      = 8,    /* Random Access, cyclic gopsize = 8 */
};

/**
 * Adding a header syntax layer into the encoded bitstream.
 * The headerType, buf are input parameters to VPU.
 * The size is a returned value from VPU after completing requested operation.
 */
typedef struct {
    /* Physical address of the generated stream */
    bm_pa_t buf;

    /* The size of the generated stream in bytes */
    size_t  size;

    /* A type of header
     * (1<<2): encode VPS nal unit explicitly
     * (1<<3): encode SPS nal unit explicitly
     * (1<<4): encode PPS nal unit explicitly */
    int32_t headerType;
} VpuEncHeaderParam;


/**
 * The parameters of Nth picture in a custom GOP
 */
typedef struct {
    int picType;      /* A picture type */
    int pocOffset;    /* A POC */
    int picQp;        /* A quantization parameter */
    int numRefPicL0;  /* The number of reference L0 */
    int refPocL0;     /* A POC of reference L0 */
    int refPocL1;     /* A POC of reference L1 */
    int temporalId;   /* A temporal ID */
} VpuCustomGopPicParam;

/**
 * Custom GOP parameters.
 */
typedef struct {
    int                  customGopSize;          /* The size of custom GOP (0~8) */
    VpuCustomGopPicParam picParam[MAX_GOP_NUM];  /* Picture parameters in custom GOP */
} VpuCustomGopParam;

/**
 * H.264/H.265 encoder parameters.
 */
typedef struct {
    /* A profile indicator
     *   1: Main profile
     *   2: Main10 profile
     *   3: Main still picture profile */
    int       profile;
    /* A level indicator (level * 10) */
    int       level;
    /* A tier indicator
     *   0: Main tier
     *   1: High tier */
    int       tier;

    /* A bit-depth (8bit or 10bit) which VPU internally uses for encoding
     * VPU encodes with internalBitDepth instead of InputBitDepth.
     * For example, if InputBitDepth is 8 and InternalBitDepth is 10,
     * VPU converts the 8-bit source frames into 10-bit ones and then encodes them. */
    int       internalBitDepth;

    /* Enable lossless coding. */
    int       losslessEnable;

    /* Enable constrained intra prediction. */
    int       constIntraPredFlag;

    /* A GOP structure preset option
     * low delay case: 1, 2, 3, 6, 7
     *   0: custom gop,     user defined GOP structure
     *   1: all I,          all Intra, gopsize = 1
     *   2: I-P-P,          consecutive P, cyclic gopsize = 1
     *   3: I-B-B-B,        consecutive B, cyclic gopsize = 1
     *   4: I-B-P-B-P,      gopsize = 2
     *   5: I-B-B-B-P,      gopsize = 4
     *   6: I-P-P-P-P,      consecutive P, cyclic gopsize = 4
     *   7: I-B-B-B-B,      consecutive B, cyclic gopsize = 4
     *   8: Random Access, I-B-B-B-B-B-B-B-B, cyclic gopsize = 8 */
    int       gopPresetIdx;

    /* The type of I picture to be inserted at every intraPeriod
     *   0: Non-IRAP
     *   1: CRA
     *   2: IDR */
    int       decodingRefreshType;
    /* A quantization parameter of intra picture */
    int       intraQP;
    /* A period of intra picture in GOP size */
    int       intraPeriod;

    /* A top/bottom/left/right offset of conformance window */
    int       confWinTop;
    int       confWinBot;
    int       confWinLeft;
    int       confWinRight;

    /* A slice mode for independent slice
     *   0: no multi-slice
     *   1: slice in CTU number */
    int       independSliceMode;
    /* The number of CTU for a slice when independSliceMode is set with 1 */
    int       independSliceModeArg;

    /* A slice mode for dependent slice
     *   0: no multi-slice
     *   1: slice in CTU number
     *   2: slice in number of byte */
    int       dependSliceMode;
    /* The number of CTU or bytes for a slice when dependSliceMode is set with 1 or 2 */
    int       dependSliceModeArg;

    /* An intra refresh mode
     *   0: no intra refresh
     *   1: row
     *   2: column
     *   3: step size in CTU
     *   4: adaptive intra refresh */
    int       intraRefreshMode;
    /* Specify an intra CTU refresh interval.
     * Depending on intraRefreshMode, it can mean one of the followings.
     *   The number of consecutive CTU rows for IntraCtuRefreshMode of 1
     *   The number of consecutive CTU columns for IntraCtuRefreshMode of 2
     *   A step size in CTU for IntraCtuRefreshMode of 3
     *   The number of Intra CTUs to be encoded in a picture for IntraCtuRefreshMode of 4 */
    int       intraRefreshArg;

    /* Use one of the recommended encoder parameter presets.
     *   0: custom setting
     *   1: recommended encoder parameters (slow encoding speed, highest picture quality)
     *   2: boost mode (normal encoding speed, moderate picture quality)
     *   3: fast mode (fast encoding speed, low picture quality) */
    int       useRecommendEncParam;

    /* Enable a scaling list. */
    int       scalingListEnable;
    /* Enable CU(Coding Unit) size to be used in encoding process.
     * Host application can also select multiple CU sizes.
     *   3'b001 : 8x8
     *   3'b010 : 16x16
     *   3'b100 : 32x32 */
    int       cuSizeMode;

    /* Enable temporal motion vector prediction. */
    int       tmvpEnable;

    /* Enable WPP (Wave-front Parallel Processing).
     * WPP is unsupported in ring buffer mode of bitstream buffer. */
    int       wppEnable;

    /* Specify the number of merge candidates in RDO (1 or 2).
     * 2 of maxNumMerge (default) offers better quality of encoded picture,
     * while 1 of maxNumMerge improves encoding performance.  */
    int       maxNumMerge;

    /* Disable in-loop deblocking filtering. */
    int       disableDeblk;
    /* Enable filtering across slice boundaries for in-loop deblocking. */
    int       lfCrossSliceBoundaryEnable;
    /* Set BetaOffsetDiv2/TcOffsetDiv3 for deblocking filter. */
    int       betaOffsetDiv2;
    int       tcOffsetDiv2;

    /* Enable transform skip for an intra CU. */
    int       skipIntraTrans;

    /* Enable SAO (Sample Adaptive Offset). */
    int       saoEnable;

    /* Enable intra NxN PUs. */
    int       intraNxNEnable;

    /* Specify picture bits allocation mode.
     * Only valid when RateControl is enabled and GOP size is larger than 1.
     *   0: More referenced pictures have more bits than less referenced pictures do.
     *   1: All pictures in GOP have similar amount of bits.
     *   2: Each picture in GOP is allocated a portion (fixedBitRatio) of total bit budget. */
    int       bitAllocMode;
    /* A fixed bit ratio (1 ~ 255) for each picture of GOP's bit allocation
     *    N = 0 ~ (MAX_GOP_SIZE - 1)
     *    MAX_GOP_SIZE = 8
     * For instance when MAX_GOP_SIZE is 3, FixedBitRatio0, FixedBitRatio1, and
     * FixedBitRatio2 can be set as 2, 1, and 1 repsectively for the fixed bit ratio 2:1:1.
     * This is only valid when BitAllocMode is 2.  */
    int       fixedBitRatio[MAX_GOP_NUM];

    /* Enable CU level rate control. */
    int       cuLevelRCEnable;

    /* Enable CU QP adjustment for subjective quality enhancement. */
    int       hvsQPEnable;
    /* A QP scaling factor for CU QP adjustment when hvsQpScaleEnable is 1 */
    int       hvsQpScale;

    /* A maximum delta QP for rate control */
    int       maxDeltaQp;

    /* CUSTOM GOP */
    VpuCustomGopParam gopParam;

    /* Enable ROI map.
     * NOTE: It is valid when rate control is on. */
    int       roiEnable;

    /* Calculate frameRate syntax. */
    /* Specify the number of time units of a clock operating at the frequency time_scale Hz. */
    uint32_t  numUnitsInTick;
    /* Specify the number of time units that pass in one second. */
    uint32_t  timeScale;
    /* Specify the number of clock ticks corresponding to a difference
     * of picture order count values equal to 1. */
    uint32_t  numTicksPocDiffOne;

    /* The value of chroma(Cb/Cr) QP offset */
    int       chromaCbQpOffset;
    int       chromaCrQpOffset;

    /* The value of initial QP by HOST application.
     * The value 63 is meaningless. */
    int       initialRcQp;

    /* Enable noise reduction algorithm to Y/Cb/Cr component. */
    uint32_t  nrYEnable;
    uint32_t  nrCbEnable;
    uint32_t  nrCrEnable;

    /* A weight to Y noise level for intra picture (0 ~ 31).
     * nrIntraWeight/4 is multiplied to the noise level that has been estimated.
     * This weight is put for intra frame to be filtered more strongly or more weakly
     * than just with the estimated noise level. */
    uint32_t  nrIntraWeightY;
    /* A weight to Cb/Cr noise level for intra picture (0 ~ 31) */
    uint32_t  nrIntraWeightCb;
    uint32_t  nrIntraWeightCr;

    /* A weight to Y noise level for inter picture (0 ~ 31).
     * nrInterWeight/4 is multiplied to the noise level that has been estimated.
     * This weight is put for inter frame to be filtered more strongly or more weakly
     * than just with the estimated noise level. */
    uint32_t  nrInterWeightY;
    /* A weight to Cb/Cr noise level for inter picture (0 ~ 31) */
    uint32_t  nrInterWeightCb;
    uint32_t  nrInterWeightCr;

    /* Enable noise estimation for noise reduction.
     * When disabled, host carries out noise estimation with nrNoiseSigmaY/Cb/Cr. */
    uint32_t  nrNoiseEstEnable;
    /* Specify Y/Cb/Cr noise standard deviation when nrNoiseEstEnable is 0.  */
    uint32_t  nrNoiseSigmaY;
    uint32_t  nrNoiseSigmaCb;
    uint32_t  nrNoiseSigmaCr;

    /* Enable long-term reference function. */
    uint32_t  useLongTerm;

    /* Enable monochrome encoding mode. */
    uint32_t  monochromeEnable;
    /* Enable strong intra smoothing. */
    uint32_t  strongIntraSmoothEnable;

    /* Enable to use weighted prediction.*/
    uint32_t  weightPredEnable;

    /* Enable background detection. */
    uint32_t  bgDetectEnable;
    /* Specify the threshold of max/mean difference that is used in s2me block.
     * It is valid when background detection is on. */
    uint32_t  bgThrDiff;
    uint32_t  bgThrMeanDiff;
    /* Specify the minimum lambda QP value to be used in the background area. */
    uint32_t  bgLambdaQp;
    /* Specify the difference between the lambda QP value of background and
     * the lambda QP value of foreground. */
    int       bgDeltaQp;

    /* Enable custom lambda table. */
    uint32_t  customLambdaEnable;
    /* Enable custom mode decision. */
    uint32_t  customMDEnable;

    /* Added to the total cost of 4x4/8x8/16x16/32x32 blocks */
    int       pu04DeltaRate;
    int       pu08DeltaRate;
    int       pu16DeltaRate;
    int       pu32DeltaRate;

    /* Added to rate when calculating cost(=distortion + rate) in 4x4 intra
     * prediction mode: Planar/DC/Angular. */
    int       pu04IntraPlanarDeltaRate;
    int       pu04IntraDcDeltaRate;
    int       pu04IntraAngleDeltaRate;

    /* Added to rate when calculating cost(=distortion + rate) in 8x8 intra
     * prediction mode: Planar/DC/Angular. */
    int       pu08IntraPlanarDeltaRate;
    int       pu08IntraDcDeltaRate;
    int       pu08IntraAngleDeltaRate;

    /* Added to rate when calculating cost(=distortion + rate) in 16x16 intra
     * prediction mode: Planar/DC/Angular. */
    int       pu16IntraPlanarDeltaRate;
    int       pu16IntraDcDeltaRate;
    int       pu16IntraAngleDeltaRate;

    /* Added to rate when calculating cost(=distortion + rate) in 32x32 intra
     * prediction mode: Planar/DC/Angular. */
    int       pu32IntraPlanarDeltaRate;
    int       pu32IntraDcDeltaRate;
    int       pu32IntraAngleDeltaRate;

    /* Added to rate when calculating cost for intra/inter/merge CU8x8 */
    int       cu08IntraDeltaRate;
    int       cu08InterDeltaRate;
    int       cu08MergeDeltaRate;

    /* Added to rate when calculating cost for intra/inter/merge CU16x16 */
    int       cu16IntraDeltaRate;
    int       cu16InterDeltaRate;
    int       cu16MergeDeltaRate;

    /* Added to rate when calculating cost for intra/inter/merge CU32x32 */
    int       cu32IntraDeltaRate;
    int       cu32InterDeltaRate;
    int       cu32MergeDeltaRate;

    /* Disable the transform coefficient clearing algorithm for P or B picture.
     * If this is 1, all-zero coefficient block is not evaluated in RDO. */
    int       coefClearDisable;

    /* A minimum/maximum QP of I/P/B picture for rate control */
    int       minQpI;
    int       maxQpI;
    int       minQpP;
    int       maxQpP;
    int       minQpB;
    int       maxQpB;

    /* specify the address of custom lambda map.  */
    bm_pa_t   customLambdaAddr;

    /* specify the address of user scaling list file. */
    bm_pa_t   userScalingListAddr;

    /* H.264 encoder on WAVE */

    /* Skip RDO(rate distortion optimization). */
    int       rdoSkip;
    /* Enable lambda scaling using custom GOP. */
    int       lambdaScalingEnable;
    /* Enable 8x8 intra prediction and 8x8 transform. */
    int       transform8x8Enable;

    /* A slice mode for independent slice
     *   0: no multi-slice
     *   1: slice in MB number */
    int       avcSliceMode;
    /* The number of MB for a slice when avcSliceMode is set with 1 */
    int       avcSliceArg;

    /* An intra refresh mode
     *   0: no intra refresh
     *   1: row
     *   2: column
     *   3: step size in CTU */
    int       intraMbRefreshMode;

    /* Specify an intra MB refresh interval.
     * Depending on intraMbRefreshMode, it can mean one of the followings.
     *   The number of consecutive MB rows for intraMbRefreshMode of 1
     *   The number of consecutive MB columns for intraMbRefreshMode of 2
     *   A step size in MB for intraMbRefreshMode of 3 */
    int       intraMbRefreshArg;
    /* Enable MB-level rate control. */
    int       mbLevelRcEnable;
    /* Select the entropy coding mode used in encoding process.
     *   0: CAVLC
     *   1: CABAC */
    int       entropyCodingMode;

    /* Disable s2me_fme (only for AVC encoder). TODO */
    int       s2fmeDisable;
} VpuEncWaveParam;


typedef struct {
    bm_pa_t pa;
    int     size;
} VpuPaType;

typedef struct {
    VpuPaType MV;
    VpuPaType FbcYTbl;
    VpuPaType FbcCTbl;
    VpuPaType SubSamBuf;
} VpuEncCoreBuffer;


/**
 * Data structure for a new encoder instance.
 */
typedef struct {
    /* For soc mode, sophon device index is 0;
     * For pcie mode, it is the index of Sophon device. */
    int             socIdx;

    /* VPU core index number
     * 0 to (number of VPU core - 1) */
    int             coreIdx;

    /* The start address of working buffer into which encoder works on.
     * This address must be aligned to AXI bus width. */
    bm_pa_t         workBuffer;
    /* The size of the buffer in bytes pointed by workBuffer.
     * This value must be a multiple of 1024.  */
    uint32_t        workBufferSize;

    /* The start address of bitstream buffer into which encoder puts bitstream.
     * This address must be aligned to AXI bus width. */
    bm_pa_t         bitstreamBuffer;
    /* The size of the buffer in bytes pointed by bitstreamBuffer.
     * This value must be a multiple of 1024.  */
    uint32_t        bitstreamBufferSize;

    VpuPaType       MV;
    VpuPaType       FbcYTbl;
    VpuPaType       FbcCTbl;
    VpuPaType       SubSamBuf;

    /* The standard type of bitstream in encoder operation.
     *   0: CODEC_AVC;
     *   1: CODEC_HEVC */
    int             bitstreamFormat;

    /* The width/height of a picture to be encoded in unit of sample. */
    int             picWidth;
    int             picHeight;

    /* The 16 LSB bits, [15:0], is a numerator, means clock ticks per second.
     * The 16 MSB bits, [31:16], is a denominator for calculating frame rate.
     * The denominator is clock ticks between frames minus 1.
     *
     * So the frame rate can be defined by (numerator/(denominator + 1)),
     * which equals to (frameRateInfo & 0xffff) /((frameRateInfo >> 16) + 1).
     *
     * For example, the value 30 of frameRateInfo represents 30 frames/sec, and the
     * value 0x3e87530 represents 29.97 frames/sec.  */
    int             frameRateInfo;

    /* VBV buffer size in msec
     * Specifies the size of the VBV buffer in msec (10 ~ 3000).
     * For example, 3000 should be set for 3 seconds.
     * This value is valid when rcEnable is 1.
     * VBV buffer size in bits is bitRate * vbvBufferSize / 1000.
     * This value is ignored if rate control is disabled. */
    int             vbvBufferSize;

    /* Target bit rate in bps
     *   0: no rate control. */
    int64_t         bitRate;

    /* Enable rate control
     *   0: rate control is off.
     *   1: rate control is on. */
    int             rcEnable;

    /* 0 : Cb data are written in Cb frame memory and Cr data are written in Cr frame memory.
     *     (chroma separate mode)
     * 1 : Cb and Cr data are written in the same chroma memory.
     *     (chroma interleave mode) */
    int             cbcrInterleave;
    /* CbCr order in planar mode (YV12 format)
     * 0 : Cb data are written first and then Cr written in their separate plane.
     * 1 : Cr data are written first and then Cb written in their separate plane. */
    int             cbcrOrder;
    /* CbCr order in interleave mode
     * 0 : CbCr data is interleaved in chroma source frame memory. (NV12)
     * 1 : CrCb data is interleaved in chroma source frame memory. (NV21) */
    int             nv21;

    /* 0 : Disable
     * 1 : Enable
     * This flag is used to encode frame-based streaming video with line buffer.
     * If this field is set, VPU sends a buffer full interrupt when line buffer is full
     * and waits until the interrupt is cleared.
     * HOST should read the bitstream in line buffer and clear the interrupt.
     * If this field is not set, VPU does not send a buffer full interrupt even if
     * line buffer is full. */
    int             lineBufIntEn; // TODO

    /* 0: NOT_PACKED
     * 1: PACKED_YUYV
     * 2: PACKED_YVYU
     * 3: PACKED_UYVY
     * 4: PACKED_VYUY */
    int             packedFormat;
    int             srcFormat;     /* A color format of source image: FB_FMT_XXX. */
    int             outputFormat;  /* A color format of output image: FB_FMT_XXX. */

    /* Calculate PTS instead of passing through the pts of input source */
    int             enablePTS;

    /* low latency mode setting(2bits).
     * bit[1]: low latency interrupt enable,
     * bit[0]: fast bitstream-packing enable. */
    int             lowLatencyMode;

    /* enable VPU to get input source frames that are encoded by CFrame50.  */
    int             cframe50Enable;
    /* set whether source frames are lossless encoded or not by CFrame50.  */
    int             cframe50LosslessEnable;
    /* set the target bit of each luma 4x4 block. */
    int             cframe50Tx16Y;
    /* set the target bit of each chroma 4x4 block. */
    int             cframe50Tx16C;
    /* enable 422 to 420 conversion (H.264 encoder in WAVE5 only) */
    int             cframe50_422;

    /* 1: FBC data of non-reference picture are written into framebuffer. */
    int             enableNonRefFbcWrite;

    /* bit[0]: USE_RDO_INTERNAL_BUF
     * bit[1]: USE_LF_INTERNAL_BUF */
    int32_t         secondaryAXI;

    VpuEncWaveParam waveParam;
} VpuEncOpenParam;

/**
 * The number of source frame buffer and reconstructed frame buffer required
 * for running an encoder instance.
 * This is returned after calling vpu_EncSetSeqInfo().
 */
typedef struct {
    int minFrameBufferCount; /* Minimum number of frame buffer */
    int minSrcFrameCount;    /* Minimum number of source buffer */
    int maxLatencyPictures;  /* Maximum number of picture latency */

    int seqInitErrReason;    /* Error information */
    int warnInfo;            /* Warn information */

    int reconFbNum; /* appended */
    int srcFbNum;   /* appended */
} VpuEncInitialInfo;


/**
 * Representing frame buffer information such as pointer of each YUV
 * component, endian, map type, etc.
 *
 * All of the 3 component addresses must be aligned to AXI bus width.
 * HOST application must allocate external SDRAM spaces for those components by using this data
 * structure. For example, YCbCr 4:2:0, one pixel value
 * of a component occupies one byte, so the frame data sizes of Cb and Cr buffer are 1/4 of Y buffer size.
 *
 * In case of CbCr interleave mode, Cb and Cr frame data are written to memory area started from bufCb address.
 */
typedef struct {
    /* The base address for Y component in the physical address space when linear map is used.
     * It is also compressed Y buffer. */
    bm_pa_t bufY;
    /* The base address for Cb component in the physical address space when linear map is used.
     * It is also compressed CbCr buffer. */
    bm_pa_t bufCb;
    /* The base address for Cr component in the physical address space when linear map is used. */
    bm_pa_t bufCr;

    /* Specify a chroma interleave mode of frame buffer.
     *   0: Cb data are written in Cb frame memory and Cr data are written in Cr frame memory.
     *      (chroma separate mode)
     *   1: Cb and Cr data are written in the same chroma memory.
     *      (chroma interleave mode) */
    int cbcrInterleave;

    /* Specify the way chroma data is interleaved in the frame buffer, bufCb or bufCbBot.
     *   0: CbCr data is interleaved in chroma memory (NV12).
     *   1: CrCb data is interleaved in chroma memory (NV21). */
    int nv21;

    /* Specify endianess of frame buffer.
     *   0 : little endian format
     *   1 : big endian format
     *   2 : 32 bit little endian format
     *   3 : 32 bit big endian format
     *   16 ~ 31 : 128 bit endian format
     * NOTE: For setting specific values of 128 bit endiness, please refer to the 'WAVE Datasheet'. */
    int endian;

    /* A frame buffer index to identify each frame buffer that is processed by VPU. */
    int myIndex;

    /* A map type for GDI inferface or FBC (Frame Buffer Compression). */
    int mapType;

    int stride;            /* A horizontal stride for given frame buffer */
    int width;             /* A width for given frame buffer */
    int height;            /* A height for given frame buffer */
    int size;              /* A size for given frame buffer */

    int lumaBitDepth;      /* Bit depth for luma component */
    int chromaBitDepth;    /* Bit depth for chroma component  */

    /* A YUV format of frame buffer: FB_FMT_XXX */
    int format;

    /* A sequence number that the frame belongs to.
     * It increases by 1 every time a sequence changes in decoder.  */
    int sequenceNo;

    /* 1: VPU updates API-internal framebuffer information
     * when any of the information is changed. */
    int updateFbInfo;
} VpuFrameBuffer;

/**
 * Data structure for configuring picture encode operation.
 * The variables can change every time one picture is encoded.
 */
typedef struct {
    /* The frame buffer containing source image to be encoded. */
    VpuFrameBuffer* sourceFrame;
    /* A offset table buffer address for Cframe50  */
    VpuFrameBuffer* OffsetTblBuffer;

    /* 0: the encoder encodes a picture as normal.
     * 1: the encoder ignores sourceFrame and generates a skipped picture.
     *    In this case, the reconstructed image at decoder side is a duplication
     *    of the previous picture.
     * The skipped picture is encoded as P-type regardless of the GOP size. */
    int       skipPicture;

    /* A source frame buffer index */
    int       srcIdx;

    /* A flag indicating that there is no more source frame buffer to encode */
    int       srcEndFlag;


    /* The start address of picture stream buffer for encoded output
     * under line-buffer mode.
     *
     * In buffer-reset mode, HOST might use multiple picture stream buffers
     * for the best performance.
     * By using this variable, HOST application could re-register
     * the start position of the picture stream while issuing a picture
     * encoding operation.
     *
     * In packet-based streaming with ring-buffer, this variable is ignored. */
    bm_pa_t   picStreamBufferAddr;
    /* The byte size of picture stream buffer.
     * In line-buffer mode, this value should be big enough for storing multiple
     * picture streams with average size. because encoder output could be
     * corrupted if this size is smaller than any picture encoded output.
     * In packet-based streaming with ring-buffer, this variable is ignored. */
    int       picStreamBufferSize;


    /* Use a force picture QP */
    /* A flag to use a force picture QP */
    int       forcePicQpEnable;
    /* A force picture QP for I/P/B picture.
     * It is valid when forcePicQpEnable is 1. */
    int       forcePicQpI;
    int       forcePicQpP;
    int       forcePicQpB;

    /* A flag to use a force picture type */
    int       forcePicTypeEnable;
    /* A force picture type (I, P, B, IDR, CRA).
     * It is valid when forcePicTypeEnable is 1. */
    int       forcePicType;

    /* A flag for the current picture to be used as a longterm
     * reference picture later when other picture's encoding */
    uint32_t  useCurSrcAsLongtermPic;

    /* A flag to use a longterm reference picture in DPB when encoding
     * the current picture */
    uint32_t  useLongtermRef;

    /* Forces all coefficients to be zero after TQ . */
    uint32_t  forceAllCtuCoefDropEnable;

    /* NAL unit coding options */
    struct {
        /* Whether HOST application encodes a header implicitly or not.
         * If this value is 1, encodeVPS, encodeSPS, and encodePPS are ignored. */
        int implicitHeaderEncode;

        int encodeVCL;      /* A flag to encode VCL nal unit explicitly */

        int encodeVPS;      /* A flag to encode VPS nal unit explicitly */
        int encodeSPS;      /* A flag to encode SPS nal unit explicitly */
        int encodePPS;      /* A flag to encode PPS nal unit explicitly */

        int encodeAUD;      /* A flag to encode AUD nal unit explicitly */
        int encodeEOS;      /* A flag to encode EOS nal unit explicitly.
                             * This should be set when to encode the last
                             * source picture of sequence. */
        int encodeEOB;      /* A flag to encode EOB nal unit explicitly.
                             * This should be set when to encode the last
                             * source picture of sequence. */
        int encodeFiller;   /* A flag to encode Filler nal unit explicitly */
    } codeOption;

    /* Custom map options in H.265/HEVC encoder. */
    struct {
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
    } customMapOpt;

    /* For weighted prediction */
    struct {
        /* Pixel variance of Y/Cb/Cr component */
        uint32_t SigmaY;
        uint32_t SigmaCb;
        uint32_t SigmaCr;

        /* Pixel mean of Y/Cb/Cr component */
        uint32_t MeanY;
        uint32_t MeanCb;
        uint32_t MeanCr;
    } wpPix;

    /* PTS/DTS/Context of input source */
    void*     context;
    uint64_t  pts;
    uint64_t  dts;
} VpuEncParam;

/**
 * Reporting encoder information.
 */
typedef struct {
    /* 0 : reporting disable
     * 1 : reporting enable */
    int     enable;

    /* picture type reporting in MVInfo and Sliceinfo. */
    int     type;

    /* each reporting data size (MBinfo, MVinfo, Sliceinfo). */
    int     size;

    /* reporting buffer into which encoder puts data. */
    bm_pa_t addr;
} VpuEncReportInfo;

/**
 * Reporting the results of picture encoding operations.
 */
typedef struct {
    /* The physical address of the starting point of newly encoded picture stream.
     * If dynamic buffer allocation is enabled in line-buffer mode, this value is
     * identical with the specified picture stream buffer address by HOST. */
    bm_pa_t   bitstreamBuffer;
    /* The byte size of encoded bitstream */
    uint32_t  bitstreamSize;

    /* A read pointer in bitstream buffer,
     * which is where HOST has read encoded bitstream from the buffer */
    int       rdPtr;
    /* A write pointer in bitstream buffer,
     * which is where VPU has written encoded bitstream into the buffer */
    int       wrPtr;

    int       picType;            /* 0: I, 1: P, 2: B, 5: IDR */

    /* The number of slices of the currently being encoded Picture */
    int       numOfSlices;

    /* A reconstructed frame index.
     * The reconstructed frame can be used for reference of future frame.
     *
     * >=0 : encoded picture buffer index
     *  -1 : encoding end
     *  -2 : encoding delay
     *  -3 : header encoded only
     *  -4 : change of encoding parameter */
    int       reconFrameIndex;
    /* A reconstructed frame address and information. */
    VpuFrameBuffer reconFrame;

    /* A flag which represents whether the current encoding has been skipped or not. */
    int       picSkipped;

    int       numOfIntra;      /* The number of intra coded block */
    int       numOfMerge;      /* The number of merge block in 8x8 */
    int       numOfSkipBlock;  /* The number of skip block in 8x8 */

    int       avgCtuQp;        /* The average value of CTU QPs */

    int       encPicByte;      /* The number of encoded picture bytes */

    int       encGopPicIdx;    /* The GOP index of the currently encoded picture */
    int       encPicPoc;       /* The POC(Picture Order Count) value of the current picture */
    int       encSrcIdx;       /* The source buffer index of the current picture */

    int       encNumNut;       /* The number of nal_unit_type of the current picture */
    int       encNuts;         /* Encoded NAL unit type */
    int       encNuts1;        /* Encoded NAL unit type */

    int       encPicCnt;       /* Encoded picture number */

    int       errorReason;     /* Encoding error reason */
    int       warnInfo;        /* Encoding warn information */

    /* Report MB data/motion vector/slice Information */
    VpuEncReportInfo mbInfo;
    VpuEncReportInfo mvInfo;
    VpuEncReportInfo sliceInfo;

    /* Report the cycle number of encoding one frame. */
    int       frameCycle;

    /* The info of the encoded picture which is retrieved and managed from VPU API */
    void*     context; // TODO
    uint64_t  pts;
    uint64_t  dts; // TODO

    uint32_t  cyclePerTick;

    uint32_t  encHostCmdTick;
    uint32_t  encPrepareStartTick;
    uint32_t  encPrepareEndTick;
    uint32_t  encProcessingStartTick;
    uint32_t  encProcessingEndTick;
    uint32_t  encEncodeStartTick;
    uint32_t  encEncodeEndTick;

    uint32_t  prepareCycle;
    uint32_t  processing;
    uint32_t  EncodedCycle;

    /* SSD (Sum of Squared Differences) between source Y picture and reconstructed Y picture */
    uint32_t  picDistortionLow;
    uint32_t  picDistortionHigh;

    int       result; /* VPU_RET_xxx */
} VpuEncOutputInfo;

typedef struct {
    int                 socIdx;
    int                 coreIdx;

    /* W_HEVC_ENC  = 0x01
     * W_AVC_ENC   = 0x03
     * W_SVAC_ENC  = 0x21 */
    int                 codecMode;

    int                 loggingEnable;
    int                 rotationEnable;
    int                 mirrorEnable;

    /* 0: No mirroring */
    /* 1: Vertical mirroring */
    /* 2: Horizontal mirroring */
    /* 3: Horizontal and vertical mirroring */
    int                 mirrorDirection;
    int                 rotationAngle;

    /* dma memory: colMV buffer */
    VpuPaType           vbMV;

    /* dma memory: FBC Luma table buffer */
    VpuPaType           vbFbcYTbl;

    /* dma memory: FBC Chroma table buffer */
    VpuPaType           vbFbcCTbl;

    /* dma memory: Sub-sampled buffer for ME */
    VpuPaType           vbSubSamBuf;

    /* Context/PTS/DTS mapped with source frame index */
    struct {
        void*           context;
        uint64_t        pts;
        uint64_t        dts;
        int             idx;
    } inputMap[32];

    void*               priv;
    int                 findFirstIframe;
    int                 fistIFramePTS;
    int                 bframe_delay;
} VpuEncoder;


DECL_EXPORT int  vpu_EncGetUniCoreIdx(int soc_idx);

DECL_EXPORT int  vpu_EncInit(int soc_idx, void *);
DECL_EXPORT int  vpu_EncUnInit(int soc_idx);

DECL_EXPORT int  vpu_EncIsBusy(int soc_idx);
DECL_EXPORT int  vpu_EncSWReset(VpuEncoder* h, int reset_mode);

DECL_EXPORT int  vpu_lock(int soc_idx);
DECL_EXPORT int  vpu_unlock(int soc_idx);

DECL_EXPORT void vpu_SetEncOpenParam(VpuEncOpenParam *p_par,
                         int width, int height,
                         int fps_n, int fps_d,
                         int64_t bit_rate, int cqp);
DECL_EXPORT int vpu_EncOpen(VpuEncoder** h, VpuEncOpenParam* p_par);
DECL_EXPORT int vpu_EncClose(VpuEncoder* h);
DECL_EXPORT int vpu_EncSetSeqInfo(VpuEncoder* h, VpuEncInitialInfo* pinfo);
DECL_EXPORT int vpu_EncStartOneFrame(VpuEncoder* h, VpuEncParam* param);
DECL_EXPORT int vpu_EncWaitForInt(VpuEncoder* h, int timeout_in_ms);
DECL_EXPORT int vpu_EncGetOutputInfo(VpuEncoder* h, VpuEncOutputInfo* info);

DECL_EXPORT int vpu_EncPutVideoHeader(VpuEncoder* h, VpuEncHeaderParam* par);

DECL_EXPORT int vpu_EncCalcCoreBufferSize(VpuEncoder* h, int num_framebuffer);
DECL_EXPORT int vpu_EncRegisterFrameBuffer(VpuEncoder* h, VpuFrameBuffer* bufArray, int num);

DECL_EXPORT int vpu_CalcStride(int mapType, uint32_t width, uint32_t height,
                   int yuv_format, int cbcrInterleave);
DECL_EXPORT int vpu_CalcLumaSize(int mapType, uint32_t stride, uint32_t height,
                     int yuv_format, int cbcrInterleave);
DECL_EXPORT int vpu_CalcChromaSize(int mapType, uint32_t stride, uint32_t height,
                       int yuv_format, int cbcrInterleave);
DECL_EXPORT int vpu_GetFrameBufSize(int mapType, int stride, int height,
                        int yuv_format, int interleave);


#endif /* __BM_VPU_LIB_H__ */

