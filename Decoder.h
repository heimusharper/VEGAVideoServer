#ifndef DECODER_H
#define DECODER_H

#if defined (USE_NVMPI)
#include <nvmpi.h>
#endif
#include "helper.h"
#include <iostream>
#include <thread>
#include "MavContext.h"
#include <boost/lockfree/spsc_queue.hpp>
#include <atomic>
#include <mutex>
#include "IPacketReader.h"
#include <queue>

class Decoder : public IPacketReader
{
public:
    Decoder(const std::string &preset, const std::string &tune);
    Decoder(const Decoder&c) = delete;
    virtual ~Decoder();

    virtual void onCreateStream(AVStream *stream) override;

    AVFrame *takeFrame()
    {
        m_frameLock.lock();
        if (!m_frame) {
            m_frameLock.unlock();
            return nullptr;
        }
        AVFrame *frame = av_frame_alloc();
        av_frame_ref(frame, m_frame);
        m_frameLock.unlock();
        return frame;
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

    const std::string m_preset;
    const std::string m_tune;

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
