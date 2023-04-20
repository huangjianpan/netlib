#pragma once

#include "./model.h"
#include "json.h"

namespace json {

inline void unmarshal(json::Json& j, unsigned long& ret) {
  j.move_to(ret);
}

inline void unmarshal(json::Json& j, std::string& ret) {
  j.move_to(ret);
}

inline void unmarshal(json::Json& j, A& ret) {
  unmarshal(j["id"], ret.id);
  unmarshal(j["name"], ret.name);
}

inline const char* unmarshal(const char* begin, const char* end, A& ret) {
  const char* errmsg = nullptr;
  json::Json j = json::Json::unmarshal(begin, end, errmsg);
  if (errmsg != nullptr) {
    return errmsg;
  }
  unmarshal(j, ret);
  return nullptr;
}

inline const char* unmarshal(const std::string& raw, A& ret) {
  return unmarshal(raw.c_str(), raw.c_str() + raw.size(), ret);
}

inline json::Json convert(A&& model) {
  json::Json j(json::Json::Object{});
  j.add("id", model.id);
  j.add("name", std::move(model.name));
  return j;
}

inline std::string marshal(const A& model) {
  return convert(A(model)).marshal();
}

inline std::string marshal(A&& model) {
  return convert(std::move(model)).marshal();
}

} // namespace json
