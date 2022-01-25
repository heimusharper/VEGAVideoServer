#include "FFPlayerInstance.h"

FFPlayerInstance::FFPlayerInstance(const std::string &address,
                                   std::function<bool (AVStream *)> create) :
    m_address(address),
    m_fnCreate(create)
{
    m_stop.store(false);
    m_mainThread = new std::thread(&FFPlayerInstance::run, this);
}

FFPlayerInstance::~FFPlayerInstance()
{
    m_stop.store(true);
    if (m_mainThread->joinable())
        m_mainThread->join();
    delete m_mainThread;
}

void FFPlayerInstance::takePackets(std::queue<AVPacket *> &pkt)
{
    try {
        while (m_packets.read_available() > 0)
        {
            pkt.push(m_packets.front());
            m_packets.pop();
        }
    } catch (std::exception *e) {
    }
}

void FFPlayerInstance::run()
{
    bool reloadStream = true;

    AVFormatContext *input_format_ctx = nullptr;

    AVDictionary *options = nullptr;
    //av_dict_set(&options, "fflags", "nobuffer",0);
    //av_dict_set(&options, "threads", "auto",0);

    int videoStreamIndex = -1;

    while (!m_stop.load())
    {
        if (reloadStream) {
            if (input_format_ctx)
                avformat_free_context(input_format_ctx);
            input_format_ctx = avformat_alloc_context();
            int err = avformat_open_input(&input_format_ctx, m_address.c_str(), NULL, &options);
            if (err >= 0)
            {
                err = avformat_find_stream_info(input_format_ctx, NULL);
                if (err >= 0) {
                    for (uint32_t i = 0; i < input_format_ctx->nb_streams; i++) {
                        if (input_format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                            videoStreamIndex = (int)i;
                            reloadStream = false; // done stream
                            if (m_fnCreate)
                                m_fnCreate(input_format_ctx->streams[i]);
                            break;
                        }
                    }
                    continue;
                } else
                    std::cout << "failed find stream info, error " << AVHelper::av2str(err);
            } else
                std::cout << "failed open input, error " << AVHelper::av2str(err);

            avformat_free_context(input_format_ctx);
            usleep(1000000);
        } else {
            AVPacket *pkt = av_packet_alloc();
            int err = av_read_frame(input_format_ctx, pkt);
            if (err >= 0 && pkt->stream_index == videoStreamIndex && m_packets.write_available())
                m_packets.push(pkt);
            else
                av_packet_free(&pkt);

            usleep(1000);
        }
    }
    if (input_format_ctx)
        avformat_free_context(input_format_ctx);
}
