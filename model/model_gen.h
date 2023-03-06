#pragma once

#include "../model/model.h"
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

inline const char* unmarshal(const std::string& raw, model::Location& ret) {
  return unmarshal(raw.c_str(), ret);
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

inline const char* unmarshal(const std::string& raw, model::Req& ret) {
  return unmarshal(raw.c_str(), ret);
}

inline json::Json convert(model::Location&& model) {
  json::Json j(json::Json::Object{});
  j.add("latitude", model.lat);
  j.add("longitude", model.lng);
  return j;
}

inline std::string marshal(const model::Location& model) {
  return convert(model::Location(model)).marshal();
}

inline std::string marshal(model::Location&& model) {
  return convert(std::move(model)).marshal();
}

inline json::Json convert(std::map<std::string, model::Location>&& model) {
  json::Json j(json::Json::Object{});
  for (auto& kv : model) {
    j.add(kv.first, convert(std::move(kv.second)));
  }
  return j;
}

inline json::Json convert(model::Req&& model) {
  json::Json j(json::Json::Object{});
  j.add("id", model.id);
  j.add("name", std::move(model.name));
  j.add("src", convert(std::move(model.src)));
  j.add("dest", convert(std::move(model.dest)));
  j.add("extra", convert(std::move(model.extra)));
  return j;
}

inline std::string marshal(const model::Req& model) {
  return convert(model::Req(model)).marshal();
}

inline std::string marshal(model::Req&& model) {
  return convert(std::move(model)).marshal();
}

} // namespace json
