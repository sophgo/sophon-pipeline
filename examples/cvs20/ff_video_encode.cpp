#include "ff_video_encode.h"
VideoEnc_FFMPEG::VideoEnc_FFMPEG()
{
    ofmt_ctx = NULL;
    picture= NULL;
    input_picture = NULL;
    out_stream = NULL;
    aligned_input = NULL;
    frame_width = 0;
    frame_height = 0;
    frame_idx = 0;
}

VideoEnc_FFMPEG::~VideoEnc_FFMPEG()
{
    printf("#######VideoEnc_FFMPEG exit \n");
}

int VideoEnc_FFMPEG::openEnc(const char* filename, int soc_idx, int codecId, int framerate,
                             int width, int height, int inputformat, int bitrate, int roi_enable)
{
    int ret = 0;
    const AVCodec *encoder;
    AVDictionary *dict = NULL;
    frame_idx = 0;
    frame_width = width;
    frame_height = height;

    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
    if (!ofmt_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
        return AVERROR_UNKNOWN;
    }

    encoder = find_hw_video_encoder(codecId);
    if (!encoder) {
        av_log(NULL, AV_LOG_FATAL, "hardware video encoder not found\n");
        return AVERROR_INVALIDDATA;
    }

    enc_ctx = avcodec_alloc_context3(encoder);
    if (!enc_ctx) {
        av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
        return AVERROR(ENOMEM);
    }
    enc_ctx->codec_id = (AVCodecID)codecId;
    enc_ctx->width  = width;
    enc_ctx->height = height;
    enc_ctx->pix_fmt = (AVPixelFormat)inputformat;
    enc_ctx->bit_rate_tolerance = bitrate;
    enc_ctx->bit_rate = (int64_t)bitrate;
    enc_ctx->gop_size = 32;
    /* video time_base can be set to whatever is handy and supported by encoder */
    // enc_ctx->time_base = (AVRational){1, framerate};   // only for network stream frame rate
    // enc_ctx->framerate = (AVRational){framerate,1};
    enc_ctx->time_base.num = 1;
    enc_ctx->time_base.den = framerate;
    enc_ctx->framerate.num = framerate;
    enc_ctx->framerate.den = 1;
    av_log(NULL, AV_LOG_DEBUG, "enc_ctx->bit_rate = %ld\n", enc_ctx->bit_rate);

    out_stream = avformat_new_stream(ofmt_ctx, encoder);
    out_stream->time_base = enc_ctx->time_base;
    out_stream->avg_frame_rate = enc_ctx->framerate;
    out_stream->r_frame_rate = out_stream->avg_frame_rate;
    av_dict_set_int(&dict, "sophon_idx", soc_idx, 0);
    av_dict_set_int(&dict, "gop_preset", 8, 0);
    /* Use system memory */
    av_dict_set_int(&dict, "is_dma_buffer", 0, 0);
    // av_dict_set_int(&dict, "qp", 25, 0);
    if (roi_enable == 1) {
        av_dict_set_int(&dict, "roi_enable", 1, 0);
    }else{
        av_dict_set_int(&dict, "roi_enable", 0, 0);
    }

    /* Third parameter can be used to pass settings to encoder */
    ret = avcodec_open2(enc_ctx, encoder, &dict);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder ");
        return ret;
    }
    ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream ");
        return ret;
    }
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
            return ret;
        }
    }
    /* init muxer, write output file header */
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
        return ret;
    }
    //av_dump_format(ofmt_ctx, 0, filename, 1);   // only for debug

    picture = av_frame_alloc();
    picture->format = enc_ctx->pix_fmt;
    picture->width = width;
    picture->height = height;

    if (roi_enable == 1) {
        AVFrameSideData *fside = av_frame_new_side_data(picture, AV_FRAME_DATA_BM_ROI_INFO, sizeof(AVBMRoiInfo));
        if (fside == NULL) {
            return -1;
        }
    }

    return 0;
}

