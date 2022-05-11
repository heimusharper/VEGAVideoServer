#ifndef FFJPEGENCODERINSTANCE_H
#define FFJPEGENCODERINSTANCE_H

#include "FFH264DecoderInstance.h"
#include "FFMpegFileSave.h"

class FFJPEGEncoderInstance
{
public:
    FFJPEGEncoderInstance(FFH264DecoderInstance *decoder, int w, int h, int quality);
    FFJPEGEncoderInstance(const FFJPEGEncoderInstance&c) = delete;
    virtual ~FFJPEGEncoderInstance();

    AVPacket *takeFrame();

    int lifetime() const
    {
        return m_decoder->lifetime();
    }
    float scaleFactor() const
    {
        return m_scaleFactor;
    }

    int width() const
    {
        return m_targetW;
    }
    int height() const
    {
        return m_targetH;
    }

private:
    const int m_targetW;
    const int m_targetH;

    const int m_quality;

    int w = 0;
    int h = 0;
    uint8_t *buffer = nullptr;

    float m_scaleFactor = 1;

    FFH264DecoderInstance *m_decoder = nullptr;
    FFMpegFileSave *m_encoder = nullptr;
    // decoder
    AVCodecContext *m_jpegContext = nullptr;
    SwsContext* yuv420_conversion = nullptr;
};

#endif // FFH264DECODERINSTANCE_H
