#ifndef JPEGHTTPSINK_H
#define JPEGHTTPSINK_H

#include "EncoderJPEG.h"
#include <thread>
#include "helper.h"
#include <exiv2/exiv2.hpp>
#include "MavContext.h"
#include <HeightSource.h>

struct Image {
    char *image;
    long size = 0;

    ~Image() {
        delete [] image;
    }
};

class JPEGHttpSink
{
public:
    ~JPEGHttpSink();

    void init(EncoderJPEG *sink);

    static JPEGHttpSink &instance();

    Image *getImage();

private:
    // источник фреймов видео
    EncoderJPEG *m_sink = nullptr;
    JPEGHttpSink();
};

#endif // FFIMAGEHTTPSINK_H
