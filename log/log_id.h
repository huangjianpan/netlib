#pragma once

#include <sys/types.h>

#include <cstring>

#include "utils/random_number.h"

// please use logger::LogID::generate() to generate log id

namespace logger {

class LogIDGenerator;
class LogID;

extern thread_local const LogIDGenerator tls_log_id_generator;

class LogIDGenerator {
 public:
  LogIDGenerator() : random_(0, 35) {}

  LogIDGenerator(const LogIDGenerator&) = delete;

  LogIDGenerator& operator=(const LogIDGenerator&) = delete;

  LogID generate() const;

 private:
  char gen_char() const {
    size_t i = random_.generate();
    return i < 10 ? '0' + i : 'A' + (i - 10);
  }

  utils::RandomNumber random_;
};

class LogID {
  friend class LogIDGenerator;
  
 public:
  LogID() { memset(id_, 0, id_length); }

  LogID(const LogID& rhs) { strncpy(id_, rhs.id_, id_length); }

  LogID& operator=(const LogID& rhs) {
    strncpy(id_, rhs.id_, id_length);
    return *this;
  }

  bool operator==(const LogID& rhs) const {
    return strncmp(id_, rhs.id_, id_length) == 0;
  }

  // generate: thread-safe
  static LogID generate() { return tls_log_id_generator.generate(); }

  const char* c_str() const { return id_; }

 private:
  char id_[32];  // format: yyyyMMddHHmmssXXXXXX (X is random characters from
                 // '0' to '9' and 'A' to 'Z')

  static constexpr size_t id_length = 32;
};
}  // namespace logger
