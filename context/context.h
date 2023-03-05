#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

namespace context {

// Context thread-safe
class Context {
 public:
  Context() : p_kvs_(nullptr) {}

  ~Context();

  bool set_key_value(std::string&& key, void* value, void (*deleter)(void*));

  bool set_key_value(const std::string& key, void* value,
                     void (*deleter)(void*)) {
    return set_key_value(key.c_str(), value, deleter);
  }

  bool set_key_value(const char* key, void* value, void (*deleter)(void*)) {
    return set_key_value(std::string(key), value, deleter);
  }

  bool set_key_value_non_thread_safe(std::string&& key, void* value,
                                     void (*deleter)(void*));

  bool set_key_value_non_thread_safe(const std::string& key, void* value,
                                     void (*deleter)(void*)) {
    return set_key_value_non_thread_safe(key.c_str(), value, deleter);
  }

  bool set_key_value_non_thread_safe(const char* key, void* value,
                                     void (*deleter)(void*)) {
    return set_key_value_non_thread_safe(std::string(key), value, deleter);
  }

  void* get_value(const std::string& key);

  void* get_value(const char* key) { return get_value(std::string(key)); }

  void* get_value_non_thread_safe(const std::string& key);

  void* get_value_non_thread_safe(const char* key) {
    return get_value_non_thread_safe(std::string(key));
  }

 private:
  std::unordered_map<std::string, std::pair<void*, void (*)(void*)>>* p_kvs_;
  std::mutex mu_;
};

}  // namespace context