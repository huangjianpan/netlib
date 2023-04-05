#pragma once

#include "context/context.h"
#include "http_request.h"
#include "log/log_id.h"

namespace http {

class HttpContext : public context::Context {
 public:
  HttpContext() = default;

  static logger::LogID& log_id(context::Context& ctx) {
    return (static_cast<HttpContext*>(&ctx))->log_id_;
  }

  static HttpRequest& http_request(context::Context& ctx) {
    return (static_cast<HttpContext*>(&ctx))->request_;
  }

  // reset log id and request.
  // raw is remote request data.
  bool reset(const char* raw, size_t size) {
    log_id_ = logger::LogID::generate();
    request_.parse(raw, size);
    return request_.errmsg() == nullptr;
  }

 private:
  logger::LogID log_id_;
  HttpRequest request_;
};
}  // namespace http
