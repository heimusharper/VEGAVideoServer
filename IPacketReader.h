#ifndef IPACKETREADER_H
#define IPACKETREADER_H
#include "helper.h"
#include <boost/lockfree/spsc_queue.hpp>
#include "log.h"

class IPacketReader
{
public:

    AVPacket* takePacket()
    {
        try {
            if (m_packets.read_available() > 0)
            {
                AVPacket *pkt = m_packets.front();
                m_packets.pop();
                return pkt;
            }
        } catch (std::exception *e) {
            LOG->warn("Failed take pkt {}", e->what());
        }
        return nullptr;
    }
    void flushPacket(AVPacket *pkt)
    {
        try {
            if (m_packets.write_available() == 0) {
                av_packet_unref(m_packets.front());
                m_packets.pop();
            }
            if (m_packets.write_available() > 0) {
                AVPacket* newp = av_packet_alloc();
                newp->pts = pkt->pts;
                newp->dts = pkt->dts;
                av_packet_ref(newp, pkt);
                m_packets.push(newp);
            }
        } catch (std::exception* e) {
            LOG->warn("Failed flush pkt {}", e->what());
        }
    }

    virtual void onCreateStream(AVStream *stream) = 0;

private:
    boost::lockfree::spsc_queue<AVPacket*, boost::lockfree::capacity<10>> m_packets;
};

#endif // IPACKETREADER_H
