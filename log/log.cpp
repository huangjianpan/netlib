#include "log.h"

#include <cassert>
#include <unordered_map>

#include "utils/file.h"

namespace logger {

LogMsgPool::LogMsgPool(size_t capacity) : pool_(capacity) {
  raw_data_ = new (std::nothrow) LogMsg[capacity];
  assert(raw_data_ != nullptr);
  for (size_t i = 0; i < capacity; ++i) {
    pool_.push(&raw_data_[i]);
  }
}

bool Logger::start_logger() {
  // check whether the log directory exists
  utils::file::Directory dir(conf_.directory.c_str());
  if (!dir.exist()) {
    if (!dir.create(conf_.directory.c_str())) {
      return false;
    }
  }

  std::thread t([this]() {
    size_t line = conf_.max_line_count;  // the number of lines written to the
                                         // current log file
    size_t log_file_num = 1;             // the number of log file
    utils::file::File f;                 // point to current log file
    LogMsg* msg = nullptr;
    char buffer[65536];  // the maximum number of characters per line is 65536
    for (;;) {
      // when `line` equal to `max_line_count`, create a new log file
      if (line == conf_.max_line_count) {
        line = 0;
        char file_name[64];
        format_now_time(file_name);
        file_name[10] = '\0';
        char absolute_path[1024];
        do {
          ::snprintf(absolute_path, sizeof(absolute_path), "%s/%s_%lu.log",
                     conf_.directory.c_str(), file_name, log_file_num);
          ++log_file_num;
        } while (!f.create(absolute_path));
      }

      msg = buf_.pop();
      int length =
          ::snprintf(buffer, sizeof(buffer), "%s LogID:%s %s %s:%d %s\n",
                     msg->level, msg->log_id.data(), msg->time_format,
                     msg->file_name, msg->line, msg->raw.data());
      if ((size_t)length >= sizeof(buffer)) {  // truncate
        length = sizeof(buffer);
        buffer[sizeof(buffer) - 1] = '\n';
      }
      ssize_t wlen = ::write(f.fd(), buffer, length);
      (void)wlen;
      pool_.put(msg);
      ++line;
    }
  });
  t_hander_ = t.native_handle();
  t.detach();
  return true;
}

void Logger::format_now_time(char* time_format) {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  struct tm tm_time;
  localtime_r((const time_t*)&tv.tv_sec, &tm_time);
  int wsize =
      sprintf(time_format, "%4d-%02d-%02d %02d:%02d:%02d.%06ld",
              tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
              tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, tv.tv_usec);
  time_format[wsize] = '\0';
}

bool InitLogger::load_config(LoggerConf& conf) {
  std::unordered_map<std::string, std::string> log_conf =
#include "./log.conf"
      ;

  for (const auto& entry : log_conf) {
    const auto& k = entry.first;
    const auto& v = entry.second;
    if (k == "min_level") {
      for (size_t i = 0; i < 5; ++i) {
        if (v == g_log_level[i]) {
          conf.min_level = i;
          break;
        }
      }
    } else if (k == "capacity") {
      conf.capacity = (size_t)atoi(v.c_str());
    } else if (k == "log_directory") {
      conf.directory = v;
    } else if (k == "max_line_count") {
      conf.max_line_count = (size_t)atoi(v.c_str());
    }
  }

  // printf(
  //     "Logger:\n"
  //     "    min_level = %s\n"
  //     "    capacity = %lu\n",
  //     "    log_directory = %s\n", g_log_level[min_level], capacity,
  //     log_directory.c_str());

  return (0 <= conf.min_level && conf.min_level <= 5) && conf.capacity > 0 &&
         conf.directory.size() > 0 && conf.max_line_count > 0;
}

InitLogger::InitLogger() {
  LoggerConf conf;
  if (!load_config(conf)) {
    abort();
  }
  g_logger = new (std::nothrow) Logger(&conf);
  assert(g_logger != nullptr);
  assert(g_logger->start_logger());
}

Logger* g_logger = nullptr;
InitLogger init_g_logger__;

}  // namespace logger