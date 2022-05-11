#ifndef FFPLAYERINSTANCE_H
#define FFPLAYERINSTANCE_H

#include "helper.h"
#include <atomic>
#include <boost/chrono/chrono.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <exception>
#include <functional>
#include <IPacketReader.h>
#include <iostream>
#include <queue>
#include <thread>

class FFPlayerInstance
{
public:
    FFPlayerInstance();
    ~FFPlayerInstance();

    void start(const std::string& address, bool sync);


    void addReader(IPacketReader *r)
    {
        m_readers.push_back(r);
    }

private:

    void run();

private:
    std::string m_address;
    bool m_sync;

    std::atomic_bool m_stop;
    std::thread *m_mainThread;

    std::vector<IPacketReader*> m_readers;
};

#endif // FFPLAYERINSTANCE_H
