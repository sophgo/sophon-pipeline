
#ifndef __BM_JPU_LIB_H__
#define __BM_JPU_LIB_H__

#define API_VERSION                 200

#define DC_TABLE_INDEX0             0
#define AC_TABLE_INDEX0             1
#define DC_TABLE_INDEX1             2
#define AC_TABLE_INDEX1             3


//------------------------------------------------------------------------------
// common struct and definition
//------------------------------------------------------------------------------
#if !defined DECL_EXPORT
#ifdef _WIN32
    #define DECL_EXPORT __declspec(dllexport)
#else
    #define DECL_EXPORT
#endif
#endif


typedef enum {
    ENABLE_JPG_ROTATION,
    DISABLE_JPG_ROTATION,
    ENABLE_JPG_MIRRORING,
    DISABLE_JPG_MIRRORING,
    SET_JPG_MIRROR_DIRECTION,
    SET_JPG_ROTATION_ANGLE,
    SET_JPG_ROTATOR_OUTPUT,
    SET_JPG_ROTATOR_STRIDE,
    SET_JPG_SCALE_HOR,
    SET_JPG_SCALE_VER,
    SET_JPG_USE_PARTIAL_MODE,
    SET_JPG_PARTIAL_FRAME_NUM,
    SET_JPG_PARTIAL_LINE_NUM,
    SET_JPG_ENCODE_NEXT_LINE,
    SET_JPG_USE_STUFFING_BYTE_FF,
    ENC_JPG_GET_HEADER,
    ENABLE_LOGGING,
    DISABLE_LOGGING,
    JPG_CMD_END
} JpgCommand;

typedef enum {
    JPG_RET_SUCCESS,
    JPG_RET_FAILURE,
    JPG_RET_BIT_EMPTY,
    JPG_RET_EOS,
    JPG_RET_INVALID_HANDLE,
    JPG_RET_INVALID_PARAM,
    JPG_RET_INVALID_COMMAND,
    JPG_RET_ROTATOR_OUTPUT_NOT_SET,
    JPG_RET_ROTATOR_STRIDE_NOT_SET,
    JPG_RET_FRAME_NOT_COMPLETE,
    JPG_RET_INVALID_FRAME_BUFFER,
    JPG_RET_INSUFFICIENT_FRAME_BUFFERS,
    JPG_RET_INVALID_STRIDE,
    JPG_RET_WRONG_CALL_SEQUENCE,
    JPG_RET_CALLED_BEFORE,
    JPG_RET_NOT_INITIALIZED
} JpgRet;

typedef enum {
    MIRDIR_NONE,
    MIRDIR_VER,
    MIRDIR_HOR,
    MIRDIR_HOR_VER
} JpgMirrorDirection;

typedef enum {
    FORMAT_420 = 0,
    FORMAT_422 = 1,
    FORMAT_224 = 2,
    FORMAT_444 = 3,
    FORMAT_400 = 4
} FrameFormat;

typedef enum {
    CBCR_ORDER_NORMAL,
    CBCR_ORDER_REVERSED
} CbCrOrder;


/*  Cb/Cr InterLeave */
typedef enum {
    CBCR_SEPARATED = 0,
    CBCR_INTERLEAVE,
    CRCB_INTERLEAVE
} CbCrInterLeave;

/*  Packed Output Format */
typedef enum {
    PACKED_FORMAT_NONE,
    PACKED_FORMAT_422_YUYV,
    PACKED_FORMAT_422_UYVY,
    PACKED_FORMAT_422_YVYU,
    PACKED_FORMAT_422_VYUY,
    PACKED_FORMAT_444,
    PACKED_FORMAT_444_RGB
} PackedOutputFormat;

typedef enum {
    INT_JPU_DONE = 0,
    INT_JPU_ERROR = 1,
    INT_JPU_BIT_BUF_EMPTY = 2,
    INT_JPU_BIT_BUF_FULL = 2,
    INT_JPU_PARIAL_OVERFLOW = 3,
    INT_JPU_PARIAL_BUF0_EMPTY = 4,
    INT_JPU_PARIAL_BUF1_EMPTY,
    INT_JPU_PARIAL_BUF2_EMPTY,
    INT_JPU_PARIAL_BUF3_EMPTY,
    INT_JPU_BIT_BUF_STOP
} InterruptJpu;

typedef enum {
    JPG_TBL_NORMAL,
    JPG_TBL_MERGE
} JpgTableMode;

typedef enum {
    ENC_HEADER_MODE_NORMAL,
    ENC_HEADER_MODE_SOS_ONLY
} JpgEncHeaderMode;

typedef void*    EncHandle;
typedef void*    DecHandle;
typedef uint32_t CodecCommand;
typedef uint64_t PhysicalAddress;

typedef struct {
    PhysicalAddress bufY;
    PhysicalAddress bufCb;
    PhysicalAddress bufCr;
    int strideY;
    int strideC;
    int myIndex;
} FrameBuffer;

/* Structures for decoder */
typedef struct {
    PhysicalAddress bitstreamBuffer;
    int bitstreamBufferSize;
    int streamEndian;
    int frameEndian;
    CbCrInterLeave chromaInterleave;
    int thumbNailEn;
    PackedOutputFormat packedFormat;

    int roiEnable;
    int roiOffsetX;
    int roiOffsetY;
    int roiWidth;
    int roiHeight;
    int deviceIndex;

    int rotationEnable;
    int mirrorEnable;
    int mirrorDirection;
    int rotationAngle;
} DecOpenParam;

