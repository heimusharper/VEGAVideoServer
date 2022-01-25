#include "FFImageHttpSink.h"

FFImageHttpSink::~FFImageHttpSink()
{
    if (m_sink)
        delete m_sink;
}

FFImageHttpSink &FFImageHttpSink::instance()
{
    static FFImageHttpSink s;
    return s;
}

void FFImageHttpSink::create(const std::string &str)
{

    m_sink = new FFH264DecoderInstance(str);
}

Image *FFImageHttpSink::getImage()
{
    if (!m_sink)
        return nullptr;
    if (m_sink->lifetime() > 1000)
        return nullptr;
    AVPacket frame = m_sink->takeFrame();
    Image *image = new Image;
    image->image = new char[frame.size];
    image->size = frame.size;
    memcpy(image->image, frame.data, frame.size);
    av_packet_unref(&frame);
    return image;
}

FFImageHttpSink::FFImageHttpSink()
{
}
