#include <cstdarg>
#include <string>
#include <thread>

#include "log/log.h"
#include "utils/fmt.h"

int main(int argc, char** argv) {
  using namespace logger;
  
  LOG_ERROR(LogID::generate(), utils::fmt::sprintf("%s_%s", "aaa", "bbb"));
  LOG_ERROR(LogID::generate(), std::string("4574534"));
  LOG_WARN(LogID::generate(), "你好");
  LOG_DEBUG(LogID::generate(), "sfhjh");

  std::this_thread::sleep_for(std::chrono::seconds(3));
  return 0;
}