/* data is alligned with 32 */
int VideoEnc_FFMPEG::writeFrame(const uint8_t* data, int step, int width, int height)
{
    int ret = 0 ;
    int got_output = 0;
    if (step % STEP_ALIGNMENT != 0) {
        av_log(NULL, AV_LOG_ERROR, "input step must align with STEP_ALIGNMENT\n");
        return -1;
    }
    static unsigned int frame_nums = 0;
    frame_nums++;
    AVFrameSideData *fside = av_frame_get_side_data(picture, AV_FRAME_DATA_BM_ROI_INFO);
    if (fside) {
        AVBMRoiInfo *roiinfo = (AVBMRoiInfo*)fside->data;
        memset(roiinfo, 0, sizeof(AVBMRoiInfo));
        if (enc_ctx->codec_id  == AV_CODEC_ID_H264) {
            if (false) {
            // if (frame_nums >= 20) {
                roiinfo->customRoiMapEnable = 0;
                roiinfo->customModeMapEnable = 0;
            }else{
                roiinfo->customRoiMapEnable = 1;
                roiinfo->customModeMapEnable = 0;
                for (int i = 0;i <(BM_ALIGN16(height) >> 4);i++) {
                    for (int j=0;j < (BM_ALIGN16(width) >> 4);j++) {
                        int pos = i*(BM_ALIGN16(width) >> 4) + j;
                        // test_1
                        if ( (j >= (BM_ALIGN16(width) >> 4)/2) && (i >= (BM_ALIGN16(height) >> 4)/2) ) {
                            roiinfo->field[pos].H264.mb_qp = 10;
                        }else{
                            roiinfo->field[pos].H264.mb_qp = 40;
                        }

                        // test_2
                        // roiinfo->field[pos].H264.mb_qp = frame_nums%51;

                        // test_3
                        // if (i> 10) {
                        //     roiinfo->field[pos].H264.mb_qp = 29;
                        // } else {
                        //     roiinfo->field[pos].H264.mb_qp = 15;
                        // }
                    }
                }
            }
        } else if (enc_ctx->codec_id  == AV_CODEC_ID_H265) {
            roiinfo->customRoiMapEnable    = 1;
            roiinfo->customModeMapEnable   = 0;
            roiinfo->customLambdaMapEnable = 0;
            roiinfo->customCoefDropEnable  = 0;

            for (int i = 0;i <(BM_ALIGN64(height) >> 6);i++) {
                for (int j=0;j < (BM_ALIGN64(width) >> 6);j++) {
                    int pos = i*(BM_ALIGN64(width) >> 6) + j;

                    // test_1
                    if ( (j > (BM_ALIGN64(width) >> 6)/2) && (i > (BM_ALIGN64(height) >> 6)/2) ) {
                        roiinfo->field[pos].HEVC.sub_ctu_qp_0 = 10;
                        roiinfo->field[pos].HEVC.sub_ctu_qp_1 = 10;
                        roiinfo->field[pos].HEVC.sub_ctu_qp_2 = 10;
                        roiinfo->field[pos].HEVC.sub_ctu_qp_3 = 10;
                    } else {
                        roiinfo->field[pos].HEVC.sub_ctu_qp_0 = 40;
                        roiinfo->field[pos].HEVC.sub_ctu_qp_1 = 40;
                        roiinfo->field[pos].HEVC.sub_ctu_qp_2 = 40;
                        roiinfo->field[pos].HEVC.sub_ctu_qp_3 = 40;
                    }

                    // test_2
                    // roiinfo->field[pos].HEVC.sub_ctu_qp_0 = frame_nums%51;
                    // roiinfo->field[pos].HEVC.sub_ctu_qp_1 = frame_nums%51;
                    // roiinfo->field[pos].HEVC.sub_ctu_qp_2 = frame_nums%51;
                    // roiinfo->field[pos].HEVC.sub_ctu_qp_3 = frame_nums%51;

                    // test_3
                    // if ((i == 3) || (i == 4)) {
                    //     roiinfo->field[pos].HEVC.sub_ctu_qp_0 = 30;
                    //     roiinfo->field[pos].HEVC.sub_ctu_qp_1 = 30;
                    //     roiinfo->field[pos].HEVC.sub_ctu_qp_2 = 30;
                    //     roiinfo->field[pos].HEVC.sub_ctu_qp_3 = 30;
                    // }else {
                    //     roiinfo->field[pos].HEVC.sub_ctu_qp_0 = 10;
                    //     roiinfo->field[pos].HEVC.sub_ctu_qp_1 = 10;
                    //     roiinfo->field[pos].HEVC.sub_ctu_qp_2 = 10;
                    //     roiinfo->field[pos].HEVC.sub_ctu_qp_3 = 10;
                    // }

                    roiinfo->field[pos].HEVC.ctu_force_mode = 0;
                    roiinfo->field[pos].HEVC.ctu_coeff_drop = 0;
                    roiinfo->field[pos].HEVC.lambda_sad_0 = 0;
                    roiinfo->field[pos].HEVC.lambda_sad_1 = 0;
                    roiinfo->field[pos].HEVC.lambda_sad_2 = 0;
                    roiinfo->field[pos].HEVC.lambda_sad_3 = 0;
                }
            }
        }
    }

    av_image_fill_arrays(picture->data, picture->linesize, (uint8_t *) data, enc_ctx->pix_fmt, width, height, 1);
    picture->linesize[0] = step;

    picture->pts = frame_idx;
    frame_idx++;

    av_log(NULL, AV_LOG_DEBUG, "Encoding frame\n");

    /* encode filtered frame */
    AVPacket enc_pkt;
    enc_pkt.data = NULL;
    enc_pkt.size = 0;
    av_init_packet(&enc_pkt);
    ret = avcodec_send_frame(enc_ctx, picture);
    // ret = avcodec_encode_video2(enc_ctx, &enc_pkt, picture, &got_output);
    if (ret < 0)
        return ret;
    ret = avcodec_receive_packet(enc_ctx, &enc_pkt);
    if (ret < 0){
        av_log(NULL, AV_LOG_WARNING, "No output from encoder\n");
        return ret;
    }
    // if (got_output == 0) {
    //     av_log(NULL, AV_LOG_WARNING, "No output from encoder\n");
    //     return -1;
    // }

    /* prepare packet for muxing */
    av_log(NULL, AV_LOG_DEBUG, "enc_pkt.pts=%ld, enc_pkt.dts=%ld\n",
           enc_pkt.pts, enc_pkt.dts);
    av_packet_rescale_ts(&enc_pkt, enc_ctx->time_base,out_stream->time_base);
    av_log(NULL, AV_LOG_DEBUG, "rescaled enc_pkt.pts=%ld, enc_pkt.dts=%ld\n",
           enc_pkt.pts,enc_pkt.dts);

    av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");

    /* mux encoded frame */
    ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
    return ret;
}

