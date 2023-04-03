#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include "spdlog/spdlog.h"
#include "ffmpeg/avformat.h"
#include "ffmpeg/avcodec.h"
#include "ffmpeg/avutil.h"
#include "ffmpeg/avfilter.h"
#include "ffmpeg/imgutils.h"
#include "cimg/CImg.h"

using namespace std;
using namespace cimg_library;


/**
 * @brief 初始化日志
 *
 */
void init_log() {
    try {
        auto file_logger = spdlog::basic_logger_mt("file_logger", LOG_FILENAME);
        auto console_logger = spdlog::stdout_color_mt("console_logger");
        spdlog::set_default_logger(spdlog::create_combo_logger("default", {file_logger, console_logger}));
        spdlog::flush_on(spdlog::level::info);
    } catch (const spdlog::spdlog_ex& ex) {
        cerr << "Log init failed: " << ex.what() << endl;
    }
}

/**
 * @brief 视频裁剪
 *
 * @param input_filename 输入视频文件名
 * @param output_filename 输出视频文件名
 * @param start_time 起始时间(秒)
 * @param end_time 终止时间(秒)
 * @return 是否成功
 */
bool video_cut(const string& input_filename, const string& output_filename, double start_time, double end_time) {
    AVFormatContext* input_ctx = nullptr;
    AVFormatContext* output_ctx = nullptr;
    AVPacket pkt = {0};
    int ret = 0;
    try {
        // 打开输入文件
        if ((ret = avformat_open_input(&input_ctx, input_filename.c_str(), nullptr, nullptr)) < 0) {
            spdlog::error("Could not open input file {}", input_filename);
            return false;
        }
        // 查找流信息
        if ((ret = avformat_find_stream_info(input_ctx, nullptr)) < 0) {
            spdlog::error("Could not find stream information {}", input_filename);
            return false;
        }
        // 复制流信息到输出文件
        if ((ret = avformat_alloc_output_context2(&output_ctx, nullptr, nullptr, output_filename.c_str())) < 0) {
            spdlog::error("Could not allocate output context");
            return false;
        }
        for (unsigned int i = 0; i < input_ctx->nb_streams; i++) {
            AVStream* in_stream = input_ctx->streams[i];
            AVStream* out_stream = avformat_new_stream(output_ctx, in_stream->codecpar->codec);
            if (!out_stream) {
                spdlog::error("Failed allocating output stream");
                return false;
            }
            if ((ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar)) < 0) {
                spdlog::error("Failed to copy codec parameters");
                return false;
            }
            out_stream->codecpar->codec_tag = 0;
        }

        // 打开输出文件
        if (!(output_ctx->oformat->flags & AVFMT_NOFILE)) {
            ret = avio_open(&output_ctx->pb, output_filename.c_str(), AVIO_FLAG_WRITE);
            if (ret < 0) {
                spdlog::error("Could not open output file {}", output_filename);
                return false;
            }
        }

        // 写头部信息
        if ((ret = avformat_write_header(output_ctx, nullptr)) < 0) {
            spdlog::error("Error occurred when opening output file {}", output_filename);
            return false;
        }

        // 找到起始时间对应的帧索引
        int64_t start_timestamp = start_time * AV_TIME_BASE;
        int64_t end_timestamp = end_time * AV_TIME_BASE;
        int start_frame_index = -1;
        for (unsigned int i = 0; i < input_ctx->nb_streams; i++) {
            AVStream* stream = input_ctx->streams[i];
            AVRational time_base = stream->time_base;
            int64_t timestamp = stream->start_time;
            int frame_index = 0;
            while ((ret = av_read_frame(input_ctx, &pkt)) >= 0) {
                if (pkt.stream_index == i) {
                    timestamp = pkt.pts * time_base.num / time_base.den;
                    if (timestamp >= start_timestamp) {
                        start_frame_index = frame_index;
                        av_packet_unref(&pkt);
                        break;
                    }
                    frame_index++;
                }
                av_packet_unref(&pkt);
            }
            if (start_frame_index == -1) {
                spdlog::error("Failed to find start frame");
                return false;
            }
            // 将文件指针定位到起始帧
            av_seek_frame(input_ctx, i, start_frame_index, AVSEEK_FLAG_FRAME);
        }
        // 读取起始帧到结束时间的帧
        while ((ret = av_read_frame(input_ctx, &pkt)) >= 0) {
            int64_t timestamp = pkt.pts * input_ctx->streams[pkt.stream_index]->time_base.num / input_ctx->streams[pkt.stream_index]->time_base.den;
            if (timestamp <= end_timestamp) {
                // 写入输出文件
                av_interleaved_write_frame(output_ctx, &pkt);
            } else {
                break;
            }
            av_packet_unref(&pkt);
        }

        // 写尾部信息
        av_write_trailer(output_ctx);

        avformat_close_input(&input_ctx);
        avformat_free_context(input_ctx);
        avio_closep(&output_ctx->pb);
        avformat_free_context(output_ctx);
    } catch (const exception& ex) {
        spdlog::error("Exception occurred during video cut: {}", ex.what());
        return false;
    }
    spdlog::info("Video cut succeeded, output file: {}", output_filename);
    return true;
}

