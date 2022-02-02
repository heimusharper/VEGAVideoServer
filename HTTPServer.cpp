#include "HTTPServer.h"

HTTPServer::HTTPServer(const HTTPServerConfiguration &config) :
    m_config(config)
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
          Image *image = FFImageHttpSink::instance().getImage();
          if (image) {
              evkeyvalq* outhead = evhttp_request_get_output_headers(req);

              uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

              evhttp_add_header(outhead, "X-TimeStamp", std::to_string(time).c_str());
              evhttp_add_header(outhead, "Content-Type", "image/jpg");
              auto *OutBuf = evhttp_request_get_output_buffer(req);
              if (!OutBuf)
                return;
              evbuffer_add(OutBuf, image->image, image->size);
              evhttp_send_reply(req, HTTP_OK, "", OutBuf);
              delete image;
          } else {
              evhttp_send_error(req, HTTP_NOCONTENT, NULL);
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
