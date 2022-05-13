#include "FFMpegFileSave.h"

FFMpegFileSave::FFMpegFileSave(const std::string &suffix)
    : IPacketReader()
    , m_suffix(suffix)
{
    m_stop.store(false);
    m_mainThread = new std::thread(&FFMpegFileSave::run, this);
}

FFMpegFileSave::~FFMpegFileSave()
{
    m_stop.store(true);
    if (m_mainThread->joinable())
        m_mainThread->join();
    delete m_mainThread;
}

void FFMpegFileSave::onCreateStream(AVStream *stream)
{
    m_contextLock.lock();
    m_sourceStream = stream;
    m_contextLock.unlock();
}

void FFMpegFileSave::run()
{
    AVStream *stream = nullptr;
    int64_t nowTime = 0;
    while(!m_stop.load())
    {
        int sleepTime = 10;
        if (!m_context) {
            sleepTime = 1000000;
            nowTime = MavContext::instance().time() / 1000000.;
            m_contextLock.lock();
            if (nowTime > 0 && m_sourceStream) {
                struct tm  tstruct;
                time_t nowTime_t = nowTime;
                memcpy(&tstruct, localtime(&nowTime_t), sizeof(struct tm));
                char *buf = new char[80];
                std::strftime(buf, 80, "%Y_%m_%d_%H_%M_%S", &tstruct);
                std::string filename = m_suffix + "_" + std::string((const char*)buf) + ".mp4";
                delete [] buf;
                std::cout << "Create file " << filename << std::endl;
                // create stream
                if (int err = avformat_alloc_output_context2(&m_context, NULL, NULL, filename.c_str()) < 0)
                {
                    std::cerr << "Failed create output context " << AVHelper::av2str(err) << std::endl;
                    m_context = nullptr;
                } else {
                    m_context->video_codec_id = m_sourceStream->codecpar->codec_id;
                    stream = avformat_new_stream(m_context, NULL);
                    if (!stream)
                    {
                        std::cerr << "Failed create stream" << std::endl;
                        avformat_free_context(m_context);
                        m_context = nullptr;
                    } else {
                        stream->discard = AVDISCARD_DEFAULT;
                        stream->time_base = m_sourceStream->time_base;
                        stream->codecpar->level = 42;
                        stream->codecpar->profile = FF_PROFILE_H264_HIGH;
                        stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
                        if (int err = avcodec_parameters_copy(stream->codecpar, m_sourceStream->codecpar))
                        {
                            std::cerr << "Failed to copy parameters to context " << std::endl;
                            avformat_free_context(m_context);
                            m_context = nullptr;
                        } else {
                            // open io
                            if (int err = avio_open(&m_context->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0)
                            {
                                std::cerr << "Failed open io context " << filename << std::endl;
                                avformat_free_context(m_context);
                                m_context = nullptr;
                            } else {
                                AVDictionary *dict = nullptr;
                                av_dict_set(&dict, "movflags", "faststart", 0);
                                av_dict_set(&dict, "movflags", "frag_keyframe+empty_moov", 0);

                                if (int err = avformat_write_header(m_context, &dict) < 0)
                                {
                                    std::cerr << "Failed write header " << filename << std::endl;
                                    avformat_free_context(m_context);
                                    m_context = nullptr;
                                } else {

                                }
                            }
                        }
                    }
                }
            }
            m_contextLock.unlock();
        } else {
            m_contextLock.lock();
            AVPacket *pkt = takePacket();
            if (pkt)
            {
                if (int err = av_interleaved_write_frame(m_context, pkt) < 0)
                {
                    // failure
                }
                av_packet_unref(pkt);
            }
            m_contextLock.unlock();
        }
        usleep(sleepTime);
    }
    if (m_context)
    {
        av_write_trailer(m_context);
        avformat_free_context(m_context);
        m_context = nullptr;
    }
}
