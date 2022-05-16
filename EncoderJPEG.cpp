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

    if (!m_jpegContext)
    {

        AVDictionary *options = nullptr;
        av_dict_set(&options, "fflags", "nobuffer", 0);

        LOG->info("Create mJPEG encoder...");
        const AVCodec* jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
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
        av_opt_set(m_jpegContext->priv_data, "q", std::to_string(m_quality).c_str(), 0);
        if (int err = avcodec_open2(m_jpegContext, jpegCodec, &options) < 0) {
            LOG->info("failed create mJPEG encoder {}", AVHelper::av2str(err));
        } else {
            yuv420_conversion = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format,
                targetWidth, targetHeight, AV_PIX_FMT_YUV420P,
                SWS_BICUBIC, NULL, NULL, NULL);
        }
    }
    if (m_jpegContext)
    {
        AVFrame* dstframe = nullptr;
        if (yuv420_conversion) {
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
                dstframe = av_frame_alloc();
                dstframe->format = AV_PIX_FMT_YUV420P;
                dstframe->width = targetWidth;
                dstframe->height = targetHeight;
#if 0
                avpicture_fill((AVPicture*)dstframe, buffer, (AVPixelFormat)dstframe->format, dstframe->width, dstframe->height);
#else
                av_image_fill_arrays(dstframe->data, dstframe->linesize, buffer, (AVPixelFormat)dstframe->format, dstframe->width, dstframe->height, 1);
#endif
                sws_scale(yuv420_conversion, frame->data, frame->linesize, 0, frame->height, dstframe->data, dstframe->linesize);
            } else {
                return nullptr;
            }
        } else {
            dstframe = frame;
        }
        int err = avcodec_send_frame(m_jpegContext, dstframe);
        if (err == 0) {
            err = 0;
            do {
                AVPacket* p = av_packet_alloc();
                err = avcodec_receive_packet(m_jpegContext, p);
                if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
                    av_packet_free(&p);
                    break;
                } else if (err < 0) {
                    LOG->warn("Error encoding frame");
                    av_packet_free(&p);
                    break;
                }
                p->dts = av_rescale_q(p->dts, m_jpegContext->time_base, m_jpegContext->time_base);
                p->pts = av_rescale_q(p->pts, m_jpegContext->time_base, m_jpegContext->time_base);

                av_frame_unref(dstframe);
                av_frame_unref(frame);
                return p;
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
}
