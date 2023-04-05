#pragma once

#include <string>

#include "log_id.h"

namespace logger {

// LogData: use to proxy std::string, static const char*, LogID
class LogData {
 public:
  enum class Type { LogID, CString, CXXString };

  LogData(const LogID& log_id) : log_id_(log_id), type_(Type::LogID) {}

  LogData(const char* c_string = nullptr)
      : c_string_(c_string), type_(Type::CString) {}

  LogData(const std::string& cxx_string)
      : cxx_string_(cxx_string), type_(Type::CXXString) {}

  LogData(std::string&& cxx_string)
      : cxx_string_(std::move(cxx_string)), type_(Type::CXXString) {}

  LogData(const LogData&) = delete;

  LogData& operator=(const LogData&) = delete;

  LogData& operator=(const LogID& log_id) {
    type_ = Type::LogID;
    log_id_ = log_id;
    return *this;
  }

  LogData& operator=(const char* c_string) {
    type_ = Type::CString;
    c_string_ = c_string;
    return *this;
  }

  LogData& operator=(const std::string& cxx_string) {
    type_ = Type::CXXString;
    cxx_string_ = cxx_string;
    return *this;
  }

  LogData& operator=(std::string&& cxx_string) {
    type_ = Type::CXXString;
    cxx_string_ = std::move(cxx_string);
    return *this;
  }

  const char* data() const {
    return type_ == Type::CXXString
               ? cxx_string_.c_str()
               : (type_ == Type::CString ? c_string_ : log_id_.c_str());
  }

  Type type() const { return type_; }

 private:
  union {
    LogID log_id_;
    const char* c_string_;
  };
  std::string cxx_string_;
  Type type_;
};

}  // namespace logger
