#ifndef FFMPEGFILESAVE_H
#define FFMPEGFILESAVE_H
#include "helper.h"
#include "FFImageFrame.h"
#include <iostream>
#include <thread>
#include "MavContext.h"
#include <boost/lockfree/spsc_queue.hpp>
#include <atomic>
#include <mutex>
#include "IPacketReader.h"
#include <queue>

class FFMpegFileSave : public IPacketReader
{
public:
    FFMpegFileSave(const std::string &suffix);
    virtual ~FFMpegFileSave();
    virtual void onCreateStream(AVStream *stream) override;

private:
    void run();
private:

    std::atomic_bool m_stop;
    std::thread *m_mainThread;

    const std::string m_suffix;
};

#endif // FFMPEGFILESAVE_H