int  VideoEnc_FFMPEG::flush_encoder()
{
    int ret;
    int got_frame = 0;

    if (!(enc_ctx->codec->capabilities & AV_CODEC_CAP_DELAY))
        return 0;

    while (1) {
        av_log(NULL, AV_LOG_INFO, "Flushing video encoder\n");
        AVPacket enc_pkt;
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);

        // ret = avcodec_encode_video2(enc_ctx, &enc_pkt, NULL, &got_frame);
        ret = avcodec_send_frame(enc_ctx, NULL);
        if (ret < 0)
            return ret;
        ret = avcodec_receive_packet(enc_ctx, &enc_pkt);
        if (ret < 0)
            return ret;

        // if (!got_frame)
        //     break;

        /* prepare packet for muxing */
        av_log(NULL, AV_LOG_DEBUG, "enc_pkt.pts=%ld, enc_pkt.dts=%ld\n",
               enc_pkt.pts,enc_pkt.dts);
        av_packet_rescale_ts(&enc_pkt, enc_ctx->time_base,out_stream->time_base);
        av_log(NULL, AV_LOG_DEBUG, "rescaled enc_pkt.pts=%ld, enc_pkt.dts=%ld\n",
               enc_pkt.pts,enc_pkt.dts);

        /* mux encoded frame */
        av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
        ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }

    return ret;
}

void VideoEnc_FFMPEG::closeEnc()
{
    flush_encoder();
    av_write_trailer(ofmt_ctx);

    av_frame_free(&picture);

    if (input_picture)
        av_free(input_picture);


    avcodec_free_context(&enc_ctx);

    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
}