#include "json.h"

#include <cassert>
#include <cstring>

namespace json {

Json::Json(const Json& rhs) : type_(rhs.type_), data_(nullptr) {
  if (rhs.is_object()) {
    if (rhs.data_ != nullptr) {
      data_ =
          new (std::nothrow) ObjectData(*static_cast<ObjectData*>(rhs.data_));
    }
  } else if (rhs.is_string()) {
    if (rhs.data_ != nullptr) {
      data_ =
          new (std::nothrow) StringData(*static_cast<StringData*>(rhs.data_));
    }
  } else if (rhs.is_array()) {
    if (rhs.data_ != nullptr) {
      data_ = new (std::nothrow) ArrayData(*static_cast<ArrayData*>(rhs.data_));
    }
  } else {
    data_ = rhs.data_;  // data_ has the maximum length in the union.
  }
}

Json& Json::operator=(Json&& rhs) {
  if (this == &g_null_json_) {  // for don't modify g_null_json
    return *this;
  }
  Json j(std::move(rhs));
  std::swap(j.type_, type_);
  std::swap(j.data_, data_);
  return *this;
}

Json& Json::operator=(const Json& rhs) {
  if (this == &g_null_json_) {  // for don't modify g_null_json
    return *this;
  }
  Json j(rhs);
  std::swap(j.type_, type_);
  std::swap(j.data_, data_);
  return *this;
}

Json& Json::operator[](size_t i) {
  if (is_array()) {
    auto data = static_cast<ArrayData*>(data_);
    if (data != nullptr && i < data->size()) {
      return (*data)[i];
    }
  }
  return g_null_json_;
}

Json& Json::operator[](const std::string& key) {
  if (is_object()) {
    auto data = static_cast<ObjectData*>(data_);
    if (data != nullptr) {
      auto it = data->find(key);
      if (it != data->end()) {
        return it->second;
      }
    }
  }
  return g_null_json_;
}

bool Json::add_kv(std::string key, Json value) {
  if (value.is_null() || value.is_error()) {
    return false;
  }
  if (is_object()) {
    if (data_ == nullptr) {
      data_ = new (std::nothrow) ObjectData;
      if (data_ == nullptr) {
        return false;
      }
    }
    auto& data = *static_cast<ObjectData*>(data_);
    return data.emplace(std::move(key), std::move(value)).second;
  }
  return false;
}

bool Json::add_elem(Json value) {
  if (value.is_null() || value.is_error()) {
    return false;
  }
  if (is_array()) {
    if (data_ == nullptr) {
      data_ = new (std::nothrow) ArrayData;
      if (data_ == nullptr) {
        return false;
      }
    }
    auto& data = *static_cast<ArrayData*>(data_);
    data.emplace_back(std::move(value));
    return true;
  }
  return false;
}

char* Json::marshal(char* buffer) const {
  if (is_string()) {
    *buffer = '"';
    if (data_ != nullptr) {
      const auto& data = *static_cast<StringData*>(data_);
      memcpy(buffer + 1, data.c_str(), data.size());
      buffer = buffer + 1 + data.size();
    }
    *buffer = '"';
    return buffer + 1;
  } else if (is_integer()) {
    return buffer + snprintf(buffer, 32, "%Ld", i_);
  } else if (is_float()) {
    buffer += snprintf(buffer, 32, "%.15lf", f_) - 1;
    while (*buffer == '0') {
      --buffer;
    }
    return *buffer == '.' ? buffer + 2 : buffer + 1;
  } else if (is_object()) {
    *buffer = '{';
    ++buffer;
    if (data_ != nullptr) {
      const auto& data = *static_cast<ObjectData*>(data_);
      size_t cnt = data.size();
      for (const auto& kv : data) {
        *buffer = '"';
        memcpy(buffer + 1, kv.first.c_str(), kv.first.size());
        buffer = buffer + 1 + kv.first.size();
        *buffer = '"';
        ++buffer;
        *buffer = ':';
        buffer = kv.second.marshal(buffer + 1);
        if (--cnt > 0) {
          *buffer = ',';
          ++buffer;
        }
      }
    }
    *buffer = '}';
    return buffer + 1;
  } else if (is_array()) {
    *buffer = '[';
    ++buffer;
    if (data_ != nullptr) {
      const auto& data = *static_cast<ArrayData*>(data_);
      size_t cnt = data.size();
      for (const auto& j : data) {
        buffer = j.marshal(buffer);
        if (--cnt > 0) {
          *buffer = ',';
          ++buffer;
        }
      }
    }
    *buffer = ']';
    return buffer + 1;
  } else if (is_bool()) {
    if (b_) {
      memcpy(buffer, "true", 4);
      return buffer + 4;
    }
    memcpy(buffer, "false", 5);
    return buffer + 5;
  }
  return buffer;
}

size_t Json::marshal_count() const {
  size_t count = 0;
  if (is_string()) {
    count += 2 + (data_ == nullptr
                      ? 0
                      : static_cast<StringData*>(data_)->size());  // "xxx"
  } else if (is_integer() || is_float()) {
    count += 32;
  } else if (is_object()) {
    if (data_ != nullptr) {
      const auto& data = *static_cast<ObjectData*>(data_);
      count += (data.size() > 0 ? data.size() - 1 : 0);  // the number of ','
      for (const auto& kv : data) {
        count += 3 + kv.first.size() + kv.second.marshal_count();  // "xxx":xxx
      }
    }
    count += 2;  // {}
  } else if (is_array()) {
    if (data_ != nullptr) {
      const auto& data = *static_cast<ArrayData*>(data_);
      count += (data.size() > 0 ? data.size() - 1 : 0);  // the number of ','
      for (const auto& j : data) {
        count += j.marshal_count();
      }
    }
    count += 2;  // []
  } else if (is_bool()) {
    count += b_ ? 4 : 5;
  }
  return count;
}

Json Json::build(const char* begin, const char* end) {
  begin = tool::skip_blank(begin, end);
  if (begin == end) {
    return build_error(ErrMsg[0]);
  }
  if (*begin == '{') {
    Json j = build_object(begin, end, begin);
    if (j.is_error()) {
      return j;
    }
    if (tool::skip_blank(begin, end) != end) {
      return build_error(ErrMsg[0]);
    }
    return j;
  }
  if (*begin == '[') {
    Json j = build_array(begin, end, begin);
    if (j.is_error()) {
      return j;
    }
    if (tool::skip_blank(begin, end) != end) {
      return build_error(ErrMsg[0]);
    }
    return j;
  }
  return build_error(ErrMsg[0]);
}

Json Json::build(const char* begin, const char* end, const char*& next) {
  begin = tool::skip_blank(begin, end);
  if (begin == end) {
    return build_error(ErrMsg[0]);
  }
  if (*begin == '"') {
    return build_string(begin, end, next);
  }
  if (*begin == '-' || ('0' <= *begin && *begin <= '9')) {
    return build_numeric(begin, end, next);
  }
  if (*begin == '{') {
    return build_object(begin, end, next);
  }
  if (*begin == '[') {
    return build_array(begin, end, next);
  }
  if (*begin == 't' || *begin == 'f') {
    return build_bool(begin, end, next);
  }
  if (*begin == 'n') {
    return build_null(begin, end, next);
  }
  return build_error(ErrMsg[0]);
}

Json Json::build_object(const char* begin, const char* end, const char*& next) {
  const char* p = tool::skip_blank(begin + 1, end);
  if (p == end) {
    return build_error(ErrMsg[0]);
  }
  if (*p == '}') {  // empty object
    next = p + 1;
    return Json(Type::Object, nullptr);
  }
  ObjectData* data = new (std::nothrow) ObjectData;
  if (data == nullptr) {
    return build_error(ErrMsg[3]);
  }
  for (;;) {
    if (*p == '"') {
      const char* p2 = tool::find(p + 1, end, '"');  // *p2 = '"'
      if (p2 == end || *p2 != '"') {                 // err
        delete data;
        return build_error(ErrMsg[1]);
      }
      const char* p3 = tool::skip_blank(p2 + 1, end);  // *p3 == ':'
      if (p3 == end || *p3 != ':') {                   // err
        delete data;
        return build_error(ErrMsg[2]);
      }
      Json j = build(p3 + 1, end, p3);
      if (j.is_error()) {  // err
        delete data;
        return j;
      }
      data->emplace(std::string(p + 1, p2), std::move(j));
      p = tool::skip_blank(p3, end);  // *p == ',' || *p == '}'
      if (p == end) {                 // err
        delete data;
        return build_error(ErrMsg[4]);
      } else if (*p == ',') {
        p = tool::skip_blank(p + 1, end);
        if (p == end) {
          delete data;
          return build_error(ErrMsg[5]);
        }
      } else if (*p == '}') {  // success
        break;
      } else {  // err
        delete data;
        return build_error(ErrMsg[4]);
      }
    } else {  // err
      delete data;
      return build_error(ErrMsg[5]);
    }
  }
  next = p + 1;
  return Json(Type::Object, (void*)data);
}

Json Json::build_bool(const char* begin, const char* end, const char*& next) {
  if (begin + 4 < end && *begin == 't' && *(begin + 1) == 'r' &&
      *(begin + 2) == 'u' && *(begin + 3) == 'e') {
    next = begin + 4;
    return Json(true);
  } else if (begin + 5 < end && *begin == 'f' && *(begin + 1) == 'a' &&
             *(begin + 2) == 'l' && *(begin + 3) == 's' &&
             *(begin + 4) == 'e') {
    next = begin + 5;
    return Json(false);
  }
  return build_error(ErrMsg[6]);
}

Json Json::build_numeric(const char* begin, const char* end,
                         const char*& next) {
  bool is_neg = false;
  if (*begin == '-') {
    ++begin;
    is_neg = true;
  }
  long long integer = 0;
  while (begin != end && '0' <= *begin && *begin <= '9') {
    integer = 10 * integer + (*begin - '0');
    ++begin;
  }
  if (begin == end) {  // err
    return build_error(ErrMsg[0]);
  }
  if (*begin == '.') {  // float
    ++begin;
    if (begin == end || *begin < '0' || *begin > '9') {  // err
      return build_error(ErrMsg[7]);
    }
    double f = 0.0;
    long long pow = 1;
    while (begin != end && '0' <= *begin && *begin <= '9') {
      f = 10 * f + (*begin - '0');
      pow *= 10;
      ++begin;
    }
    next = begin;
    f = integer + f / pow;
    return Json(is_neg ? -f : f);  // float
  }
  next = begin;
  return Json(is_neg ? -integer : integer);  // integer
}

Json Json::build_string(const char* begin, const char* end, const char*& next) {
  const char* p = tool::find(begin + 1, end, '"');
  if (p == end || *p != '"') {
    return build_error(ErrMsg[8]);
  }
  StringData* data = new (std::nothrow) StringData(begin + 1, p);
  if (data == nullptr) {
    return build_error(ErrMsg[9]);
  }
  next = p + 1;
  return Json(Type::String, (void*)data);
}

Json Json::build_array(const char* raw, const char* end, const char*& next) {
  const char* p = tool::skip_blank(raw + 1, end);
  if (p == end) {
    return build_error(ErrMsg[0]);
  }
  if (*p == ']') {
    next = p + 1;
    return Json(Type::Array, nullptr);
  }
  ArrayData* data = new (std::nothrow) ArrayData;
  if (data == nullptr) {
    return build_error(ErrMsg[10]);
  }
  for (;;) {
    data->emplace_back(build(p, end, p));
    if (data->back().type_ == Type::Error) {
      auto errmsg = data->back().data_;
      delete data;
      return build_error(static_cast<const char*>(errmsg));
    }
    p = tool::skip_blank(p, end);
    if (p == end) {  // err
      delete data;
      return build_error(ErrMsg[11]);
    } else if (*p == ',') {
      p = tool::skip_blank(p + 1, end);
    } else if (*p == ']') {  // success
      break;
    } else {  // err
      delete data;
      return build_error(ErrMsg[11]);
    }
  }
  next = p + 1;
  return Json(Type::Array, (void*)data);
}

Json Json::build_null(const char* begin, const char* end, const char*& next) {
  if (begin + 4 < end && *begin == 'n' && *(begin + 1) == 'u' &&
      *(begin + 2) == 'l' && *(begin + 3) == 'l') {
    next = begin + 4;
    return Json(Type::Null, nullptr);
  }
  return build_error(ErrMsg[12]);
}

Json Json::g_null_json_(Json::Type::Null, nullptr);

}  // namespace json
