#include <iostream>
#include "HTTPServer.h"

using namespace std;

int main()
{

    // FFImageHttpSink::instance().create("rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov");
    FFImageHttpSink::instance().create("http://distribution.bbb3d.renderfarming.net/video/mp4/bbb_sunflower_1080p_60fps_normal.mp4");
    HTTPServerConfiguration config;

    HTTPServer server (config);
    server.start();
    return 0;
}
