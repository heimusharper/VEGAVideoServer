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
        m_frameLock.lock();
        if (!m_frame) {
            m_frameLock.unlock();
            return nullptr;
        }
        AVFrame *packet = av_frame_alloc();
        av_frame_ref(packet, m_frame);
        m_frameLock.unlock();
        return packet;
    }
    int lifetime()
    {
        //!TODO m_lastFrame - не инициализированна и не обновляется
        m_frameLock.lock();
        if (!m_frame) {
            m_frameLock.unlock();
            return 1000000;
        }
        const std::chrono::time_point<std::chrono::system_clock> now =
            std::chrono::system_clock::now();
        int lt = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFrame).count();
        m_frameLock.unlock();
        return lt;
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
