#ifndef FFPLAYERINSTANCE_H
#define FFPLAYERINSTANCE_H

#include "helper.h"
#include <queue>
#include <boost/lockfree/spsc_queue.hpp>
#include <thread>
#include <atomic>
#include <iostream>
#include <functional>
#include <exception>

class FFPlayerInstance
{
public:
    FFPlayerInstance(const std::string &address,
                     std::function<bool(AVStream*)> create);
    ~FFPlayerInstance();

    void takePackets(std::queue<AVPacket*> &pkt);

private:

    void run();

private:
    std::string m_address;
    std::function<bool(AVStream*)> m_fnCreate;

    std::atomic_bool m_stop;
    std::thread *m_mainThread;


    boost::lockfree::spsc_queue<AVPacket*, boost::lockfree::capacity<50>> m_packets;

};

#endif // FFPLAYERINSTANCE_H
