#pragma once

#include <memory>

#include "context/context.h"
#include "http_context.h"
#include "http_json_model.h"

namespace http {

class IHandler {
 protected:
  IHandler() = default;

 public:
  virtual ~IHandler() {}

  virtual std::string handle(void*) = 0;
};

template <typename Req, typename Rsp>
class PostHandler : public IHandler {
 public:
  using Func = void (*)(context::Context&, Req&, Rsp&);

  PostHandler(Func handler) : handler_(handler) {}

  virtual std::string handle(void* arg) {
    context::Context* ctx = (context::Context*)arg;
    std::unique_ptr<Req> req = std::make_unique<Req>();
    const auto& body = HttpContext::http_request(*ctx).body();
    if (json::unmarshal(body.begin, body.end, *req) !=
        nullptr) {
      // send request error
      return "";
    }
    std::unique_ptr<Rsp> rsp = std::make_unique<Rsp>();
    handler_(*ctx, *req, *rsp);
    return json::marshal(std::move(*rsp));
  }

 private:
  Func handler_;
};

template <typename Rsp>
class GetHandler : IHandler {
 public:
  using Func = void (*)(context::Context&, Rsp&);

  GetHandler(Func handler) : handler_(handler) {}

  virtual std::string handle(void* arg) {
    context::Context* ctx = (context::Context*)arg;
    std::unique_ptr<Rsp> rsp = std::make_unique<Rsp>();
    handler_(*ctx, *rsp);
    return json::marshal(std::move(*rsp));
  }

 private:
  Func handler_;
};

}  // namespace http