#ifndef FFIMAGEHTTPSINK_H
#define FFIMAGEHTTPSINK_H

#include "FFJPEGEncoderInstance.h"
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

class FFImageHttpSink
{
public:
    ~FFImageHttpSink();

    void init(FFJPEGEncoderInstance *sink);

    static FFImageHttpSink &instance();

    Image *getImage();

private:
    // источник фреймов видео
    FFJPEGEncoderInstance *m_sink = nullptr;
    FFImageHttpSink();
};

#endif // FFIMAGEHTTPSINK_H
