#pragma once

#include <unordered_map>

#include "context/context.h"
#include "handler.h"
#include "http_context.h"
#include "http_json_model.h"
#include "net/net.h"
#include "utils/pool.hpp"

namespace http {
class HttpServer {
 public:
  using HandlerMap = std::unordered_map<std::string, IHandler*>;

  HttpServer(in_port_t port);

  ~HttpServer();

  void start();

  template <typename Req, typename Rsp>
  void POST(const char* route, void (*handler)(context::Context&, Req&, Rsp&)) {
    std::string r = route;
    // check route whether is duplicate registration.
    if (handlers_.find(r) != handlers_.end()) {
      printf("HttpServer: Duplicate register route %s\n", route);
      abort();
    }
    // check Req and Rsp can json marshal and unmarshal
    if (json::marshal(Req{}) == "" || json::marshal(Rsp{}) == "") {
      printf("HttpServer: register post handler failed, please gen json code. route = %s\n",
             route);
      abort();
    }
    IHandler* h = new PostHandler<Req, Rsp>(handler);
    handlers_[r] = h;
  }

  template <typename Rsp>
  void GET(const char* route, void (*handler)(context::Context&, Rsp&)) {
    std::string r = route;
    // check route whether is duplicate registration.
    if (handlers_.find(r) != handlers_.end()) {
      printf("HttpServer: Duplicate register route %s\n", route);
      abort();
    }
    // check Req and Rsp can json marshal and unmarshal
    if (json::marshal(Rsp{}) == "") {
      printf("register get handler failed, please gen json code. route = %s\n",
             route);
      abort();
    }
    IHandler* h = new GetHandler<Rsp>(handler);
    handlers_[r] = h;
  }

 private:
  static void new_connection_handler(net::Connection*, void*);

  static constexpr const char* name = "HttpServer";

  net::Server svr_;
  HandlerMap handlers_;
  utils::Pool<HttpContext> pool_;
};
}  // namespace http
