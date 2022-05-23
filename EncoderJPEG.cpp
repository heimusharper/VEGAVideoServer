#include "EncoderJPEG.h"

EncoderJPEG::EncoderJPEG(Decoder *decoder, int w,
                                             int h, int quality)
    : m_targetW(w)
    , m_targetH(h)
    , m_quality(quality)
    , m_decoder(decoder)
{
}

AVPacket *EncoderJPEG::takeFrame()
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

    if (!m_jpegEncoderContext) {

        AVDictionary *options = nullptr;
        av_dict_set(&options, "fflags", "nobuffer", 0);

        LOG->info("Create mJPEG encoder...");
        const AVCodec* jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
        m_jpegEncoderContext = avcodec_alloc_context3(jpegCodec);
        m_jpegEncoderContext->bit_rate = m_decoder->bitrate();
        m_jpegEncoderContext->pix_fmt = AV_PIX_FMT_YUVJ420P; // m_videoCodecContext->pix_fmt;
        m_jpegEncoderContext->height = targetHeight;
        m_jpegEncoderContext->width = targetWidth;
        // m_jpegContext->sample_aspect_ratio = m_videoCodecContext->sample_aspect_ratio;
        m_jpegEncoderContext->time_base = AVRational { 1, 25 }; // m_videoCodecContext->time_base;
        m_jpegEncoderContext->flags |= AV_CODEC_FLAG_QSCALE;
        m_jpegEncoderContext->flags |= AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS;
        m_jpegEncoderContext->thread_count = std::max(2, m_jpegEncoderContext->thread_count);
        // m_jpegContext->qmin = 1;
        // m_jpegContext->qmax = 2;
        av_opt_set(m_jpegEncoderContext->priv_data, "q", std::to_string(m_quality).c_str(), 0);
        if (int err = avcodec_open2(m_jpegEncoderContext, jpegCodec, &options) < 0) {
            LOG->info("failed create mJPEG encoder {}", AVHelper::av2str(err));
        } else {
            m_yuv420ConversionContext = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format,
                targetWidth, targetHeight, AV_PIX_FMT_YUV420P,
                SWS_BICUBIC, NULL, NULL, NULL);
        }
    }
    if (m_jpegEncoderContext) {
        AVFrame* dstframe = av_frame_alloc();
        if (m_yuv420ConversionContext) {
            // init conversion
            if (buffer == nullptr || w != targetWidth || h != targetHeight) {
                if (buffer)
                    av_free(buffer);
#if 0
                int size = avpicture_get_size((AVPixelFormat)AV_PIX_FMT_YUV420P, targetWidth, targetHeight);
#else
                int size = av_image_get_buffer_size((AVPixelFormat)AV_PIX_FMT_YUV420P, targetWidth, targetHeight, 1);
#endif
                buffer = (uint8_t*)av_malloc(size);
                w = targetWidth;
                h = targetHeight;
            }
            if (buffer) {
                dstframe->format = AV_PIX_FMT_YUV420P;
                dstframe->width = targetWidth;
                dstframe->height = targetHeight;
#if 0
                avpicture_fill((AVPicture*)dstframe, buffer, (AVPixelFormat)dstframe->format, dstframe->width, dstframe->height);
#else
                av_image_fill_arrays(dstframe->data, dstframe->linesize, buffer, (AVPixelFormat)dstframe->format, dstframe->width, dstframe->height, 1);
#endif
                sws_scale(m_yuv420ConversionContext, frame->data, frame->linesize, 0, frame->height, dstframe->data, dstframe->linesize);
            } else {
                av_frame_unref(frame);
                return nullptr;
            }
        } else {
            av_frame_ref(dstframe, frame);
        }
        int err = avcodec_send_frame(m_jpegEncoderContext, dstframe);
        if (err == 0) {
            err = 0;
            do {
                AVPacket* p = av_packet_alloc();
                err = avcodec_receive_packet(m_jpegEncoderContext, p);
                if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
                } else if (err < 0) {
                    LOG->warn("Error encoding frame");
                } else {
                    p->dts = av_rescale_q(p->dts, m_jpegEncoderContext->time_base, m_jpegEncoderContext->time_base);
                    p->pts = av_rescale_q(p->pts, m_jpegEncoderContext->time_base, m_jpegEncoderContext->time_base);

                    av_frame_unref(dstframe);
                    av_frame_unref(frame);
                    return p;
                }
                av_packet_free(&p);
            } while (err >= 0);
        }
        av_frame_unref(dstframe);
    }
    av_frame_unref(frame);
    return nullptr;
}

EncoderJPEG::~EncoderJPEG()
{
    delete m_decoder;
    if (m_jpegEncoderContext)
        avcodec_free_context(&m_jpegEncoderContext);
    if (m_yuv420ConversionContext)
        sws_freeContext(m_yuv420ConversionContext);
}
