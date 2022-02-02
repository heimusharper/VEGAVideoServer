#include <iostream>
#include "HTTPServer.h"
#include <algorithm>
#include "MavUDP.h"
#include <HeightSource.h>
#include <HeightSourceHGT1M.h>

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
    std::cout << " --cam-focal-length -- video camera focal length" << std::endl;
    std::cout << " --cam-crop-factor -- camera matrix crop factor" << std::endl;
    std::cout << " --cam-x-resolution -- camera X resolution px/cm" << std::endl;
    std::cout << " --cam-y-resolution -- camera Y resolution px/cm" << std::endl;
    std::cout << " --cam-orientation [1] -- camera orientation in EXIF orientation codes" << std::endl;
    std::cout << " --relief -- relief sources path" << std::endl;
    std::cout << " --sync -- sync video DTS/PTS if [src] is file" << std::endl;
    std::cout << " --out_width -- output frame width" << std::endl;
    std::cout << " --out_height -- output frame height" << std::endl;
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
        return -1;
    }

    // configure
    if (cmdOptionExists(argv, argv + argc, "--cam-focal-length"))
        MavContext::instance().setFocalLength(std::stof(std::string(getCmdOption(argv, argv + argc, "--cam-focal-length"))));
    else {
        std::cout << "missed --cam-focal-length param" << std::endl;
        return -1;
    }
    if (cmdOptionExists(argv, argv + argc, "--cam-crop-factor"))
        MavContext::instance().setCrop(std::stof(std::string(getCmdOption(argv, argv + argc, "--cam-crop-factor"))));
    else {
        std::cout << "missed --cam-crop-factor param" << std::endl;
        return -1;
    }
    if (cmdOptionExists(argv, argv + argc, "--cam-x-resolution") && cmdOptionExists(argv, argv + argc, "--cam-y-resolution")) {
        MavContext::instance().setXResolution(std::stof(std::string(getCmdOption(argv, argv + argc, "--cam-x-resolution"))));
        MavContext::instance().setYResolution(std::stof(std::string(getCmdOption(argv, argv + argc, "--cam-y-resolution"))));
    } else {
        std::cout << "missed --cam-[x,y]-resolution param" << std::endl;
        return -1;
    }
    if (cmdOptionExists(argv, argv + argc, "--cam-orientation"))
        MavContext::instance().setCamOrientationEXIF(
                    std::stof(std::string(getCmdOption(argv, argv + argc, "--cam-orientation"))));
    // relief
    if (cmdOptionExists(argv, argv + argc, "--relief"))
    {
        geo::HeightSourceHGT1M *hgt1 = new geo::HeightSourceHGT1M(
                    std::filesystem::path(std::string(getCmdOption(argv, argv + argc, "--relief"))));
        geo::HeightSource::instance().setInterface(hgt1);
    } else {
        std::cout << "missed --relief param" << std::endl;
        return -1;
    }

    // video
    //
    int videoWidth = 0;
    int videoHeight = 0;
    if (cmdOptionExists(argv, argv + argc, "--out_width"))
        videoWidth = std::stoi(std::string(getCmdOption(argv, argv + argc, "--out_width")));
    if (cmdOptionExists(argv, argv + argc, "--out_height"))
        videoHeight = std::stoi(std::string(getCmdOption(argv, argv + argc, "--out_height")));
    bool sync = cmdOptionExists(argv, argv + argc, "--sync");
    std::string video = std::string(getCmdOption(argv, argv + argc, "--src")); // "http://distribution.bbb3d.renderfarming.net/video/mp4/bbb_sunflower_1080p_60fps_normal.mp4"
    FFImageHttpSink::instance().create(video, sync, videoWidth, videoHeight);

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
