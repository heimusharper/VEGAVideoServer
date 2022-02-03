#include "FFJPEGEncoderInstance.h"

FFJPEGEncoderInstance::FFJPEGEncoderInstance(const std::string& address, bool sync, int w, int h)
    : m_sync(sync)
    , m_targetW(w)
    , m_targetH(h)
{
    m_decoder = new FFH264DecoderInstance(address, sync);
    //av_log_set_level(AV_LOG_TRACE);
    avdevice_register_all();
    avcodec_register_all();
    avformat_network_init();
}

AVPacket *FFJPEGEncoderInstance::takeFrame()
{
    AVFrame *frame = m_decoder->takeFrame();
    if (!frame)
        return nullptr;
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

    if (!m_jpegContext)
    {

        AVDictionary *options = nullptr;
        av_dict_set(&options, "fflags", "nobuffer", 0);

        std::cout << "Create mJPEG encoder...";
        AVCodec *jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
        m_jpegContext = avcodec_alloc_context3(jpegCodec);
        m_jpegContext->bit_rate = m_decoder->bitrate();
        m_jpegContext->pix_fmt = AV_PIX_FMT_YUVJ420P; // m_videoCodecContext->pix_fmt;
        m_jpegContext->height = targetHeight;
        m_jpegContext->width = targetWidth;
        // m_jpegContext->sample_aspect_ratio = m_videoCodecContext->sample_aspect_ratio;
        m_jpegContext->time_base = AVRational{1, 25};// m_videoCodecContext->time_base;
        m_jpegContext->flags |= AV_CODEC_FLAG_QSCALE;
        m_jpegContext->flags |= AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS;
        m_jpegContext->thread_count = std::max(2, m_jpegContext->thread_count);
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
    } else  {
        if (yuv420_conversion)
        {
            // convert to AV_PIX_FMT_YUV420P
            if (buffer == nullptr || w != targetWidth || h != targetHeight)
            {
                if (buffer)
                    av_free(buffer);
                int size = avpicture_get_size((AVPixelFormat)AV_PIX_FMT_YUV420P, targetWidth, targetHeight);
                buffer = (uint8_t*)av_malloc(size);
                w = targetWidth;
                h = targetHeight;
            }
            if (buffer)
            {
                AVFrame *dstframe = av_frame_alloc();
                dstframe->format = AV_PIX_FMT_YUV420P;
                dstframe->width = targetWidth;
                dstframe->height = targetHeight;
                avpicture_fill((AVPicture*)dstframe, buffer, (AVPixelFormat)dstframe->format, dstframe->width, dstframe->height);
                sws_scale(yuv420_conversion, frame->data, frame->linesize, 0, frame->height, dstframe->data, dstframe->linesize);
                int got;
                AVPacket *m_packet = av_packet_alloc();
                if (avcodec_encode_video2(m_jpegContext, m_packet, dstframe, &got) >= 0) {
                    av_frame_unref(frame);
                    av_frame_free(&dstframe);
                    return m_packet;
                }
                av_frame_free(&dstframe);
                av_packet_free(&m_packet);
            }
        } else {
            int got;
            AVPacket *m_packet = av_packet_alloc();
            if (avcodec_encode_video2(m_jpegContext, m_packet, frame, &got) >= 0) {
                av_frame_unref(frame);
                return m_packet;
            }
            av_packet_free(&m_packet);
        }
    }
    av_frame_unref(frame);
    return nullptr;
}

FFJPEGEncoderInstance::~FFJPEGEncoderInstance()
{
    delete m_decoder;
}
