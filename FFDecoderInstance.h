#ifndef FFDECODERINSTANCE_H
#define FFDECODERINSTANCE_H

#include "helper.h"
#include "FFImageFrame.h"
#include "FFPlayerInstance.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <boost/lockfree/spsc_queue.hpp>

class FFDecoderInstance
{
public:
    FFDecoderInstance(bool sync, int w, int h)
        : m_sync(sync)
        , m_targetW(w)
        , m_targetH(h)
    {
    }
    virtual ~FFDecoderInstance()
    {
    }
    AVPacket *takeFrame()
    {
        std::lock_guard g(m_frameLock);
        if (!m_packet)
            return nullptr;
        AVPacket *packet = av_packet_alloc();
        av_packet_ref(packet, m_packet);
        return packet;
    }
    int lifetime() const
    {
        const std::chrono::time_point<std::chrono::system_clock> now =
            std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFrame).count();
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

protected:
    bool m_sync;
    int m_targetW;
    int m_targetH;

    std::chrono::time_point<std::chrono::system_clock>
        m_lastFrame;
    std::mutex m_frameLock;
    AVPacket *m_packet = nullptr;
    float m_scaleFactor = 1;
};

#endif // FFDECODERINSTANCE_H
