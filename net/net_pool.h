#pragma once

#include <type_traits>

#include "connection.h"
#include "utils/pool.hpp"
#include "utils/spin_mutex.h"

namespace net {

class NetPool {
 private:
  static constexpr size_t pool_init_size = 2048;

  static utils::Pool<Connection, utils::SpinMutex> connection_pool;

 public:
  // maybe return nullptr.
  template <typename T>
  static T* get() {
    if constexpr (std::is_same<T, Connection>::value) {
      return connection_pool.get();
    }
    return nullptr;
  }

  template <typename T>
  static void put(T* t) {
    if (t != nullptr) {
      if constexpr (std::is_same<T, Connection>::value) {
        connection_pool.put(t);
      }
    }
  }
};

}  // namespace net
