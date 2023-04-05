#include "connection.h"

#include "net_pool.h"
#include "task_coroutine/task_coroutine.h"

namespace net {
void Connection::close() {
  fd_operator_.poller()->control(&fd_operator_, net::Epoller::Event::DEL);
  ::close(fd_operator_.fd());
  NetPool::put<Connection>(this);
}

void Connection::on_hup(void* arg) {
  Connection* conn = (Connection*)arg;
  int expected = 0;
  while (!conn->state_.compare_exchange_weak(expected, 1)) {
    task_coroutine::Coroutine::yield();
    expected = 0;
  }
  conn->state_.store(0);
  ::close(conn->fd_operator_.fd());
  NetPool::put<Connection>(conn);
}
}  // namespace net