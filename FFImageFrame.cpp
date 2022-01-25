#include "FFImageFrame.h"

FFImageFrame::FFImageFrame(const AVFrame *frame)
{
    m_frame = av_frame_alloc();
    //av_frame_copy(m_frame, frame);
    av_frame_ref(m_frame, frame);
}

FFImageFrame::FFImageFrame(const FFImageFrame &d)
{
    m_frame = av_frame_alloc();
    av_frame_ref(m_frame, d.frame());
    //av_frame_copy(m_frame, d.frame());
}

FFImageFrame::~FFImageFrame()
{
    av_frame_unref(m_frame);
}

AVFrame *FFImageFrame::frame() const
{
    return m_frame;
}
