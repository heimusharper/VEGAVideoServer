#include <iostream>
#include "HTTPServer.h"
#include <algorithm>

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
    std::cout << " --host [127.0.0.1] -- http server host" << std::endl;
    std::cout << " --port [8088] -- http server port" << std::endl;
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

    std::string video = std::string(getCmdOption(argv, argv + argc, "--src")); // "http://distribution.bbb3d.renderfarming.net/video/mp4/bbb_sunflower_1080p_60fps_normal.mp4"
    std::string host = "127.0.0.1";
    uint16_t port = 8088;

    if (cmdOptionExists(argv, argv + argc, "--host"))
        host = std::string(getCmdOption(argv, argv + argc, "--host"));
    if (cmdOptionExists(argv, argv + argc, "--port"))
        port = std::stoi(std::string(getCmdOption(argv, argv + argc, "--port")));
    FFImageHttpSink::instance().create(video);
    HTTPServerConfiguration config;
    config.address = host;
    config.port = port;

    HTTPServer server (config);
    server.start();
    return 0;
}
