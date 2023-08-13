/*
 * Copyright (c) 2019 BitMain
 *
 * @file
 *  example for video encode from BGR data,
 *  PS: i implement a C++ encoder it will be more easy to understand ffmpeg
 */
#ifndef FF_VIDEO_ENCODE_H
#define FF_VIDEO_ENCODE_HS
#include <iostream>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavformat/avformat.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
}

#define STEP_ALIGNMENT 32
#ifdef WIN32
#define strcasecmp stricmp
#endif

#define BM_ALIGN16(_x)             (((_x)+0x0f)&~0x0f)
#define BM_ALIGN32(_x)             (((_x)+0x1f)&~0x1f)
#define BM_ALIGN64(_x)             (((_x)+0x3f)&~0x3f)


class VideoEnc_FFMPEG
{
public:
    VideoEnc_FFMPEG();
    ~VideoEnc_FFMPEG();

    int  openEnc(const char* filename, int soc_idx, int codecId, int framerate,
                 int width, int height,int inputformat,int bitrate, int roi_enable);
    void closeEnc();
    int  writeFrame(const uint8_t* data, int step, int width, int height);
    int  flush_encoder();
    bool is_opened = false;
private:
    AVFormatContext * ofmt_ctx;
    AVCodecContext  * enc_ctx;
    AVFrame         * picture;
    AVFrame         * input_picture;
    AVStream        * out_stream;
    uint8_t         * aligned_input;

    int               frame_width;
    int               frame_height;

    int               frame_idx;

    const AVCodec* find_hw_video_encoder(int codecId)
    {
        const AVCodec *encoder = NULL;
        switch (codecId)
        {
        case AV_CODEC_ID_H264:
            encoder = avcodec_find_encoder_by_name("h264_bm");
            break;
        case AV_CODEC_ID_H265:
            encoder = avcodec_find_encoder_by_name("h265_bm");
            break;
        default:
            break;
        }
        return encoder;
    }
};



// static void usage(char* app_name);

// int main(int argc, char **argv)
// {
//     int soc_idx = 0;
//     int enc_id = AV_CODEC_ID_H264;
//     int inputformat = AV_PIX_FMT_YUV420P;
//     int framerate = 30;
//     int ret;
//     if (argc < 6 || argc > 11) {
//         usage(argv[0]);
//         return -1;
//     }

//     if (strcasecmp(argv[3], "H265") == 0 ||
//         strcasecmp(argv[3], "HEVC") == 0)
//         enc_id = AV_CODEC_ID_H265;
//     else if (strcasecmp(argv[3], "H264") == 0 ||
//              strcasecmp(argv[3], "AVC") == 0)
//         enc_id = AV_CODEC_ID_H264;
//     else {
//         usage(argv[0]);
//         return -1;
//     }

//     int width  = atoi(argv[4]);
//     int height = atoi(argv[5]);
//     int roi_enable = 0;
//     if (argc >=7) {
//         roi_enable = atoi(argv[6]);
//     }

//     if (argc >= 8) {
//         if (strcasecmp(argv[7], "I420") == 0 ||
//             strcasecmp(argv[7], "YUV") == 0) // deprecated
//             inputformat = AV_PIX_FMT_YUV420P;
//         else if (strcasecmp(argv[7], "NV12") == 0)
//             inputformat = AV_PIX_FMT_NV12;
//         else {
//             usage(argv[0]);
//             return -1;
//         }
//     }

//     int bitrate = framerate*width*height/8;
//     if (enc_id == AV_CODEC_ID_H265)
//         bitrate = bitrate/2;
//     if (argc >= 9) {
//         int temp = atoi(argv[8]);
//         if (temp >10 && temp < 100000)
//             bitrate = temp*1000;
//     }

//     if (argc >= 10) {
//         int temp = atoi(argv[9]);
//         if (temp >10 && temp <= 60)
//             framerate = temp;
//     }

//     if (argc == 11) {
//         soc_idx = atoi(argv[10]);
//         if (soc_idx < 0)
//             soc_idx = 0;
//     }

//     int stride = (width + STEP_ALIGNMENT - 1) & ~(STEP_ALIGNMENT - 1);
//     int aligned_input_size = stride * height*3/2;

//     // TODO
//     uint8_t *aligned_input = (uint8_t*)av_mallocz(aligned_input_size);
//     if (aligned_input==NULL) {
//         av_log(NULL, AV_LOG_ERROR, "av_mallocz failed\n");
//         return -1;
//     }

//     FILE *in_file = fopen(argv[1], "rb");   //Input raw YUV data
//     if (in_file == NULL) {
//         fprintf(stderr, "Failed to open input file\n");
//         usage(argv[0]);
//         return -1;
//     }

//     bool isFileEnd = false;

//     VideoEnc_FFMPEG writer;

//     ret = writer.openEnc(argv[2], soc_idx, enc_id, framerate , width, height, inputformat, bitrate, roi_enable);
//     if (ret !=0 ) {
//         av_log(NULL, AV_LOG_ERROR,"writer.openEnc failed\n");
//         return -1;
//     }

//     while(1) {
//         for (int y = 0; y < height*3/2; y++) {
//             ret = fread(aligned_input + y*stride, 1, width, in_file);
//             if (ret < width) {
//                 if (ferror(in_file))
//                     av_log(NULL, AV_LOG_ERROR, "Failed to read raw data!\n");
//                 else if (feof(in_file))
//                     av_log(NULL, AV_LOG_INFO, "The end of file!\n");
//                 isFileEnd = true;
//                 break;
//             }
//         }

//         if (isFileEnd)
//             break;

//         writer.writeFrame(aligned_input, stride, width, height);
//     }

//     writer.closeEnc();

//     av_free(aligned_input);

//     fclose(in_file);
//     av_log(NULL, AV_LOG_INFO, "encode finish! \n");
//     return 0;
// }

// static void usage(char* app_name)
// {
//     char usage_str[] =
//     "Usage:\n\t%s <input file> <output file>  <encoder> <width> <height> <roi_enable> <input pixel format> <bitrate(kbps)> <frame rate> <sophon device index>\n"
//     "\t encoder             : H264(default), H265.\n"
//     "\t roi_enable          : 0 disable(default), 1 enable roi.\n"
//     "\t input pixel format  : I420(YUV, default), NV12. YUV is deprecated.\n"
//     "\t bitrate : bitrate > 10 and bitrate < 100000\n"
//     "\t framerate : framerate > 10 and framerate <= 60\n"
//     "\t sophon device index : used in PCIE mode. min valuse is 0.\n"
//     "For example:\n"
//     "\t%s <input file> <output file> H264 width height 0 I420 3000 30 2\n"
//     "\t%s <input file> <output file> H264 width height 0 I420 3000 30\n"
//     "\t%s <input file> <output file> H265 width height 0 I420\n"
//     "\t%s <input file> <output file> H265 width height 0 NV12\n"
//     "\t%s <input file> <output file> H265 width height 0\n";

//     av_log(NULL, AV_LOG_ERROR, usage_str,
//            app_name, app_name, app_name, app_name, app_name, app_name);
// }
#endif //FF_VIDEO_ENCODE_H