/**
 * @brief 视频合并
 *
 * @param input_filenames 输入视频文件名列表
 * @param output_filename 输出视频文件名
 * @return 是否成功
 */
bool video_merge(const vector<string>& input_filenames, const string& output_filename) {
    AVFormatContext* output_ctx = nullptr;
    AVOutputFormat* output_fmt = nullptr;
    AVStream* output_stream = nullptr;
    int ret = 0;
    try {
        // 打开输出文件
        if ((ret = avformat_alloc_output_context2(&output_ctx, nullptr, nullptr, output_filename.c_str())) < 0) {
            spdlog::error("Could not allocate output context");
            return false;
        }
        output_fmt = output_ctx->oformat;

        // 创建视频流
        output_stream = avformat_new_stream(output_ctx, nullptr);
        if (!output_stream) {
            spdlog::error("Failed allocating output stream");
            return false;
        }

        AVCodec* codec = avcodec_find_encoder(output_fmt->video_codec);
        if (!codec) {
            spdlog::error("Codec not found");
            return false;
        }
        AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
        if (!codec_ctx) {
            spdlog::error("Failed to allocate codec context");
            return false;
        }
        codec_ctx->width = 640;
        codec_ctx->height = 480;
        codec_ctx->bit_rate = 400000;
        codec_ctx->time_base = {1, 25};
        codec_ctx->gop_size = 10;
        codec_ctx->max_b_frames = 1;
        codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
        if (output_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
            codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }
        if ((ret = avcodec_open2(codec_ctx, codec, nullptr)) < 0) {
            spdlog::error("Failed to open encoder: {}", av_err2str(ret));
            return false;
        }
        if ((ret = avcodec_parameters_from_context(output_stream->codecpar, codec_ctx)) < 0) {
            spdlog::error("Failed to copy parameters from codec context: {}", av_err2str(ret));
            return false;
        }
        output_stream->codecpar->codec_tag = 0;

        // 打开输出文件
        if (!(output_fmt->flags & AVFMT_NOFILE)) {
            ret = avio_open(&output_ctx->pb, output_filename.c_str(), AVIO_FLAG_WRITE);
            if (ret < 0) {
                spdlog::error("Could not open output file {}", output_filename);
                return false;
            }
            output_stream->time_base = codec_ctx->time_base;
        }

        // 写头部信息
        if ((ret = avformat_write_header(output_ctx, nullptr)) < 0) {
            spdlog::error("Error occurred when opening output file {}", output_filename);
            return false;
        }

        // 处理输入文件
        for (auto& input_filename : input_filenames) {
            AVFormatContext* input_ctx = nullptr;
            // 打开输入文件
            if ((ret = avformat_open_input(&input_ctx, input_filename.c_str(), nullptr, nullptr)) < 0) {
                spdlog::error("Could not open input file {}", input_filename);
                continue;
            }
            // 查找流信息
            if ((ret = avformat_find_stream_info(input_ctx, nullptr)) < 0) {
                spdlog::error("Could not find stream information {}", input_filename);
                continue;
            }
            // 复制视频信息到输出文件
            for (unsigned int i = 0; i < input_ctx->nb_streams; i++) {
                if (input_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                    AVStream* in_stream = input_ctx->streams[i];
                    AVStream* out_stream = avformat_new_stream(output_ctx, nullptr);
                    if (!out_stream) {
                        spdlog::error("Failed allocating output stream");
                        return false;
                    }
                    if ((ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar)) < 0) {
                        spdlog::error("Failed to copy codec parameters");
                        return false;
                    }
                    out_stream->codecpar->codec_tag = 0;
                    break;
                }
            }
            // 写入视频数据
            AVPacket pkt = {0};
            while ((ret = av_read_frame(input_ctx, &pkt)) >= 0) {
                av_interleaved_write_frame(output_ctx, &pkt);
                av_packet_unref(&pkt);
            }
            avformat_close_input(&input_ctx);
            avformat_free_context(input_ctx);
        }

        // 写尾部信息
        av_write_trailer(output_ctx);

        avio_close(output_ctx->pb);
        avformat_free_context(output_ctx);
    } catch (const exception& ex) {
        spdlog::error("Exception occurred during video merge: {}", ex.what());
        return false;
    }
    spdlog::info("Video merge succeeded, output file: {}", output_filename);
    return true;
}

