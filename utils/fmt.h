#pragma once

#include <cstring>
#include <string>

namespace utils {
namespace fmt {

constexpr size_t buffer_init_length = 1024;

std::string sprintf(const char* format, ...);

}  // namespace fmt
}  // namespace utils
