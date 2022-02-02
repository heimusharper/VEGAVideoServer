#ifndef FFH264DECODERINSTANCE_H
#define FFH264DECODERINSTANCE_H

#include "FFDecoderInstance.h"
extern "C"
{
#include <jpeglib.h>
}

class FFH264DecoderInstance : public FFDecoderInstance
{
public:
    FFH264DecoderInstance(const std::string& address, bool sync, int w, int h);
    FFH264DecoderInstance(const FFH264DecoderInstance&c) = delete;
    virtual ~FFH264DecoderInstance();

private:

    void run();

private:
    bool m_sync;
    int m_targetW;
    int m_targetH;
    std::atomic_bool m_stop;
    std::thread *m_mainThread;

    FFPlayerInstance *m_player = nullptr;

    // decoder
    AVStream *m_videoStream = nullptr;
    std::mutex m_codecContextLocker;
    AVCodecContext *m_videoCodecContext = nullptr;

    AVCodecContext *m_jpegContext = nullptr;

};

#endif // FFH264DECODERINSTANCE_H
