#include <iostream>
#include "HTTPServer.h"
#include <algorithm>
#include "MavUDP.h"

char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

void showHelp()
{
    std::cout << "VEGAVideoServer" << std::endl;
    std::cout << " -h -- this help" << std::endl;
    std::cout << " --src [URL] -- source video URL [\"http://distribution.bbb3d.renderfarming.net/video/mp4/bbb_sunflower_1080p_60fps_normal.mp4\"]" << std::endl;
    std::cout << " --http-host [127.0.0.1] -- http server host" << std::endl;
    std::cout << " --http-port [8088] -- http server port" << std::endl;
    std::cout << " --mav-host [127.0.0.1] -- mavproxy udp host" << std::endl;
    std::cout << " --mav-port [14550] -- mavproxy udp port" << std::endl;
    std::cout << " --mav-gcs [127] -- GCS ID [1-254]" << std::endl;
}

int main(int argc, char *argv[])
{

    if (cmdOptionExists(argv, argv + argc, "-h"))
    {
        showHelp();
        return 0;
    }

    if (!cmdOptionExists(argv, argv + argc, "--src"))
    {
        showHelp();
        return 0;
    }

    // video source
    //
    std::string video = std::string(getCmdOption(argv, argv + argc, "--src")); // "http://distribution.bbb3d.renderfarming.net/video/mp4/bbb_sunflower_1080p_60fps_normal.mp4"
    FFImageHttpSink::instance().create(video);

    // MavProxy
    //
    std::string mav_host = "127.0.0.1";
    uint16_t mav_port = 14550;
    uint8_t mav_gcs = 127;
    if (cmdOptionExists(argv, argv + argc, "--mav-host"))
        mav_host = std::string(getCmdOption(argv, argv + argc, "--mav-host"));
    if (cmdOptionExists(argv, argv + argc, "--mav-port"))
        mav_port = std::stoi(std::string(getCmdOption(argv, argv + argc, "--mav-port")));
    if (cmdOptionExists(argv, argv + argc, "--mav-gcs"))
        mav_gcs = std::stoi(std::string(getCmdOption(argv, argv + argc, "--mav-gcs")));
    MavUDP udp(mav_host, mav_port, mav_gcs);

    // HTTP server
    //
    std::string http_host = "127.0.0.1";
    uint16_t http_port = 8088;
    if (cmdOptionExists(argv, argv + argc, "--http-host"))
        http_host = std::string(getCmdOption(argv, argv + argc, "--http-host"));
    if (cmdOptionExists(argv, argv + argc, "--http-port"))
        http_port = std::stoi(std::string(getCmdOption(argv, argv + argc, "--http-port")));
    HTTPServerConfiguration config;
    config.address = http_host;
    config.port = http_port;

    HTTPServer server (config);
    server.start();
    return 0;
}
