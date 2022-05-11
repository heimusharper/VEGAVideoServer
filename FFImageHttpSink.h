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

    static FFImageHttpSink &instance();

    void create(const std::string& str, bool sync, int w, int h,
                const std::string &preset, const std::string &tune, int quality);

    Image *getImage();

private:
    FFImageHttpSink();

private:
    // источник фреймов видео
    FFJPEGEncoderInstance *m_sink = nullptr;
};

#endif // FFIMAGEHTTPSINK_H