void compressVideo(const string& inFileName, const string& outFileName, int bitrate) {
    // 输出日志 - 开始处理视频压缩任务
    logger->info("开始处理视频压缩任务: {} -> {} [bitrate={}kbps]", inFileName, outFileName, bitrate);

    // 初始化 FFmpeg
    av_register_all();
    avcodec_register_all();
    avformat_network_init();

    AVFormatContext* pFormatCtx = nullptr;
    if (avformat_open_input(&pFormatCtx, inFileName.c_str(), nullptr, nullptr) != 0) {
        logger->error("无法打开输入文件: {}", inFileName);
        return;
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        logger->error("无法读取文件信息: {}", inFileName);
        avformat_close_input(&pFormatCtx);
        return;
    }

    int videoStreamIndex = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1) {
        logger->error("输入文件没有视频流: {}", inFileName);
        avformat_close_input(&pFormatCtx);
        return;
    }

    AVCodecParameters* pCodecParams = pFormatCtx->streams[videoStreamIndex]->codecpar;
    AVCodec* pCodec = avcodec_find_decoder(pCodecParams->codec_id);
    if (!pCodec) {
        logger->error("无法找到解码器: codec_id={}", pCodecParams->codec_id);
        avformat_close_input(&pFormatCtx);
        return;
    }

    AVCodecContext* pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, pCodecParams);
    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
        logger->error("无法打开解码器: codec_id={}", pCodecParams->codec_id);
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return;
    }

    // 根据指定的比特率设置输出视频的码率
    int outBitrate = bitrate * 1000;
    pCodecCtx->bit_rate = outBitrate;
    pFormatCtx->bit_rate = outBitrate;

    AVOutputFormat* pOutFormat = av_guess_format("mp4", nullptr, nullptr);
    if (!pOutFormat) {
        logger->error("无法找到输出格式");
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return;
    }

    AVCodecID codecId = pCodecParams->codec_id;
    if (codecId == AV_CODEC_ID_NONE) {
        logger->error("未知的编解码器ID");
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return;
    }

    // 创建输出文件
    AVFormatContext* pOutFormatCtx = nullptr;
    if (avformat_alloc_output_context2(&pOutFormatCtx, pOutFormat, nullptr, outFileName.c_str()) < 0) {
        logger->error("无法创建输出文件: {}", outFileName);
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        return;
    }

    AVStream* pOutStream = avformat_new_stream(pOutFormatCtx, nullptr);
    if (!pOutStream) {
        logger->error("无法创建输出流");
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pOutFormatCtx);
        return;
    }

    if (avcodec_parameters_copy(pOutStream->codecpar, pCodecParams) < 0) {
        logger->error("无法复制编解码器参数");
        avcodec_free_context(&pCodecCtx);
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pOutFormatCtx);
        return;
    }

    pOutStream->codecpar->codec_tag = 0;

    // 打开输出文件
    if (!(pOutFormatCtx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&pOutFormatCtx->pb, outFileName.c_str(), AVIO_FLAG_WRITE) < 0) {
            logger->error("无法打开输出文件： {}", outFileName);
            avcodec_free_context(&pCodecCtx);
            avformat_close_input(&pFormatCtx);
            avformat_free_context(pOutFormatCtx);
            return;
        }
    }

    // 写入文件头信息
    avformat_write_header(pOutFormatCtx, nullptr);

    AVFrame* pFrame = av_frame_alloc();
    AVPacket* pPacket = av_packet_alloc();
    struct SwsContext* pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                                pCodecCtx->pix_fmt, pCodecCtx->width,
                                                pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                                SWS_BICUBIC, NULL, NULL, NULL);

    while (av_read_frame(pFormatCtx, pPacket) >= 0) {
        if (pPacket->stream_index == videoStreamIndex) {
            avcodec_send_packet(pCodecCtx, pPacket);
            while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                // 转换像素格式
                uint8_t* dstData[4] = {nullptr};
                int dstStride[4] = {0};
                av_image_alloc(dstData, dstStride, pCodecCtx->width, pCodecCtx->height,
                                AV_PIX_FMT_YUV420P, 1);
                sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0,
                            pCodecCtx->height, dstData, dstStride);

                AVFrame* pOutFrame = av_frame_alloc();
                pOutFrame->width = pCodecCtx->width;
                pOutFrame->height = pCodecCtx->height;
                pOutFrame->format = AV_PIX_FMT_YUV420P;

                av_image_fill_arrays(pOutFrame->data, pOutFrame->linesize, dstData[0],
                                        AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

                // 编码并写入视频帧
                int ret = avcodec_send_frame(pCodecCtx, pOutFrame);
                if (ret == 0) {
                    while (true) {
                        ret = avcodec_receive_packet(pCodecCtx, pPacket);
                        if (ret == 0) {
                            av_write_frame(pOutFormatCtx, pPacket);
                            av_packet_unref(pPacket);
                        }
                        else if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                            break;
                        } else {
                            logger->error("音频帧编码失败: {}", ret);
                        }
                    }
                }
                
                av_frame_free(&pOutFrame);
            }
        }

        av_packet_unref(pPacket);
    }

    sws_freeContext(pSwsCtx);
    av_frame_free(&pFrame);
    av_packet_free(&pPacket);

    // 写入文件尾信息
    av_write_trailer(pOutFormatCtx);

    // 释放资源
    avcodec_free_context(&pCodecCtx);
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pOutFormatCtx);

    // 输出日志 - 处理视频压缩任务完成
    logger->info("处理视频压缩任务完成");
}

