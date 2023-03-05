#pragma once

#include <condition_variable>
#include <mutex>
#include <vector>

namespace utils {

template <typename T, typename Mutex = std::mutex>
class BlockedQueue {
 public:
  BlockedQueue(size_t capacity)
      : capacity_(capacity), size_(0), lp_(0), rp_(0) {
    queue_.resize(capacity_);
  }

  BlockedQueue(const BlockedQueue& rhs) = delete;
  BlockedQueue& operator=(const BlockedQueue& rhs) = delete;

  size_t capacity() const { return capacity_; }

  size_t size() {
    std::lock_guard<Mutex> lock(mu_);
    return size_;
  }

  void push(const T& value) { return push(T{value}); }

  void push(T&& value) {
    std::unique_lock<Mutex> lock(mu_);
    while (size_ == capacity_) {
      not_fill_cond_.wait(lock);
    }
    queue_[rp_] = std::move(value);
    rp_ = rp_ + 1 == capacity_ ? 0 : rp_ + 1;
    ++size_;
    not_empty_cond_.notify_one();
  }

  T pop() {
    std::unique_lock<Mutex> lock(mu_);
    while (size_ == 0) {
      not_empty_cond_.wait(lock);
    }
    T ret(std::move(queue_[lp_]));
    lp_ = lp_ + 1 == capacity_ ? 0 : lp_ + 1;
    --size_;
    not_fill_cond_.notify_one();
    return ret;
  }

  bool try_pop(T& ret) {
    std::lock_guard<Mutex> lock(mu_);
    if (size_ != 0) {
      ret = std::move(queue_[lp_]);
      lp_ = lp_ + 1 == capacity_ ? 0 : lp_ + 1;
      --size_;
      return true;
    }
    return false;
  }

 private:
  Mutex mu_;
  std::condition_variable_any not_empty_cond_;
  std::condition_variable_any not_fill_cond_;
  std::vector<T> queue_;

  size_t capacity_;
  size_t size_;

  // index is [lp_, rp_)
  size_t lp_;
  size_t rp_;
};

}  // namespace utils