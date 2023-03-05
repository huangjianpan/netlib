#pragma once

#include "model/model.h"
#include "json.h"

namespace json {

inline void unmarshal(json::Json& j, double& ret) {
  j.move_to(ret);
}

inline void unmarshal(json::Json& j, model::Location& ret) {
  unmarshal(j["latitude"], ret.lat);
  unmarshal(j["longitude"], ret.lng);
}

inline const char* unmarshal(const char* raw, model::Location& ret) {
  const char* errmsg = nullptr;
  json::Json j = json::Json::unmarshal(raw, errmsg);
  if (errmsg != nullptr) {
    return errmsg;
  }
  unmarshal(j, ret);
  return nullptr;
}

inline void unmarshal(json::Json& j, int& ret) {
  j.move_to(ret);
}

inline void unmarshal(json::Json& j, std::string& ret) {
  j.move_to(ret);
}

inline void unmarshal(json::Json& j, std::map<std::string, model::Location>& ret) {
  j.move_to(ret, static_cast<void(*)(Json&, model::Location&)>(unmarshal));
}

inline void unmarshal(json::Json& j, model::Req& ret) {
  unmarshal(j["id"], ret.id);
  unmarshal(j["name"], ret.name);
  unmarshal(j["src"], ret.src);
  unmarshal(j["dest"], ret.dest);
  unmarshal(j["extra"], ret.extra);
}

inline const char* unmarshal(const char* raw, model::Req& ret) {
  const char* errmsg = nullptr;
  json::Json j = json::Json::unmarshal(raw, errmsg);
  if (errmsg != nullptr) {
    return errmsg;
  }
  unmarshal(j, ret);
  return nullptr;
}

} // namespace json
