#pragma once

#include <cstdlib>
#include <string>
#include <unordered_map>

// http request format:
// method url protocol/version\r\n    ===>    request line
// key1:value1\r\n
// key2:value2\r\n                    ===>    request header
// key3:value3\r\n
// \r\n
// body                               ===>    request body

namespace http {

class HttpRequest {
 private:
  class Parser {
    static constexpr char CR = '\r';
    static constexpr char LF = '\n';
    static constexpr char BLANK = ' ';
    static constexpr char TAB = '\t';

    static constexpr const char* ErrMsg[] = {"parse request line failed",
                                             "parse request headers failed",
                                             "parse request body failed"};

    static const char* skip_blank(const char* begin, const char* end) {
      while (begin != end && (*begin == BLANK || *begin == TAB)) {
        ++begin;
      }
      return begin;
    }

    static const char* find_blank(const char* begin, const char* end) {
      while (begin != end) {
        if (*begin == BLANK || *begin == TAB) {
          break;
        }
        ++begin;
      }
      return begin;
    }

    static const char* find(const char* begin, const char* end, char target) {
      while (begin != end) {
        if (*begin == target) {
          break;
        }
        ++begin;
      }
      return begin;
    }

    static bool is_upper(char ch) { return 'A' <= ch && ch <= 'Z'; }

   public:
    Parser(const char* first, const char* last)
        : pos(first), end(last), errmsg(nullptr) {}

    void parse_request(HttpRequest& req);

    void parse_request_line(HttpRequest& req);

    void parse_request_headers(HttpRequest& req);

    void parse_request_body(HttpRequest& req);

   private:
    const char* pos;  // current parse position
    const char* end;
    const char* errmsg;
  };

 public:
  struct Pointer {
    const char* begin;
    const char* end;

    Pointer() : begin(nullptr), end(nullptr) {}

    Pointer(const char* first, const char* last) : begin(first), end(last) {}

    void reset() {
      begin = nullptr;
      end = nullptr;
    }

    std::string to_string() const {
      if (begin == nullptr) {
        return "";
      }
      return std::string(begin, end);
    }
  };

  using KVs = std::unordered_map<std::string, Pointer>;

  HttpRequest() {}

  // please call errmsg() to check whether is success.
  HttpRequest(const char* data, size_t size) {
    Parser(data, data + size).parse_request(*this);
  }

  HttpRequest(HttpRequest&& rhs)
      : errmsg_(rhs.errmsg_),
        method_(rhs.method_),
        url_(rhs.url_),
        protocol_(rhs.protocol_),
        version_(rhs.version_),
        body_(rhs.body_),
        headers_(std::move(rhs.headers_)) {}

  HttpRequest& operator=(HttpRequest&& rhs) {
    errmsg_ = rhs.errmsg_;
    method_ = rhs.method_;
    url_ = rhs.url_;
    protocol_ = rhs.protocol_;
    version_ = rhs.version_;
    body_ = rhs.body_;
    headers_ = std::move(rhs.headers_);
    return *this;
  }

  const char* errmsg() const { return errmsg_; }

  const Pointer& method() const { return method_; }

  const Pointer& url() const { return url_; }

  const Pointer& protocol() const { return protocol_; }

  const Pointer& version() const { return version_; }

  const KVs& headers() const { return headers_; }

  const KVs& request_params() const { return request_params_; }

  const Pointer& body() const { return body_; }

  // please call errmsg() to check whether is success.
  void parse(const char* data, size_t size) {
    errmsg_ = nullptr;
    method_.reset();
    url_.reset();
    protocol_.reset();
    version_.reset();
    body_.reset();
    request_params_.clear();
    headers_.clear();

    Parser(data, data + size).parse_request(*this);
  }

 private:
  const char* errmsg_;
  Pointer method_;
  Pointer url_;
  Pointer protocol_;
  Pointer version_;
  Pointer body_;
  KVs request_params_;
  KVs headers_;
};
}  // namespace http
