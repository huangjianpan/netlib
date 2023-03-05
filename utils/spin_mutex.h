#pragma once

#include <atomic>

namespace utils {
class SpinMutex {
 public:
  SpinMutex() : flag_(ATOMIC_FLAG_INIT) {}

  SpinMutex(const SpinMutex&) = delete;
  SpinMutex& operator=(const SpinMutex&) = delete;

  void lock() const {
    while (flag_.test_and_set(std::memory_order_acquire))
      ;
  }

  void unlock() const { flag_.clear(std::memory_order_release); }

 private:
  mutable std::atomic_flag flag_;
};
}  // namespace utils
