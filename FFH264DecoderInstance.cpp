#include "FFH264DecoderInstance.h"

FFH264DecoderInstance::FFH264DecoderInstance(const std::string& address, bool sync, int w, int h)
    : FFDecoderInstance(sync, w, h)
{
    //av_log_set_level(AV_LOG_TRACE);
    avdevice_register_all();
    avcodec_register_all();
    avformat_network_init();

    m_player = new FFPlayerInstance(address, m_sync,
        [this](AVStream* stream) -> bool {
            std::lock_guard g(m_codecContextLocker);
#if defined (USE_NVMPI)
            if (stream->codec->codec_id == AV_CODEC_ID_H264) {
#endif
            {
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
                    m_videoCodecContext->thread_count = std::max(3, m_videoCodecContext->thread_count);

                    if (avcodec_open2(m_videoCodecContext, nullptr, &options) < 0)
                    {
                        std::cout << "failed open codec";
                        avcodec_free_context(&m_videoCodecContext);
                    } else {
        #if defined (USE_NVMPI)
                        if (nvmpiInitDecoder(m_videoCodecContext) < 0)
                        {
                            std::cout << "Failed initialize nvmpi codec context";
                            avcodec_free_context(&m_videoCodecContext);
                            return false;
                        }
        #endif
                        return true;
                    }
                } else
                    std::cout << "failed create new decoder" << AVHelper::av2str(err);
            }
            }
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
#if defined (USE_NVMPI)
    nvmpi_decoder_close(((nvmpiDecodeContext *)m_videoCodecContext)->ctx);
#endif
    avformat_network_deinit();
}

void FFH264DecoderInstance::run()
{
    int w = 0;
    int h = 0;
    uint8_t *buffer = nullptr;

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
#if defined (USE_NVMPI)
                    nvmpiDecodeContext *nvmpi_context = (nvmpiDecodeContext*)m_videoCodecContext->priv_data;
                    nvPacket packet;
                    nvFrame _nvframe={0};
                    int error = 0;
                    if(pkt->size){
                        packet.payload_size=pkt->size;
                        packet.payload=pkt->data;
                        packet.pts=pkt->pts;
                        error = nvmpi_decoder_put_packet(nvmpi_context->ctx, &packet);
                        //std::cout << "RES TEST:" << error << std::endl;
                    }
                    error = nvmpi_decoder_get_frame(nvmpi_context->ctx,&_nvframe, m_videoCodecContext->flags & AV_CODEC_FLAG_LOW_DELAY);
#else
                    int error = avcodec_send_packet(m_videoCodecContext, pkt);
#endif
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
#if defined (USE_NVMPI)
                            int linesize[3];
                            linesize[0]=_nvframe.linesize[0];
                            linesize[1]=_nvframe.linesize[1];
                            linesize[2]=_nvframe.linesize[2];
                            uint8_t* ptrs[3];
                            ptrs[0]=_nvframe.payload[0];
                            ptrs[1]=_nvframe.payload[1];
                            ptrs[2]=_nvframe.payload[2];
                            av_image_copy(frame->data, frame->linesize, (const uint8_t **) ptrs, linesize, m_videoCodecContext->pix_fmt, _nvframe.width,_nvframe.height);

                            frame->width=_nvframe.width;
                            frame->height=_nvframe.height;

                            frame->format=AV_PIX_FMT_YUV420P;
                            frame->pts=_nvframe.timestamp;
                            frame->pkt_dts = AV_NOPTS_VALUE;

                            m_videoCodecContext->coded_width=_nvframe.width;
                            m_videoCodecContext->coded_height=_nvframe.height;
                            m_videoCodecContext->width=_nvframe.width;
                            m_videoCodecContext->height=_nvframe.height;
#else

#endif
                            int targetWidth = frame->width;
                            int targetHeight = frame->height;

                            if (m_targetH == 0 && m_targetW > 0) {
                                // width priority
                                m_scaleFactor = (float)m_targetW / (float)targetWidth;
                                targetHeight = targetHeight * m_scaleFactor;
                                targetWidth = m_targetW;
                            } else if (m_targetW == 0 && m_targetH > 0) {
                                m_scaleFactor = (float)m_targetH / (float)targetHeight;
                                targetWidth = targetWidth * m_scaleFactor;
                                targetHeight = m_targetH;
                            } else if (m_targetH > 0 && m_targetW > 0) {
                                m_scaleFactor = std::min((float)m_targetW / (float)targetWidth,
                                    (float)m_targetH / (float)targetHeight);
                                targetHeight = m_targetH;
                                targetWidth = m_targetW;
                            }

                            if (!m_jpegContext) {

                                AVDictionary *options = nullptr;
                                av_dict_set(&options, "fflags", "nobuffer", 0);

                                std::cout << "Create mJPEG encoder...";
                                AVCodec *jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
                                m_jpegContext = avcodec_alloc_context3(jpegCodec);
                                m_jpegContext->bit_rate = m_videoCodecContext->bit_rate;
                                m_jpegContext->pix_fmt = AV_PIX_FMT_YUVJ420P; // m_videoCodecContext->pix_fmt;
                                m_jpegContext->height = targetHeight;
                                m_jpegContext->width = targetWidth;
                                // m_jpegContext->sample_aspect_ratio = m_videoCodecContext->sample_aspect_ratio;
                                m_jpegContext->time_base = AVRational{1, 25};// m_videoCodecContext->time_base;
                                m_jpegContext->flags |= AV_CODEC_FLAG_QSCALE;
                                m_jpegContext->flags |= AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS;
                                // m_jpegContext->qmin = 1;
                                // m_jpegContext->qmax = 2;
                                av_opt_set(m_jpegContext->priv_data, "q", "30", 0);
                                if (int err = avcodec_open2(m_jpegContext, jpegCodec, &options) < 0) {
                                    std::cout << "failed create mJPEG encoder" << AVHelper::av2str(err);
                                } else {
                                    yuv420_conversion = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format,
                                        targetWidth, targetHeight, AV_PIX_FMT_YUV420P,
                                        SWS_BICUBIC, NULL, NULL, NULL);
                                }
                            }

                            if (m_jpegContext) {
                                if (yuv420_conversion)
                                {
                                    // convert to AV_PIX_FMT_YUV420P
                                    AVFrame *dstframe = av_frame_alloc();
                                    dstframe->format = AV_PIX_FMT_YUV420P;
                                    dstframe->width = targetWidth;
                                    dstframe->height = targetHeight;
                                    if (buffer == nullptr || w != dstframe->width || h != dstframe->height)
                                    {
                                        if (buffer)
                                            av_free(buffer);
                                        int size = avpicture_get_size((AVPixelFormat)dstframe->format, dstframe->width, dstframe->height);
                                        buffer = (uint8_t*)av_malloc(size);
                                        w = dstframe->width;
                                        h = dstframe->height;
                                    }
                                    if (buffer)
                                    {
                                        avpicture_fill((AVPicture*)dstframe, buffer, (AVPixelFormat)dstframe->format, dstframe->width, dstframe->height);
                                        sws_scale(yuv420_conversion, frame->data, frame->linesize, 0, frame->height, dstframe->data, dstframe->linesize);
                                        int got;
                                        std::lock_guard m(m_frameLock);
                                        if (m_packet)
                                            av_packet_unref(m_packet);
                                        else
                                            m_packet = av_packet_alloc();
                                        if (avcodec_encode_video2(m_jpegContext, m_packet, dstframe, &got) >= 0)
                                            m_lastFrame = std::chrono::system_clock::now();
                                    }
                                    av_frame_free(&dstframe);
                                } else {
                                    int got;
                                    std::lock_guard m(m_frameLock);
                                    if (m_packet)
                                        av_packet_unref(m_packet);
                                    else
                                        m_packet = av_packet_alloc();
                                    if (avcodec_encode_video2(m_jpegContext, m_packet, frame, &got) >= 0)
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
            usleep(1000);
        }
    }
    if (yuv420_conversion)
        sws_freeContext(yuv420_conversion);
}
