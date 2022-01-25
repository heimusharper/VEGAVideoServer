#ifndef FFDECODERINSTANCE_H
#define FFDECODERINSTANCE_H

#include "helper.h"
#include "FFImageFrame.h"
#include "FFPlayerInstance.h"
#include <thread>
#include <atomic>
#include <queue>
#include <boost/lockfree/spsc_queue.hpp>

class FFDecoderInstance
{
public:
    FFDecoderInstance()
    {
    }
    virtual ~FFDecoderInstance()
    {
    }
    AVPacket takeFrame()
    {
        std::lock_guard g(m_frameLock);
        AVPacket packet;
        av_packet_ref(&packet, &m_packet);
        return packet;
    }
    int lifetime() const
    {
        const std::chrono::time_point<std::chrono::system_clock> now =
            std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFrame).count();
    }

protected:
    std::chrono::time_point<std::chrono::system_clock> m_lastFrame;
    std::mutex m_frameLock;
    AVPacket m_packet;
};

#endif // FFDECODERINSTANCE_H
