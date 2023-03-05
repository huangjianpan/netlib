#ifndef _TASK_COROUTINE_TASK_SCHEDULING_QUEUE_
#define _TASK_COROUTINE_TASK_SCHEDULING_QUEUE_
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

namespace task_coroutine {

template <typename T>
class TaskSchedulingQueue {
 public:
  TaskSchedulingQueue() {}

  void push(const T& value) {
    std::lock_guard<std::mutex> lock(mu_);
    sq_.push(value);
    cond_.notify_one();
  }

  void push(T&& value) {
    std::lock_guard<std::mutex> lock(mu_);
    sq_.push(std::forward<T>(value));
    cond_.notify_one();
  }

  T pop() {
    std::unique_lock<std::mutex> lock(mu_);
    while (sq_.empty()) {
      cond_.wait(lock);
    }
    T val = std::move(sq_.front());
    sq_.pop();
    return val;
  }

  bool try_pop(T& ret) {
    std::lock_guard<std::mutex> lock(mu_);
    if (sq_.empty()) {
      return false;
    }
    ret = std::move(sq_.front());
    sq_.pop();
    return true;
  }

 private:
  std::mutex mu_;
  std::condition_variable cond_;
  std::queue<T> sq_;
};

}  // namespace task_coroutine

#endif  // !_TASK_COROUTINE_TASK_SCHEDULING_QUEUE_
