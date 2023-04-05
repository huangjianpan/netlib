#include "http_server.h"

#include "http_request.h"
#include "log/log.h"
#include "utils/defer.hpp"
#include "utils/fmt.h"

namespace http {
HttpServer::HttpServer(in_port_t port)
    : svr_(port, HttpServer::new_connection_handler, this), pool_(1024) {}

HttpServer::~HttpServer() {
  for (auto& p : handlers_) {
    delete p.second;
  }
}

void HttpServer::start() { svr_.start(); }

void HttpServer::new_connection_handler(net::Connection* conn, void* arg) {
  LOG_INFO(name, utils::fmt::sprintf("new connection, fd = %d, addr = %s", conn->fd_operator().fd(), conn->address().ip()));
  conn->set_data(arg);
  conn->set_input_handler([](net::Connection* conn) -> void {
    HttpServer* svr = (HttpServer*)conn->data();
    HttpContext* context = svr->pool_.get();
    if (context == nullptr) {
      LOG_WARN(name, "get http context from pool is nullptr");
      // send 500
      conn->close();
    }

    // set defer for reuse HttpContext*
    auto f = [svr, context]() -> void {
      svr->pool_.put(context);
    };
    utils::Defer<decltype(f)> defer(std::move(f));

    // reset context: init log id and http request
    if (!context->reset(conn->input_buffer().begin(),
                        conn->input_buffer().size())) {
      LOG_WARN(name,
               utils::fmt::sprintf(
                   "request data parse failed, fd = %d, data = %s",
                   conn->fd_operator().fd(), conn->input_buffer().begin()));
      conn->close();
      return;
    }

    // check url
    context::Context* ctx = static_cast<context::Context*>(context);
    std::string url = HttpContext::http_request(*ctx).url().to_string();

    auto p = svr->handlers_.find(url);
    if (p == svr->handlers_.end()) {
      // send 400
      conn->close();
      return;
    }

    // call handler
    auto data = p->second->handle(conn, ctx);
    
  });
}

}  // namespace http
