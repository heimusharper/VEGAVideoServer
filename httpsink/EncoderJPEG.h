#ifndef ENCODERJPEG_H
#define ENCODERJPEG_H

#include "Decoder.h"
#include "helper.h"

class EncoderJPEG
{
public:
    EncoderJPEG(Decoder *decoder, int w, int h, int quality);
    EncoderJPEG(const EncoderJPEG&c) = delete;
    virtual ~EncoderJPEG();

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

    Decoder* m_decoder = nullptr;
    // decoder
    AVCodecContext* m_jpegEncoderContext = nullptr;
    SwsContext* m_yuv420ConversionContext = nullptr;
};

#endif // FFH264DECODERINSTANCE_H