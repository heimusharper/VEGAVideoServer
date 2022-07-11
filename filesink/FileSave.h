#ifndef FILESAVE_H
#define FILESAVE_H
#include "../MavContext.h"
#include "IPacketReader.h"
#include "fmt/format.h"
#include "helper.h"
#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

class FileSave : public IPacketReader
{
public:
    FileSave(const std::string &suffix, bool telem);
    virtual ~FileSave();
    virtual void onCreateStream(AVStream *stream) override;

    tm now() const {
        struct tm  tstruct;
        time_t nowTime_t = MavContext::instance().time() / 1000000.;
        memcpy(&tstruct, localtime(&nowTime_t), sizeof(struct tm));
        return tstruct;
    }
private:
    void run();
private:
    const std::string m_suffix;
    const bool m_telem;

    std::atomic_bool m_stop;
    std::thread *m_mainThread;

    std::mutex m_contextLock;

    AVStream *m_sourceStream = nullptr;
    AVFormatContext *m_context = nullptr;

    std::chrono::high_resolution_clock::time_point m_streamStart;
};

#endif // FFMPEGFILESAVE_H