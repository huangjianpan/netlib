#include "fmt.h"

#include <cstdarg>

namespace utils {
namespace fmt {

std::string sprintf(const char* format, ...) {
  std::string buf;
  buf.resize(buffer_init_length);
  va_list args;
  va_start(args, format);
  int write_len = vsnprintf(&buf[0], buf.size() + 1, format, args);
  va_end(args);
  if (write_len > (int)buf.size()) {
    buf.resize(write_len);
    va_start(args, format);
    vsnprintf(&buf[0], buf.size() + 1, format, args);
    va_end(args);
  }
  return buf;
}
}  // namespace fmt

}  // namespace utils