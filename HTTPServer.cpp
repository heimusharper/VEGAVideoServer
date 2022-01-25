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
              evhttp_add_header(outhead, "Content-Type", "image/jpg");
              auto *OutBuf = evhttp_request_get_output_buffer(req);
              if (!OutBuf)
                return;
              evbuffer_add(OutBuf, image->image, image->size);
              evhttp_send_reply(req, HTTP_OK, "", OutBuf);
              delete image;
          } else {
              auto *OutBuf = evhttp_request_get_output_buffer(req);
              if (!OutBuf)
                return;
              // evbuffer_add_printf(OutBuf, "<html><body><center><h1>Image doesnt not found</h1></center></body></html>");
              evhttp_send_reply(req, HTTP_NOCONTENT, "", OutBuf);
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
