#pragma once

#include "model.h"
#include "json/json.h"

namespace json {

inline void unmarshal(json::Json& j, int& ret) {
  j.move_to(ret);
}

inline void unmarshal(json::Json& j, std::string& ret) {
  j.move_to(ret);
}

inline void unmarshal(json::Json& j, model::Req& ret) {
  unmarshal(j["id"], ret.id);
  unmarshal(j["name"], ret.name);
}

inline const char* unmarshal(const char* begin, const char* end, model::Req& ret) {
  const char* errmsg = nullptr;
  json::Json j = json::Json::unmarshal(begin, end, errmsg);
  if (errmsg != nullptr) {
    return errmsg;
  }
  unmarshal(j, ret);
  return nullptr;
}

inline const char* unmarshal(const std::string& raw, model::Req& ret) {
  return unmarshal(raw.c_str(), raw.c_str() + raw.size(), ret);
}

inline void unmarshal(json::Json& j, std::vector<int>& ret) {
  j.move_to(ret, static_cast<void(*)(Json&, int&)>(unmarshal));
}

inline void unmarshal(json::Json& j, model::Rsp& ret) {
  unmarshal(j["data"], ret.data);
}

inline const char* unmarshal(const char* begin, const char* end, model::Rsp& ret) {
  const char* errmsg = nullptr;
  json::Json j = json::Json::unmarshal(begin, end, errmsg);
  if (errmsg != nullptr) {
    return errmsg;
  }
  unmarshal(j, ret);
  return nullptr;
}

inline const char* unmarshal(const std::string& raw, model::Rsp& ret) {
  return unmarshal(raw.c_str(), raw.c_str() + raw.size(), ret);
}

inline json::Json convert(std::vector<int>&& model) {
  json::Json j(json::Json::Array{});
  for (auto& elem : model) {
    j.add(elem);
  }
  return j;
}

inline json::Json convert(model::Rsp&& model) {
  json::Json j(json::Json::Object{});
  j.add("data", convert(std::move(model.data)));
  return j;
}

inline std::string marshal(const model::Rsp& model) {
  return convert(model::Rsp(model)).marshal();
}

inline std::string marshal(model::Rsp&& model) {
  return convert(std::move(model)).marshal();
}

inline json::Json convert(model::Req&& model) {
  json::Json j(json::Json::Object{});
  j.add("id", model.id);
  j.add("name", std::move(model.name));
  return j;
}

inline std::string marshal(const model::Req& model) {
  return convert(model::Req(model)).marshal();
}

inline std::string marshal(model::Req&& model) {
  return convert(std::move(model)).marshal();
}

} // namespace json
