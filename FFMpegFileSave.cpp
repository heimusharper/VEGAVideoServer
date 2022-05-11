#include "FFMpegFileSave.h"

FFMpegFileSave::FFMpegFileSave(const std::string &suffix)
    : IPacketReader()
    , m_suffix(suffix)
{
    m_stop.store(false);
    m_mainThread = new std::thread(&FFH264DecoderInstance::run, this);
}

FFMpegFileSave::~FFMpegFileSave()
{
    m_stop.store(true);
    if (m_mainThread->joinable())
        m_mainThread->join();
    delete m_mainThread;
}

void FFMpegFileSave::onCreateStream(AVStream *stream)
{

}

void FFMpegFileSave::run()
{
    AVCodecContext *context;
    while(!m_stop.load())
    {
        if (!stream.is_open()) {

            // open io
            avio_open(context->pb, filename, AVIO_FLAG_WRITE);
            usleep(100000);
        } else {

            usleep(100);
        }
    }
}
