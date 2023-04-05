#include "net_pool.h"

namespace net {
utils::Pool<Connection, utils::SpinMutex> NetPool::connection_pool(
    NetPool::pool_init_size);
}  // namespace net