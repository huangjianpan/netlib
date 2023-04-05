#include "log_id.h"

#include <sys/time.h>

#include <cstdio>

namespace logger {

thread_local const LogIDGenerator tls_log_id_generator;

LogID LogIDGenerator::generate() const {
  LogID log_id;
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  struct tm tm_time;
  localtime_r((const time_t*)&tv.tv_sec, &tm_time);
  ::snprintf(log_id.id_, 64, "%04d%02d%02d%02d%02d%02d%c%c%c%c%c%c",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, gen_char(),
             gen_char(), gen_char(), gen_char(), gen_char(), gen_char());
  return log_id;
}
}  // namespace logger
