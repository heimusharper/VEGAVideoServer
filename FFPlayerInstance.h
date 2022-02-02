#ifndef FFPLAYERINSTANCE_H
#define FFPLAYERINSTANCE_H

#include "helper.h"
#include <atomic>
#include <boost/chrono/chrono.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <exception>
#include <functional>
#include <iostream>
#include <queue>
#include <thread>

class FFPlayerInstance
{
public:
    FFPlayerInstance(const std::string& address, bool sync,
        std::function<bool(AVStream*)> create);
    ~FFPlayerInstance();

    void takePackets(std::queue<AVPacket*> &pkt);

private:

    void run();

private:
    std::string m_address;
    const bool m_sync;
    std::function<bool(AVStream*)> m_fnCreate;

    std::atomic_bool m_stop;
    std::thread *m_mainThread;


    boost::lockfree::spsc_queue<AVPacket*, boost::lockfree::capacity<50>> m_packets;

};

#endif // FFPLAYERINSTANCE_H
