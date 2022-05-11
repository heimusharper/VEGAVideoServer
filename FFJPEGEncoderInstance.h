#ifndef FFJPEGENCODERINSTANCE_H
#define FFJPEGENCODERINSTANCE_H

#include "FFH264DecoderInstance.h"

class FFJPEGEncoderInstance
{
public:
    FFJPEGEncoderInstance(const std::string& address, bool sync, int w, int h,
                          const std::string& preset, const std::string&tune,
                          int quality);
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
    bool m_sync;
    int m_targetW;
    int m_targetH;

    int m_quality;

    int w = 0;
    int h = 0;
    uint8_t *buffer = nullptr;

    float m_scaleFactor = 1;

    FFH264DecoderInstance *m_decoder = nullptr;
    // decoder
    AVCodecContext *m_jpegContext = nullptr;
    SwsContext* yuv420_conversion = nullptr;
};

#endif // FFH264DECODERINSTANCE_H