typedef struct {
    int picWidth;
    int picHeight;
    int minFrameBufferCount;
    int sourceFormat;
    int ecsPtr;
    int roiFrameWidth;
    int roiFrameHeight;
    int roiFrameOffsetX;
    int roiFrameOffsetY;
    int roiMCUSize;
    int colorComponents;
} DecInitialInfo;

typedef struct {
    int scaleDownRatioWidth;
    int scaleDownRatioHeight;
} DecParam;


typedef struct {
    int indexFrameDisplay;
    int numOfErrMBs;
    int decodingSuccess; /* 1: done; 0: error */
    int decPicHeight;
    int decPicWidth;
    int consumedByte;
    int bytePosFrameStart;
    int ecsPtr;
} DecOutputInfo;


/* Structures for encoder */
typedef struct {
    PhysicalAddress bitstreamBuffer;
    uint32_t        bitstreamBufferSize;

    int picWidth;
    int picHeight;

    int streamEndian;
    int frameEndian;
    CbCrInterLeave chromaInterleave;
    PackedOutputFormat packedFormat;
    int deviceIndex;
    int rgbPacked;

    int sourceFormat;
    int restartInterval;

    uint8_t huffVal[4][162];
    uint8_t huffBits[4][256];
    uint8_t qMatTab[4][64];
    uint8_t cInfoTab[4][6];

    int rotationEnable;
    int mirrorEnable;
    int mirrorDirection;
    int rotationAngle;
} EncOpenParam;

typedef struct {
    int minFrameBufferCount;
    int colorComponents;
} EncInitialInfo;

typedef struct {
    FrameBuffer* sourceFrame;
} EncParam;

typedef struct {
    PhysicalAddress bitstreamBuffer;
    uint32_t bitstreamSize;
} EncOutputInfo;

typedef struct {
    int headerMode;
    int quantMode;
    int huffMode;
    int rgbPackd;
} EncParamSet;

#ifdef  _WIN32
DECL_EXPORT void* g_vpp_fd[64];
#else
DECL_EXPORT extern int  g_vpp_fd[64];
#endif
/*****************************************
 *            Functions
 *****************************************/

DECL_EXPORT int  jpu_Init(int32_t device_index);
DECL_EXPORT void jpu_UnInit(int32_t device_index);
DECL_EXPORT int  jpu_GetVersionInfo(uint32_t * verinfo);

DECL_EXPORT int jpu_IsBusy(void);
DECL_EXPORT void     jpu_ClrStatus(uint32_t val);
DECL_EXPORT uint32_t jpu_GetStatus();
DECL_EXPORT int jpu_SWReset();
DECL_EXPORT void jpu_SetLoggingThreshold(uint32_t threshold);

DECL_EXPORT int  jpu_Lock(int sleep_us);
DECL_EXPORT void jpu_UnLock();

/* Functions for encoder */
DECL_EXPORT int jpu_EncOpen(EncHandle *, EncOpenParam *);
DECL_EXPORT int jpu_EncClose(EncHandle);
DECL_EXPORT int jpu_EncGetInitialInfo(EncHandle, EncInitialInfo *);

DECL_EXPORT int jpu_EncStartOneFrame(EncHandle handle, EncParam * param);
DECL_EXPORT int jpu_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info);
DECL_EXPORT int jpu_EncGiveCommand(EncHandle handle, CodecCommand cmd, void *parameter);

DECL_EXPORT int jpu_EncEncodeHeader(EncHandle handle, EncParamSet*);
DECL_EXPORT int jpu_EncCompleteStop(EncHandle handle);
DECL_EXPORT int jpu_EncWaitForInt(EncHandle handle, int timeout_in_ms, int timeout_counts);

/* Functions for decoder */
DECL_EXPORT int jpu_DecOpen(DecHandle *, DecOpenParam *);
DECL_EXPORT int jpu_DecClose(DecHandle);
DECL_EXPORT int jpu_DecGetInitialInfo(DecHandle handle, DecInitialInfo * info);

DECL_EXPORT int jpu_DecRegisterFrameBuffer(DecHandle handle,
                               FrameBuffer * bufArray, int num, int stride, void* par0);
DECL_EXPORT int jpu_DecUpdateBitstreamBuffer(DecHandle handle, uint32_t size);

DECL_EXPORT int jpu_DecSetBsPtr(DecHandle handle, uint8_t *data, int data_size);

DECL_EXPORT int jpu_DecStartOneFrame(DecHandle handle, DecParam * param);
DECL_EXPORT int jpu_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info);
DECL_EXPORT int jpu_DecGetRotateInfo(DecHandle handle, int *rotationEnable, int *rotationAngle, int *mirrorEnable, int *mirrorDirection);
DECL_EXPORT int jpu_DecGiveCommand(DecHandle handle, CodecCommand cmd, void *parameter);
DECL_EXPORT int jpu_DecWaitForInt(DecHandle handle, int timeout_in_ms, int timeout_counts);

DECL_EXPORT int jpu_GetDump();
DECL_EXPORT int vpp_Init(int32_t device_index);
#endif /* __BM_JPU_LIB_H__ */

