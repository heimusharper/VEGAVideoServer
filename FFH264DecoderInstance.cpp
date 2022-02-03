#include "FFH264DecoderInstance.h"

FFH264DecoderInstance::FFH264DecoderInstance(const std::string& address, bool sync)
    : m_sync(sync)
{
    //av_log_set_level(AV_LOG_TRACE);
    avdevice_register_all();
    avcodec_register_all();
    avformat_network_init();

    m_player = new FFPlayerInstance(address, m_sync,
        [this](AVStream* stream) -> bool {
#if defined (USE_NVMPI)
            if (stream->codec->codec_id == AV_CODEC_ID_H264)
#endif
            {
                AVCodecParameters *codparam = avcodec_parameters_alloc();
                if (int err = avcodec_parameters_from_context(codparam, stream->codec) >= 0) {
                    std::cout << "Create H264 decoder...";

                    AVDictionary *options = nullptr;
                    //av_dict_set(&options, "fflags", "nobuffer", 0);

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
                            std::lock_guard g(m_frameLock);
                            if (m_frame)
                                av_frame_unref(m_frame);
                            else
                                m_frame = av_frame_alloc();
                            av_frame_ref(m_frame, frame);
                        }
                        av_frame_unref(frame);
                    }
                }
                av_packet_free(&pkt);
            }
            usleep(10);
        } else {
            // long sleep
            usleep(10000);
        }
    }
    if (yuv420_conversion)
        sws_freeContext(yuv420_conversion);
}
