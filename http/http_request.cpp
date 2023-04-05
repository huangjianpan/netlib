#include "http_request.h"

namespace http {

void HttpRequest::Parser::parse_request(HttpRequest& req) {
  parse_request_line(req);
  parse_request_headers(req);
  parse_request_body(req);
  req.errmsg_ = errmsg;
}

void HttpRequest::Parser::parse_request_line(HttpRequest& req) {
  // method
  const char* p = skip_blank(pos, end);
  const char* q = nullptr;
  const char* w = nullptr;
  if (p == end || !is_upper(*p)) {
    goto error;
  }
  q = find_blank(p, end);
  req.method_.begin = p;
  req.method_.end = q;

  // url and request params
  p = skip_blank(q, end);
  if (p == end || *p == CR || *p == LF) {
    goto error;
  }
  for (q = p; q != end; ++q) {
    if (*q == BLANK || *q == TAB || *q == '?') {
      break;
    }
  }
  if (q == end) {
    goto error;
  }
  req.url_.begin = p;
  req.url_.end = q;
  if (*q == '?') {  // parse request params
    p = q + 1;
    for (;;) {
      if (p == end || *p == CR || *p == LF) {
        goto error;
      }
      if (*p == BLANK || *p == TAB) {  // success
        q = p;
        break;
      }
      for (q = p; q != end && *q != '='; ++q)
        ;
      if (q == end || p == q) {
        goto error;
      }
      for (w = q + 1; w != end; ++w) {
        if (*w == BLANK || *w == TAB || *w == '&') {
          break;
        }
      }
      if (w == end || w == q + 1) {
        goto error;
      }
      // abc=ABCX, *p = a, *q = '=', *w = X
      req.request_params_.emplace(std::string(p, q), Pointer{q + 1, w});
      p = *w == '&' ? w + 1 : w;
    }
  }

  // protocol
  p = skip_blank(q, end);
  if (p == end || *p != 'H') {
    goto error;
  }
  q = find(p, end, '/');
  if (q == end) {
    goto error;
  }
  req.protocol_.begin = p;
  req.protocol_.end = q;

  // version
  p = q + 1;
  if (p == end || !isdigit(*p)) {
    goto error;
  }
  q = find(p, end, CR);
  if (q == end) {
    goto error;
  }
  req.version_.begin = p;
  req.version_.end = q;

  // CRLF
  p = q + 1;
  if (p == end || *p != LF) {
    goto error;
  }
  pos = p + 1;
  return;

error:
  errmsg = ErrMsg[0];
  return;
}

void HttpRequest::Parser::parse_request_headers(HttpRequest& req) {
  if (errmsg != nullptr) {
    return;
  }

  const char* p = pos;
  const char* q = nullptr;
  for (;;) {
    p = skip_blank(p, end);
    if (p == end) {
      errmsg = ErrMsg[1];
      return;
    }

    // blank line
    if (*p == CR) {
      if (p + 1 != end && *(p + 1) == LF) {
        pos = p + 2;
        break;
      } else {
        errmsg = ErrMsg[1];
        return;
      }
    }

    // key
    if (*p == CR || *p == LF) {
      errmsg = ErrMsg[1];
      return;
    }
    q = find(p + 1, end, ':');
    if (q == end) {
      errmsg = ErrMsg[1];
      return;
    }
    std::string key;
    for (const char* w = q - 1;; --w) {
      if (*w != BLANK && *w != TAB) {
        key = std::string(p, w + 1);
        break;
      }
    }

    // value
    p = skip_blank(q + 1, end);
    if (p == end || *p == CR || *p == LF) {
      errmsg = ErrMsg[1];
      return;
    }
    q = find(p + 1, end, CR);
    if (q == end) {
      errmsg = ErrMsg[1];
      return;
    }
    for (const char* w = q - 1;; --w) {
      if (*w != ' ' && *w != '\t') {
        req.headers_.emplace(std::move(key), Pointer{p, w + 1});
        break;
      }
    }

    // CRLF
    p = q + 1;
    if (p == end || (*p != LF)) {
      errmsg = ErrMsg[1];
      return;
    }
    ++p;
  }
}

void HttpRequest::Parser::parse_request_body(HttpRequest& req) {
  if (errmsg != nullptr) {
    return;
  }
  req.body_.begin = pos;
  req.body_.end = end;
}

}  // namespace http