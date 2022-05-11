#ifndef IPACKETREADER_H
#define IPACKETREADER_H
#include "FFImageFrame.h"

class IPacketReader
{
public:

    AVPacket* takePacket()
    {
        try {
            while (m_packets.read_available() > 0)
            {
                AVPacket *pkt = m_packets.front();
                m_packets.pop();
                return pkt;
            }
        } catch (std::exception *e) {
        }
        return nullptr;
    }
    void flushPacket(AVPacket *pkt)
    {
        if (m_packets.write_available()) {
            AVPacket *newp = av_packet_alloc();
            av_packet_ref(newp, pkt);
            m_packets.push(pkt);
        }
    }

    virtual void onCreateStream(AVStream *stream) = 0;

private:
    boost::lockfree::spsc_queue<AVPacket*, boost::lockfree::capacity<100>> m_packets;
};

#endif // IPACKETREADER_H