void optimizeVideo(const string& inFileName, const string& outFileName) {
    // 输出日志 - 开始优化视频清晰度任务
    logger->info("开始处理视频优化清晰度任务: {} -> {}", inFileName, outFileName);

    CImg<uint8_t> img(inFileName.c_str());
    int width = img.width(), height = img.height();

    AVCodec* pCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    AVCodecContext* pCodecCtx = avcodec_alloc_context3(pCodec);
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->time_base = {1, 25};

    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
        logger->error("无法打开编码器: codec_id={}", pCodec->id);
        avcodec_free_context(&pCodecCtx);
        return;
    }

    AVFormatContext* pOutFormatCtx = nullptr;
    if (avformat_alloc_output_context2(&pOutFormatCtx, nullptr, "mp4", outFileName.c_str()) < 0) {
        logger->error("无法创建输出文件: {}", outFileName);
        avcodec_free_context(&pCodecCtx);
        return;
    }

    AVStream* pOutStream = avformat_new_stream(pOutFormatCtx, pCodec);
    if (!pOutStream) {
        logger->error("无法创建输出流");
        avcodec_free_context(&pCodecCtx);
        avformat_free_context(pOutFormatCtx);
        return;
    }

    if (avcodec_parameters_from_context(pOutStream->codecpar, pCodecCtx) < 0) {
        logger->error("无法设置编解码器参数");
        avcodec_free_context(&pCodecCtx);
        avformat_free_context(pOutFormatCtx);
        return;
    }

    if (!(pOutFormatCtx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&pOutFormatCtx->pb, outFileName.c_str(), AVIO_FLAG_WRITE) < 0) {
            logger->error("无法打开输出文件: {}", outFileName);
            avcodec_free_context(&pCodecCtx);
            avformat_free_context(pOutFormatCtx);
            return;
        }
    }

    if (avformat_write_header(pOutFormatCtx, nullptr) < 0) {
        logger->error("无法写入文件头信息");
        avcodec_free_context(&pCodecCtx);
        avformat_free_context(pOutFormatCtx);
        return;
    }

    uint8_t* buf = img.data();
    int buf_size = width * height * sizeof(uint8_t) * 3;
    AVFrame* pFrame = av_frame_alloc();
    pFrame->width = width;
    pFrame->height = height;
    pFrame->format = AV_PIX_FMT_YUVJ420P;
    av_image_fill_arrays(pFrame->data, pFrame->linesize, buf,
                            AV_PIX_FMT_RGB24, width, height, 1);

    AVPacket* pPacket = av_packet_alloc();
    int ret = avcodec_send_frame(pCodecCtx, pFrame);
    if (ret == 0) {
        while (true) {
            ret = avcodec_receive_packet(pCodecCtx, pPacket);
            if (ret == 0) {
                av_write_frame(pOutFormatCtx, pPacket);
                av_packet_unref(pPacket);
            }
            else if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else {
                logger->error("视频帧编码失败: {}", ret);
            }
        }
    }

    av_packet_free(&pPacket);
    av_frame_free(&pFrame);

    av_write_trailer(pOutFormatCtx);

    avcodec_free_context(&pCodecCtx);
    avformat_free_context(pOutFormatCtx);

    // 输出日志 - 处理视频优化清晰度任务完成
    logger->info("处理视频优化清晰度任务完成");
}

