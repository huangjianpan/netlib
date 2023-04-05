#pragma once

#include <pthread.h>
#include <sys/time.h>

#include <cassert>
#include <string>
#include <thread>

#include "log_data.h"
#include "utils/blocked_queue.hpp"
#include "utils/pool.hpp"
#include "utils/spin_mutex.h"

#define LOG_SEND(level, log_id, raw)                                \
  if (level >= logger::g_logger->min_level()) {                     \
    logger::g_logger->send(level, log_id, __FILE__, __LINE__, raw); \
  }
#define LOG_DEBUG(log_id, raw) LOG_SEND(0, log_id, raw)
#define LOG_INFO(log_id, raw) LOG_SEND(1, log_id, raw)
#define LOG_WARN(log_id, raw) LOG_SEND(2, log_id, raw)
#define LOG_ERROR(log_id, raw) LOG_SEND(3, log_id, raw)
#define LOG_FATAL(log_id, raw) LOG_SEND(4, log_id, raw)

namespace logger {

constexpr const char* g_log_level[] = {"Debug", "Info", "Warn", "Error",
                                       "Fatal"};

struct LogMsg {
  const char* level;
  LogData log_id;
  char time_format[32];  // yyyy-MM-dd HH:mm:ss.us
  const char* file_name;
  int line;
  LogData raw;
};

struct LoggerConf {
  size_t min_level;
  size_t capacity;
  std::string directory;
  size_t max_line_count;

  LoggerConf()
      : min_level(0), capacity(1024), directory(""), max_line_count(1000) {}
};

class Logger {
 public:
  Logger(LoggerConf* conf)
      : conf_(*conf),
        buf_(conf_.capacity),
        pool_(2 * conf_.capacity),
        t_hander_(0){};

  ~Logger() {
    if (t_hander_ != 0) {
      pthread_cancel(t_hander_);
    }
  }

  size_t min_level() const { return conf_.min_level; }

  bool start_logger();

  // send: use perfect forwarding
  template <typename LogIDType, typename RawType>
  void send(size_t level, LogIDType&& log_id, const char* file_name, int line,
            RawType&& raw) {
    LogMsg* msg = pool_.get();
    format_now_time(msg->time_format);
    msg->level = g_log_level[level];
    msg->log_id = std::forward<LogIDType>(log_id);
    msg->file_name = file_name;
    msg->line = line;
    msg->raw = std::forward<RawType>(raw);
    buf_.push(msg);
  }

 private:
  static void format_now_time(char* time_format);

 private:
  LoggerConf conf_;

  utils::BlockedQueue<LogMsg*> buf_;
  utils::Pool<LogMsg, utils::SpinMutex> pool_;
  pthread_t t_hander_;
};

struct InitLogger {
  InitLogger();

 private:
  bool load_config(LoggerConf& conf);
};

extern Logger* g_logger;
extern InitLogger init_g_logger__;

}  // namespace logger
