#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include "FFImageHttpSink.h"
#include <chrono>
#include <thread>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <evhttp.h>
#include <chrono>
#include <string>

struct HTTPServerConfiguration
{
    std::string address = "127.0.0.1";
    uint16_t port = 8088;

    HTTPServerConfiguration() = default;
};


class HTTPServer
{
public:
    HTTPServer(const HTTPServerConfiguration &config);
    ~HTTPServer();

    int start();

private:


    HTTPServerConfiguration m_config;
    std::atomic_bool m_run;


};

#endif // HTTPSERVER_H
