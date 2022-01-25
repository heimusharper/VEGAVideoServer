#include "FFH264DecoderInstance.h"

FFH264DecoderInstance::FFH264DecoderInstance(const std::string &address) :
    FFDecoderInstance()
{
    m_player = new FFPlayerInstance(address,
                                    [this](AVStream*stream) -> bool {
        std::lock_guard g(m_codecContextLocker);
        //if (stream->codec->codec_id == AV_CODEC_ID_H264) {
            AVCodecParameters *codparam = avcodec_parameters_alloc();
            if (int err = avcodec_parameters_from_context(codparam, stream->codec) >= 0) {
                std::cout << "Create H264 decoder...";

                AVDictionary *options = nullptr;
                av_dict_set(&options, "fflags", "nobuffer", 0);

                AVCodec *acodec = avcodec_find_decoder(stream->codec->codec_id);
                m_videoCodecContext = avcodec_alloc_context3(acodec);
                avcodec_parameters_to_context(m_videoCodecContext, codparam);

                if (stream->codec->codec_id == AV_CODEC_ID_H264)
                {
                    av_opt_set(m_videoCodecContext->priv_data, "preset", "ultrafast", 0);
                    av_opt_set(m_videoCodecContext->priv_data, "tune", "zerolatency", 0);
                    av_opt_set(m_videoCodecContext->priv_data, "crf", "23", 0);
                }
                m_videoCodecContext->thread_count = std::max(4, m_videoCodecContext->thread_count);

                if (avcodec_open2(m_videoCodecContext, nullptr, &options) < 0)
                {
                    std::cout << "failed open codec";
                    avcodec_free_context(&m_videoCodecContext);
                } else {
                    return true;
                }
            } else
                std::cout << "failed create new decoder" << AVHelper::av2str(err);
        //} else
        //    std::cout << "failed detect codec [h264, ]";
        return false;
    });

    m_stop.store(false);
    m_mainThread = new std::thread(&FFH264DecoderInstance::run, this);
}

FFH264DecoderInstance::~FFH264DecoderInstance()
{
    m_stop.store(true);
    if (m_mainThread->joinable())
        m_mainThread->join();
    delete m_mainThread;

    delete m_player;
}

void FFH264DecoderInstance::run()
{
    SwsContext* yuv420_conversion = nullptr;
    std::queue<AVPacket*> packets;
    while (!m_stop.load())
    {
        m_player->takePackets(packets);
        if (!packets.empty()) {

            while (!packets.empty()) {
                AVPacket *pkt = packets.front();
                packets.pop();
                std::lock_guard g(m_codecContextLocker);
                if (m_videoCodecContext) {
                    int error = avcodec_send_packet(m_videoCodecContext, pkt);
                    if (error == AVERROR_EOF)
                        std::cout << "FFmpeg buf EOF";
                    else if (error == AVERROR(EAGAIN))
                        std::cout << "FFmpeg buf EAGAIN";
                    else if (error == AVERROR(EINVAL))
                        std::cout << "FFmpeg EINVAL codec not opened";
                    else if (error == AVERROR(ENOMEM))
                        std::cout << "FFMpeg failure put queue packet";
                    else if (error == 0) {
                        AVFrame *frame = av_frame_alloc();
                        int decErr = avcodec_receive_frame(m_videoCodecContext, frame);
                        if (decErr == 0)
                        {
                            if (!m_jpegContext) {
                                std::cout << "Create mJPEG encoder...";
                                AVCodec *jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
                                m_jpegContext = avcodec_alloc_context3(jpegCodec);
                                m_jpegContext->bit_rate = m_videoCodecContext->bit_rate;
                                m_jpegContext->pix_fmt = AV_PIX_FMT_YUVJ420P; // m_videoCodecContext->pix_fmt;
                                m_jpegContext->height = frame->height;
                                m_jpegContext->width = frame->width;
                                m_jpegContext->flags |= AV_CODEC_FLAG_QSCALE;
                                // m_jpegContext->qmin = 30;
                                //  m_jpegContext->qmax = 31;
                                m_jpegContext->time_base = m_videoCodecContext->time_base;
                                if (int err = avcodec_open2(m_jpegContext, jpegCodec, NULL) < 0) {
                                    std::cout << "failed create mJPEG encoder" << AVHelper::av2str(err);
                                } else {
                                    if (m_videoCodecContext->pix_fmt != AV_PIX_FMT_YUV420P)
                                        yuv420_conversion = sws_getContext(frame->width, frame->height, (AVPixelFormat) frame->format,
                                                                           frame->width, frame->height, AV_PIX_FMT_YUV420P,
                                                                           SWS_BICUBIC, NULL, NULL, NULL);
                                }

                            }

                            std::lock_guard m(m_frameLock);
                            av_packet_unref(&m_packet);
                            av_init_packet(&m_packet);
                            if (m_jpegContext) {
                                if (yuv420_conversion)
                                {
                                    // convert to AV_PIX_FMT_YUV420P
                                    AVFrame *dstframe = av_frame_alloc();
                                    dstframe->format = AV_PIX_FMT_YUV420P;
                                    dstframe->width  = frame->width;
                                    dstframe->height = frame->height;
                                    if (av_image_alloc(dstframe->data, dstframe->linesize, frame->width, frame->height, (AVPixelFormat)frame->format, 0) >= 0) {
                                        sws_scale(yuv420_conversion, frame->data, frame->linesize, 0, frame->height, dstframe->data, dstframe->linesize);
                                        int got;
                                        if (avcodec_encode_video2(m_jpegContext, &m_packet, dstframe, &got) >= 0)
                                        {
                                            m_lastFrame = std::chrono::system_clock::now();
                                        }
                                    }
                                    av_frame_free(&dstframe);
                                } else {
                                    int got;
                                    if (avcodec_encode_video2(m_jpegContext, &m_packet, frame, &got) >= 0)
                                    {
                                        m_lastFrame = std::chrono::system_clock::now();
                                    }
                                }
                            }
                        }

                        av_frame_free(&frame);
                    }
                }
                av_packet_free(&pkt);
            }
            usleep(1000);
        } else {
            // long sleep
            usleep(1000000);
        }
    }
    if (yuv420_conversion)
        sws_freeContext(yuv420_conversion);
}
