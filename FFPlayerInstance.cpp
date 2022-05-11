#include "FFPlayerInstance.h"

FFPlayerInstance::FFPlayerInstance()
{
}

FFPlayerInstance::~FFPlayerInstance()
{
    m_stop.store(true);
    if (m_mainThread->joinable())
        m_mainThread->join();
    delete m_mainThread;
}

void FFPlayerInstance::start(const std::string &address, bool sync)
{
    m_sync = sync;
    m_address = address;
    m_stop.store(false);
    m_mainThread = new std::thread(&FFPlayerInstance::run, this);
}

void FFPlayerInstance::run()
{
    bool reloadStream = true;

    AVFormatContext *input_format_ctx = nullptr;

    AVDictionary *options = nullptr;
    //av_dict_set(&options, "fflags", "nobuffer",0);

    //! TODO без этого не инициализирует энкодер для jpeg
    av_register_all();
    avcodec_register_all();
    av_dict_set(&options, "threads", "auto",0);

    int videoStreamIndex = -1;
    int frameTime = -1;

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
                            for (auto x : m_readers)
                                x->onCreateStream(input_format_ctx->streams[i]);
                            break;
                        }
                    }
                    continue;
                } else
                    std::cout << "failed find stream info, error " << AVHelper::av2str(err) << std::endl << std::flush;
            } else
                std::cout << "failed open input, error " << AVHelper::av2str(err) << std::endl << std::flush;

            avformat_free_context(input_format_ctx);
            usleep(1000000);
        } else {
            boost::chrono::high_resolution_clock::time_point start;
            if (m_sync)
                start = boost::chrono::high_resolution_clock::now();
            AVPacket *pkt = av_packet_alloc();
            int sleepTime = 10;
            int err = av_read_frame(input_format_ctx, pkt);
            if (err >= 0 && pkt->stream_index == videoStreamIndex) {
                if (m_sync && pkt->pts != AV_NOPTS_VALUE) {
                    AVRational msecondbase = { 1, 1000 };
                    //int f_number = pkt->pts;
                    int f_time = av_rescale_q(pkt->pts, input_format_ctx->streams[videoStreamIndex]->time_base, msecondbase);
                    if (frameTime > 0)
                        sleepTime = (f_time - frameTime) * 1000;
                    frameTime = f_time;
                    auto stop = boost::chrono::high_resolution_clock::now();
                    int workTime = boost::chrono::duration_cast<boost::chrono::microseconds>(stop - start).count();
                    sleepTime -= workTime;
                } else {
                }
                usleep((sleepTime > 0) ? sleepTime : 10);
                for (auto x : m_readers)
                    x->flushPacket(pkt);
            }
            av_packet_unref(pkt);
            usleep((sleepTime > 0) ? sleepTime : 10);
        }
    }
    if (input_format_ctx)
        avformat_free_context(input_format_ctx);
}
