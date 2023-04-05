#pragma once

#include <memory>

#include "context/context.h"
#include "http_context.h"
#include "http_json_model.h"
#include "net/connection.h"

namespace http {

class IHandler {
 protected:
  IHandler() = default;

 public:
  virtual ~IHandler() {}

  virtual void handle(net::Connection*, void*) = 0;
};

template <typename Req, typename Rsp>
class PostHandler : IHandler {
 public:
  using Func = void (*)(context::Context&, Req&, Rsp&);

  PostHandler(Func handler) : handler_(handler) {}

  virtual void handle(net::Connection* conn, void* arg) {
    context::Context* ctx = (context::Context*)arg;
    std::unique_ptr<Req> req = std::make_unique<Req>();
    if (json::unmarshal(HttpContext::http_request(*ctx).body().begin, *req) !=
        nullptr) {
      // send request error
      return;
    }
    std::unique_ptr<Rsp> rsp = std::make_unique<Rsp>();
    handler_(*ctx, *req, *rsp);
    std::string j = json::marshal(std::move(*rsp));
    ssize_t n = write(conn->fd_operator().fd(), j.c_str(), j.size());
  }

 private:
  Func handler_;
};

template <typename Rsp>
class GetHandler : IHandler {
 public:
  using Func = void (*)(context::Context&, Rsp&);

  GetHandler(Func handler) : handler_(handler) {}

  virtual void handle(net::Connection* conn, void* arg) {
    context::Context* ctx = (context::Context*)arg;
    std::unique_ptr<Rsp> rsp = std::make_unique<Rsp>();
    handler_(*ctx, *rsp);
    std::string j = json::marshal(std::move(*rsp));
    ssize_t n = write(conn->fd_operator().fd(), j.c_str(), j.size());
  }

 private:
  Func handler_;
};

}  // namespace http