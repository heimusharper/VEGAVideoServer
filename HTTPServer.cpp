#include "HTTPServer.h"
#include <event.h>

HTTPServer::HTTPServer(const HTTPServerConfiguration &config)
    : m_config(config)
{
    m_run.store(true);
}

HTTPServer::~HTTPServer()
{
    m_run.store(false);
}

int HTTPServer::start()
{
    if (!event_init())
      {
        std::cerr << "Failed to init libevent." << std::endl;
        return -1;
      }
      std::unique_ptr<evhttp, decltype(&evhttp_free)> Server(evhttp_start(m_config.address.c_str(), m_config.port), &evhttp_free);
      if (!Server)
      {
        std::cerr << "Failed to init http server." << std::endl;
        return -1;
      }
      void (*OnReq)(evhttp_request *req, void *) = [] (evhttp_request *req, void *)
      {
          LOG->debug("On request image...");
          Image *image = JPEGHttpSink::instance().getImage();
          if (image) {
              uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

              evhttp_add_header(req->output_headers, "X-TimeStamp", std::to_string(time).c_str());
              evhttp_add_header(req->output_headers, "Content-Type", "image/jpg");
              // evbuffer_add(req->output_buffer, image->image, image->size);
              struct evbuffer *evb = evbuffer_new();
              evbuffer_add(evb, image->image, image->size);
              evhttp_send_reply(req, HTTP_OK, "OK", evb);
              evbuffer_free(evb);
              delete image;
          } else {
              LOG->warn("Failed get image, respond HTTP_NOCONTENT");
              evhttp_send_error(req, HTTP_NOCONTENT,  "Image stream not initialized");
          }
      };
      evhttp_set_gencb(Server.get(), OnReq, nullptr);
      if (event_dispatch() == -1)
      {
        std::cerr << "Failed to run messahe loop." << std::endl;
        return -1;
      }
      return 0;
}
