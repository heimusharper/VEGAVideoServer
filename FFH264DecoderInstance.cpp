#include "FFH264DecoderInstance.h"

FFH264DecoderInstance::FFH264DecoderInstance(const std::string &preset, const std::string &tune)
    : IPacketReader()
    , m_sync(sync)
    , m_preset(preset)
    , m_tune(tune)
{
    //av_log_set_level(AV_LOG_TRACE);
    avdevice_register_all();
    avcodec_register_all();
    avformat_network_init();

    m_stop.store(false);
    m_mainThread = new std::thread(&FFH264DecoderInstance::run, this);
}

FFH264DecoderInstance::~FFH264DecoderInstance()
{
    m_stop.store(true);
    if (m_mainThread->joinable())
        m_mainThread->join();
    delete m_mainThread;
}

void FFH264DecoderInstance::onCreateStream(AVStream *stream)
{
#if defined (USE_NVMPI)
    std::cout << "codec" << stream->codec->codec_id << " " << (int)AV_CODEC_ID_H264 << std::endl;
    if (stream->codec->codec_id == AV_CODEC_ID_H264)
#endif
    {
        AVCodecParameters *codparam = avcodec_parameters_alloc();
        if (int err = avcodec_parameters_from_context(codparam, stream->codec) >= 0) {

            AVDictionary *options = nullptr;
            //av_dict_set(&options, "fflags", "nobuffer", 0);

#if defined (USE_NVMPI)
            std::cout << "Create H264 decoder...";
            AVCodec *acodec = avcodec_find_decoder_by_name("h264_nvmpi");
#else
            AVCodec *acodec = avcodec_find_decoder(stream->codec->codec_id);
#endif
            m_videoCodecContext = avcodec_alloc_context3(acodec);
            avcodec_parameters_to_context(m_videoCodecContext, codparam);

#if defined (USE_NVMPI)
#else
            if (stream->codec->codec_id == AV_CODEC_ID_H264)
            {
                av_opt_set(m_videoCodecContext->priv_data, "preset", m_preset.c_str(), 0);
                av_opt_set(m_videoCodecContext->priv_data, "tune", m_tune.c_str(), 0);
                av_opt_set(m_videoCodecContext->priv_data, "crf", "23", 0);
            }
            m_videoCodecContext->thread_count = std::max(4, m_videoCodecContext->thread_count);
#endif
            std::cout << "create codec" << std::endl;
            if (avcodec_open2(m_videoCodecContext, nullptr, &options) < 0)
            {
                std::cout << "failed open codec";
                avcodec_free_context(&m_videoCodecContext);
            } else {
                return;
            }
        } else
            std::cout << "failed create new decoder" << AVHelper::av2str(err);
    }
    //} else
    //    std::cout << "failed detect codec [h264, ]";
}

void FFH264DecoderInstance::run()
{
    AVPacket* packet = nullptr;
    while (!m_stop.load())
    {
        packet = takePacket();
        if (packet) {
            if (m_videoCodecContext) {
                int error = avcodec_send_packet(m_videoCodecContext, packet);
                if (error == AVERROR_EOF)
                    std::cout << "FFmpeg buf EOF";
                else if (error == AVERROR(EAGAIN))
                    std::cout << "FFmpeg buf EAGAIN" << std::endl;
                else if (error == AVERROR(EINVAL))
                    std::cout << "FFmpeg EINVAL codec not opened" << std::endl;
                else if (error == AVERROR(ENOMEM))
                    std::cout << "FFMpeg failure put queue packet" << std::endl;
                else if (error == 0) {
                    AVFrame *frame = av_frame_alloc();
                    if (frame) {
                        int decErr = avcodec_receive_frame(m_videoCodecContext, frame);
                        if (decErr == 0)
                        {
                            m_frameLock.lock();
                            if (m_frame)
                                av_frame_unref(m_frame);
                            else
                                m_frame = av_frame_alloc();
                            m_lastFrame = std::chrono::system_clock::now();
                            av_frame_ref(m_frame, frame);
                            m_frameLock.unlock();
                        }
                        av_frame_unref(frame);
                    }
                }
            }
            av_packet_unref(packet);
        }
        usleep(10);
    }
}
