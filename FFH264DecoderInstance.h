#ifndef FFH264DECODERINSTANCE_H
#define FFH264DECODERINSTANCE_H

#if defined (USE_NVMPI)
#include <nvmpi.h>
#endif
#include "helper.h"
#include "FFImageFrame.h"
#include "FFPlayerInstance.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>

class FFH264DecoderInstance
{
public:
    FFH264DecoderInstance(const std::string& address, bool sync);
    FFH264DecoderInstance(const FFH264DecoderInstance&c) = delete;
    virtual ~FFH264DecoderInstance();

    AVFrame *takeFrame()
    {
        std::lock_guard g(m_frameLock);
        if (!m_frame)
            return nullptr;
        AVFrame *packet = av_frame_alloc();
        av_frame_ref(packet, m_frame);
        return packet;
    }
    int lifetime() const
    {
        const std::chrono::time_point<std::chrono::system_clock> now =
            std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFrame).count();
    }
    int64_t bitrate() const
    {
        return m_videoCodecContext->bit_rate;
    }

private:
    void run();

private:
    std::atomic_bool m_stop;
    std::thread *m_mainThread;

    FFPlayerInstance *m_player = nullptr;

    bool m_sync;

    std::chrono::time_point<std::chrono::system_clock>
        m_lastFrame;
    std::mutex m_frameLock;
    AVFrame *m_frame = nullptr;

    // decoder
    AVStream *m_videoStream = nullptr;
    AVCodecContext *m_videoCodecContext = nullptr;
};

#endif // FFH264DECODERINSTANCE_H
