#pragma once

#include <string>

#include "context/context.h"
#include "http_context.h"
#include "utils/fmt.h"

namespace http {
class HttpResponse {
 public:
  static constexpr const char* Html404 =
      "<html>"
      "<head><title>404 Not Found</title></head>"
      "<body><center><h1>404 Not Found</h1></center></body>"
      "</html>";

  static constexpr const char* Html400 =
      "<html>"
      "<head><title>400 Bad Request</title></head>"
      "<body><center><h1>400 Bad Request</h1></center></body>"
      "</html>";

  static constexpr const char* Html500 =
      "<html>"
      "<head><title>500 Internal Server Error</title></head>"
      "<body><center><h1>500 Internal Server Error</h1></center></body>"
      "</html>";

  static std::string response(context::Context* ctx, int status_code,
                              const char* content_type, const char* body,
                              size_t body_length) {
    bool close = status_code != 200;
    return utils::fmt::sprintf(
        "HTTP/1.1 %d %s\r\n"
        "Content-Encoding: null\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %lu\r\n"
        "Connection: %s\r\n"
        "logid: %s\r\n"
        "\r\n"
        "%s",
        status_code, status(status_code), content_type, body_length,
        close ? "close" : "keep-alive",
        ctx ? HttpContext::log_id(*ctx).c_str() : "null", body);
  }

  static const char* status(int status_code) {
    switch (status_code) {
      case 200:
        return "OK";
      case 404:
        return "Not Found";
      case 400:
        return "Bad Request";
      case 500:
        return "Internal Server Error";
    }
    return "";
  }
};

}  // namespace http
