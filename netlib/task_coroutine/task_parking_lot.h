#pragma once

#include <condition_variable>
#include <mutex>

namespace task_coroutine {

class ParkingLot {
 public:
  ParkingLot() {}

  void notify() const { cond_.notify_all(); }

  void wait() const {
    std::unique_lock<std::mutex> lock(mu_);
    cond_.wait(lock);
  }

 private:
  mutable std::mutex mu_;
  mutable std::condition_variable cond_;
};

}  // namespace task_coroutine