void modifyVideoInfo(string videoFile, string title, string author) {
    AVFormatContext *fmt_ctx = NULL;
    AVDictionaryEntry *tag = NULL;
    int ret;

    if ((ret = avformat_open_input(&fmt_ctx, videoFile.c_str(), NULL, NULL)) < 0) {
        spdlog::error("Could not open source file {}: {}", videoFile, av_err2str(ret));
        return;
    }

    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        spdlog::error("Could not find stream information: {}", av_err2str(ret));
        avformat_close_input(&fmt_ctx);
        return;
    }

    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        if (strcmp(tag->key, "title") == 0) {
            av_dict_set(&fmt_ctx->metadata, "title", title.c_str(), 0);
            spdlog::info("Video title updated to: {}", title);
        } else if (strcmp(tag->key, "artist") == 0) {
            av_dict_set(&fmt_ctx->metadata, "artist", author.c_str(), 0);
            spdlog::info("Video artist updated to: {}", author);
        }
    }

    avformat_close_input(&fmt_ctx);
}

void extractFrame(string videoFile, int frameRate, string saveDir) {
    AVFormatContext* pFormatCtx = NULL;
    int             i, videoStream;
    AVCodecContext* pCodecCtxOrig = NULL;
    AVCodecContext* pCodecCtx = NULL;
    AVCodec*        pCodec = NULL;
    AVFrame*        pFrame = NULL;
    AVPacket        packet;
    int             frameFinished;
    float           aspect_ratio;
    struct SwsContext* sws_ctx = NULL;

    if (avformat_open_input(&pFormatCtx, videoFile.c_str(), NULL, NULL) != 0) {
        spdlog::error("Failed to open video file: {}", videoFile);
        return;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        spdlog::error("Failed to retrieve input stream information");
        avformat_close_input(&pFormatCtx);
        return;
    }

    av_dump_format(pFormatCtx, 0, videoFile.c_str(), 0);

    videoStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    if (videoStream == -1) {
        spdlog::error("Failed to find a video stream");
        avformat_close_input(&pFormatCtx);
        return;
    }

    pCodecCtxOrig = avcodec_alloc_context3(NULL);
    if (!pCodecCtxOrig) {
        spdlog::error("Failed to allocate codec context");
        avformat_close_input(&pFormatCtx);
        return;
    }

    avcodec_parameters_to_context(pCodecCtxOrig, pFormatCtx->streams[videoStream]->codecpar);

    pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
    if (!pCodec) {
        spdlog::error("Failed to find decoder");
        avformat_close_input(&pFormatCtx);
        avcodec_free_context(&pCodecCtxOrig);
        return;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        spdlog::error("Failed to allocate decoder context");
        avformat_close_input(&pFormatCtx);
        avcodec_free_context(&pCodecCtxOrig);
        return;
    }

    if (avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar) < 0) {
        spdlog::error("Failed to copy decoder parameters to input decoder context");
        avformat_close_input(&pFormatCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avcodec_free_context(&pCodecCtx);
        return;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        spdlog::error("Failed to open codec");
        avformat_close_input(&pFormatCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avcodec_free_context(&pCodecCtx);
        return;
    }

    pFrame = av_frame_alloc();
    if (!pFrame) {
        spdlog::error("Failed to allocate frame");
        avformat_close_input(&pFormatCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avcodec_free_context(&pCodecCtx);
        return;
    }

    int frameNumber = -1;
    int numBytes;
    uint8_t* buffer = NULL;

    FILE* outFile;
    string filename = saveDir + "/frame";
    if (av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, 1) < 0) {
        spdlog::error("Failed to allocate raw image buffer");
        avformat_close_input(&pFormatCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avcodec_free_context(&pCodecCtx);
        av_frame_free(&pFrame);
        return;
    }

    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
        spdlog::error("Failed to initialize sws context");
        avformat_close_input(&pFormatCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avcodec_free_context(&pCodecCtx);
        av_frame_free(&pFrame);
        return;
    }

    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        if (packet.stream_index == videoStream) {
            avcodec_send_packet(pCodecCtx, &packet);
            frameNumber++;
            while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                char str[16] = { 0 };
                sprintf_s(str, sizeof(str), "%04d.bmp", frameNumber);
                filename += str;

                if (frameNumber % frameRate == 0) {
                    CImg<uint8_t> img(pFrame->data[0], pCodecCtx->width, pCodecCtx->height, 1, pFrame->linesize[0]);
                    img.save(filename.c_str());
                }

                filename = saveDir + "/frame";
                av_frame_unref(pFrame);
            }
        }
        av_packet_unref(&packet);
    }

    avformat_close_input(&pFormatCtx);
    avcodec_free_context(&pCodecCtxOrig);
    avcodec_free_context(&pCodecCtx);
    av_frame_free(&pFrame);
}

