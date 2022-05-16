#ifndef HELPER_H
#define HELPER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavformat/version.h>
#include <libavutil/imgutils.h>
#include <libavutil/version.h>
#include <libswscale/swscale.h>
}
#include <string>
struct AVHelper {

    static std::string av2str(int err) {
        char s[AV_ERROR_MAX_STRING_SIZE];
        if (av_strerror(err, s, AV_ERROR_MAX_STRING_SIZE) == 0)
            return std::string(s, strlen(s));
        return std::string("undefined");
    }
};
#endif // HELPER_H
