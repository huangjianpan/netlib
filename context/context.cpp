#include "context.h"

namespace context {

Context::~Context() {
  std::lock_guard<std::mutex> lock(mu_);
  if (p_kvs_ != nullptr) {
    auto& kvs = *p_kvs_;
    for (auto& p : kvs) {
      auto& value = p.second;
      if (value.second != nullptr) {
        value.second(value.first);
      }
    }
    delete p_kvs_;
  }
}

bool Context::set_key_value(std::string&& key, void* value,
                            void (*deleter)(void*)) {
  std::lock_guard<std::mutex> lock(mu_);
  return set_key_value_non_thread_safe(std::forward<std::string>(key), value,
                                       deleter);
}

bool Context::set_key_value_non_thread_safe(std::string&& key, void* value,
                                            void (*deleter)(void*)) {
  if (p_kvs_ == nullptr) {
    p_kvs_ = new (std::nothrow)
        std::unordered_map<std::string, std::pair<void*, void (*)(void*)>>;
    if (p_kvs_ == nullptr) {
      return false;
    }
    p_kvs_->emplace(std::forward<std::string>(key),
                    std::make_pair(value, deleter));
    return true;
  }
  auto it = p_kvs_->find(key);
  if (it != p_kvs_->end()) {
    auto& old_value = it->second;
    old_value.second(old_value.first);
    it->second = std::make_pair(value, deleter);
    return true;
  }
  p_kvs_->emplace(std::forward<std::string>(key),
                  std::make_pair(value, deleter));
  return true;
}

void* Context::get_value(const std::string& key) {
  std::lock_guard<std::mutex> lock(mu_);
  return get_value_non_thread_safe(key);
}

void* Context::get_value_non_thread_safe(const std::string& key) {
  if (p_kvs_ == nullptr) {
    return nullptr;
  }
  auto it = p_kvs_->find(key);
  if (it == p_kvs_->end()) {
    return nullptr;
  }
  return it->second.first;
}

}  // namespace context
