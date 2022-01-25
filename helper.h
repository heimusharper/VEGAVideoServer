#ifndef HELPER_H
#define HELPER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/packet.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include <string>
struct AVHelper {

    static std::string av2str(int err) {
        char s[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(err, s, AV_ERROR_MAX_STRING_SIZE);
        return std::string(s, AV_ERROR_MAX_STRING_SIZE);
    }
};
#endif // HELPER_H
