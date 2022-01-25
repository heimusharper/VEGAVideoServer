#ifndef FFIMAGEFRAME_H
#define FFIMAGEFRAME_H

#include "helper.h"
class FFImageFrame
{
public:
    FFImageFrame(const AVFrame *frame);
    FFImageFrame(const FFImageFrame &d);

    ~FFImageFrame();

    AVFrame *frame() const;

private:
    AVFrame *m_frame = nullptr;
};

#endif // FFIMAGEFRAME_H
