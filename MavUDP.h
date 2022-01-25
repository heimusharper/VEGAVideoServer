#ifndef MAVUDP_H
#define MAVUDP_H
#include <arpa/inet.h>
#include <cmath>
#include <fcntl.h>
#include <mutex>
#include <netinet/in.h>
#include <queue>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include "MavContext.h"
#include <errno.h>
#include <string>
#include <cstring>
#include <uchar.h>
#include <map>
#include "MavlinkParser.h"
#include <iostream>

typedef int UDPSocket;
class MavUDP
{
public:
    MavUDP(const std::string &host, uint16_t port, uint8_t gcs);
    ~MavUDP();

private:

    void run();

private:

    const std::string m_hostName;
    const uint16_t m_port;
    const uint8_t m_gcsID = 0;
    const size_t MAX_LINE;

    std::atomic_bool m_stop;
    std::thread *m_thread = nullptr;


    struct SockAddr
    {
        SockAddr(struct sockaddr_in a) : addr(a)
        {
        }
        bool operator<(const SockAddr &a) const
        {
            return this->addr.sin_addr.s_addr < a.addr.sin_addr.s_addr || this->addr.sin_port < a.addr.sin_port;
        }
        struct sockaddr_in addr;
    };
    std::map<SockAddr, MavlinkParser *> m_clients;

};

#endif // MAVUDP_H
