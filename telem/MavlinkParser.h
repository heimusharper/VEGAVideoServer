#ifndef MAVLINKPARSER_H
#define MAVLINKPARSER_H
#include <queue>
#include <all/mavlink.h>
#include <assert.h>
#include <thread>
#include "MavContext.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <list>
#include <mutex>
#include <unistd.h>

class MavlinkParser
{
public:
    MavlinkParser(int gcs_id);
    ~MavlinkParser();

    // parse char data
    void read(char *c, int n);

    int lifetime() const;

    // pop message and send to client(UAV)
    int write(char *buff, int max);

private:

    void ping();
    void read(const mavlink_message_t &msg);

    void processIntervals();

private:
    const int GCS_ID = 127;

    std::chrono::time_point<std::chrono::system_clock> m_lastRead;

    const uint8_t DIFFERENT_CHANNEL = 0;
    std::queue<mavlink_message_t> m_messageToSend;

    int UAV_ID = -1;
    int UAV_COMPID = 1;

    std::mutex m_messageMutex;
    std::thread *m_ping = nullptr;
    std::atomic_bool m_stop;

    uint16_t m_gpsFixType = GPS_FIX_TYPE::GPS_FIX_TYPE_NO_GPS;
};

#endif // MAVLINKPARSER_H
