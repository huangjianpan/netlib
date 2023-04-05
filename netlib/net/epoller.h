#pragma once

#include <sys/epoll.h>
#include <unistd.h>

#include <cassert>
#include <vector>

#include "fd_operator.h"

namespace net {

class Epoller {
 public:
  enum class Event { ADD_R, ADD_W, MOD_R, MOD_RW, DEL };

 public:
  using EventList = std::vector<struct epoll_event>;

  explicit Epoller() : epfd_(epoll_create1(EPOLL_CLOEXEC)), events_(16) {
    assert(epfd_ >= 0);
  }

  ~Epoller() { ::close(epfd_); }

  Epoller(const Epoller& rhs) = delete;

  Epoller& operator=(const Epoller& rhs) = delete;

  void run();

  bool wait(int timeout);

  void control(FDOperator* oper, Event event);

 private:
  void handler(size_t n);

  // run in task_coroutine
  static void* handler_hups(void* on_hups);

  void add_hup(FDOperator* op);

  const int epfd_;  // epoll fd
  EventList events_;
  std::vector<FDOperator::HandlerFunc>* on_hups_;
};

}  // namespace net
