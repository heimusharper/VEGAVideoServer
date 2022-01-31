#ifndef FFIMAGEHTTPSINK_H
#define FFIMAGEHTTPSINK_H

#include "FFDecoderInstance.h"
#include "FFH264DecoderInstance.h"
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

    static FFImageHttpSink &instance();

    void create(const std::string &str);

    Image *getImage();

private:
    FFImageHttpSink();

private:
    // источник фреймов видео
    FFDecoderInstance *m_sink = nullptr;
};

#endif // FFIMAGEHTTPSINK_H
