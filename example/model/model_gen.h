#pragma once

#include "model.h"
#include "json/json.h"

namespace json {

inline void unmarshal(json::Json& j, std::string& ret) {
  j.move_to(ret);
}

inline void unmarshal(json::Json& j, std::vector<std::string>& ret) {
  j.move_to(ret, static_cast<void(*)(Json&, std::string&)>(unmarshal));
}

inline void unmarshal(json::Json& j, std::vector<std::vector<std::string>>& ret) {
  j.move_to(ret, static_cast<void(*)(Json&, std::vector<std::string>&)>(unmarshal));
}

inline void unmarshal(json::Json& j, model::SudokuReq& ret) {
  unmarshal(j["sudokus"], ret.sudokus);
}

inline const char* unmarshal(const char* begin, const char* end, model::SudokuReq& ret) {
  const char* errmsg = nullptr;
  json::Json j = json::Json::unmarshal(begin, end, errmsg);
  if (errmsg != nullptr) {
    return errmsg;
  }
  unmarshal(j, ret);
  return nullptr;
}

inline const char* unmarshal(const std::string& raw, model::SudokuReq& ret) {
  return unmarshal(raw.c_str(), raw.c_str() + raw.size(), ret);
}

inline void unmarshal(json::Json& j, int& ret) {
  j.move_to(ret);
}

inline void unmarshal(json::Json& j, model::SudokuRsp::Entry& ret) {
  unmarshal(j["errcode"], ret.errcode);
  unmarshal(j["errmsg"], ret.errmsg);
  unmarshal(j["sudoku"], ret.sudoku);
}

inline const char* unmarshal(const char* begin, const char* end, model::SudokuRsp::Entry& ret) {
  const char* errmsg = nullptr;
  json::Json j = json::Json::unmarshal(begin, end, errmsg);
  if (errmsg != nullptr) {
    return errmsg;
  }
  unmarshal(j, ret);
  return nullptr;
}

inline const char* unmarshal(const std::string& raw, model::SudokuRsp::Entry& ret) {
  return unmarshal(raw.c_str(), raw.c_str() + raw.size(), ret);
}

inline void unmarshal(json::Json& j, std::vector<model::SudokuRsp::Entry>& ret) {
  j.move_to(ret, static_cast<void(*)(Json&, model::SudokuRsp::Entry&)>(unmarshal));
}

inline void unmarshal(json::Json& j, model::SudokuRsp& ret) {
  unmarshal(j["sudokus"], ret.sudokus);
}

inline const char* unmarshal(const char* begin, const char* end, model::SudokuRsp& ret) {
  const char* errmsg = nullptr;
  json::Json j = json::Json::unmarshal(begin, end, errmsg);
  if (errmsg != nullptr) {
    return errmsg;
  }
  unmarshal(j, ret);
  return nullptr;
}

inline const char* unmarshal(const std::string& raw, model::SudokuRsp& ret) {
  return unmarshal(raw.c_str(), raw.c_str() + raw.size(), ret);
}

inline json::Json convert(std::vector<std::string>&& model) {
  json::Json j(json::Json::Array{});
  for (auto& elem : model) {
    j.add(std::move(elem));
  }
  return j;
}

inline json::Json convert(std::vector<std::vector<std::string>>&& model) {
  json::Json j(json::Json::Array{});
  for (auto& elem : model) {
    j.add(convert(std::move(elem)));
  }
  return j;
}

inline json::Json convert(model::SudokuReq&& model) {
  json::Json j(json::Json::Object{});
  j.add("sudokus", convert(std::move(model.sudokus)));
  return j;
}

inline std::string marshal(const model::SudokuReq& model) {
  return convert(model::SudokuReq(model)).marshal();
}

inline std::string marshal(model::SudokuReq&& model) {
  return convert(std::move(model)).marshal();
}

inline json::Json convert(model::SudokuRsp::Entry&& model) {
  json::Json j(json::Json::Object{});
  j.add("errcode", model.errcode);
  j.add("errmsg", std::move(model.errmsg));
  j.add("sudoku", convert(std::move(model.sudoku)));
  return j;
}

inline std::string marshal(const model::SudokuRsp::Entry& model) {
  return convert(model::SudokuRsp::Entry(model)).marshal();
}

inline std::string marshal(model::SudokuRsp::Entry&& model) {
  return convert(std::move(model)).marshal();
}

inline json::Json convert(std::vector<model::SudokuRsp::Entry>&& model) {
  json::Json j(json::Json::Array{});
  for (auto& elem : model) {
    j.add(convert(std::move(elem)));
  }
  return j;
}

inline json::Json convert(model::SudokuRsp&& model) {
  json::Json j(json::Json::Object{});
  j.add("sudokus", convert(std::move(model.sudokus)));
  return j;
}

inline std::string marshal(const model::SudokuRsp& model) {
  return convert(model::SudokuRsp(model)).marshal();
}

inline std::string marshal(model::SudokuRsp&& model) {
  return convert(std::move(model)).marshal();
}

} // namespace json