/**
 * @brief 将一系列图片合成视频
 * 
 * @param img_paths 图片路径列表
 * @param output_path 输出视频路径
 * @param fps 视频帧率
 * @param width 视频宽度
 * @param height 视频高度
 * @param log_path 日志文件路径
 */
void images_to_video(const vector<string>& img_paths, const string& output_path, const int fps, const int width, const int height, const string& log_path) {
    // 初始化spdlog日志
    auto logger = spdlog::basic_logger_mt("basic_logger", log_path, true);
    logger->info("开始将图片转化为视频");

    // 视频编码器设置
    AVCodec* codec = avcodec_find_encoder_by_name("libx264");
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->time_base = AVRational({1, fps});
    codec_ctx->framerate = AVRational({fps, 1});
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    avcodec_open2(codec_ctx, codec, nullptr);

    // 输出格式设置
    AVFormatContext* format_ctx = nullptr;
    avformat_alloc_output_context2(&format_ctx, nullptr, nullptr, output_path.c_str());
    AVOutputFormat* format = format_ctx->oformat;

    // 从视频编码器获取输出流
    AVStream* stream = avformat_new_stream(format_ctx, codec);
    avcodec_parameters_from_context(stream->codecpar, codec_ctx);
    avio_open(&format_ctx->pb, output_path.c_str(), AVIO_FLAG_WRITE);

    // 初始化视频写入器
    avformat_write_header(format_ctx, nullptr);
    AVFrame* frame = av_frame_alloc();
    frame->format = codec_ctx->pix_fmt;
    frame->width = codec_ctx->width;
    frame->height = codec_ctx->height;
    av_frame_get_buffer(frame, 0);

    SwsContext* sws_ctx = sws_getContext(width, height, AV_PIX_FMT_RGB24, width, height, AV_PIX_FMT_YUV420P, 0, 0, 0, 0);

    // 写入视频
    for (const string& img_path : img_paths) {
        cout << "正在处理图片: " << img_path << endl;

        CImg<unsigned char> img(img_path.c_str());  // 使用CImg库读取图片
        img.resize(codec_ctx->width, codec_ctx->height, -100, -100, 3);  // 调整图片大小

        // 裁剪图片
        if (img.width() > codec_ctx->width || img.height() > codec_ctx->height) {
            double scale = min(1.0 * codec_ctx->width / img.width(), 1.0 * codec_ctx->height / img.height());
            int new_width = round(img.width() * scale);
            int new_height = round(img.height() * scale);
            img.crop((codec_ctx->width - new_width) / 2, (codec_ctx->height - new_height) / 2, (codec_ctx->width + new_width) / 2, (codec_ctx->height + new_height) / 2);
        }

        // 将CImg格式的图片转化为FFmpeg格式的帧
        av_image_fill_arrays(frame->data, frame->linesize, img.data(), AV_PIX_FMT_RGB24, width, height, 1);
        sws_scale(sws_ctx, frame->data, frame->linesize, 0, height, frame->data, frame->linesize);

        // 写入视频帧
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = nullptr;
        pkt.size = 0;
        avcodec_send_frame(codec_ctx, frame);
        while (avcodec_receive_packet(codec_ctx, &pkt) == 0) {
            av_interleaved_write_frame(format_ctx, &pkt);
        }
    }

    // 清理资源
    av_write_trailer(format_ctx);
    avcodec_free_context(&codec_ctx);
    av_frame_free(&frame);
    sws_freeContext(sws_ctx);
    avio_close(format_ctx->pb);
    avformat_free_context(format_ctx);
    logger->info("视频生成完成");
}