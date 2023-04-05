#pragma once

#include <cstdlib>
#include <mutex>
#include <vector>

namespace utils {

template <typename T, typename Mutex = std::mutex>
class Pool {
 public:
  Pool(size_t init_capacity) : init_capacity_(init_capacity) {
    pool_.reserve(init_capacity);
    supply_elem(init_capacity);
  }

  Pool(const Pool& rhs) = delete;

  Pool& operator=(const Pool& rhs) = delete;

  ~Pool() {
    for (auto raw : raws_) {
      delete[] raw;
    }
  }

  // maybe return nullptr.
  T* get() {
    std::lock_guard<Mutex> lock(mu_);
    if (pool_.empty()) {
      if (!supply_elem(init_capacity_)) {
        return nullptr;
      }
    }
    T* ret = pool_.back();
    pool_.pop_back();
    return ret;
  }

  // please judge `elem` whether is nullptr before invoke this function.
  void put(T* elem) {
    std::lock_guard<Mutex> lock(mu_);
    pool_.emplace_back(elem);
  }

 private:
  bool supply_elem(size_t count) {
    T* raw = new (std::nothrow) T[count];
    if (raw == nullptr) {
      return false;
    }
    raws_.emplace_back(raw);
    for (size_t i = 0; i < count; ++i) {
      pool_.emplace_back(&raw[i]);
    }
    return true;
  }

  Mutex mu_;
  size_t init_capacity_;
  std::vector<T*> pool_;
  std::vector<T*> raws_;
};

}  // namespace utils
