#include "epoller.h"

#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "logger.h"
#include "task_coroutine/task_coroutine.h"

namespace net {

bool Epoller::wait(int timeout) {
  int n = epoll_wait(epfd_, events_.data(), static_cast<int>(events_.size()),
                     timeout);
  if (n > 0) {
    handler(n);
    if (n == static_cast<int>(events_.size())) {
      events_.resize(2 * n);
    }
    return true;
  } else {
    if (n != 0 && errno != EINTR) {
      LOG_ERROR(
          EPOLLER_LOG_ID,
          utils::fmt::sprintf("epoll_wait failed, epfd:%d, errno:%d, n = %d",
                              epfd_, errno, n));
    }
  }
  return false;
}

void Epoller::run() {
  for (;;) {
    int n =
        epoll_wait(epfd_, events_.data(), static_cast<int>(events_.size()), 0);
    if (n == 0) {
      n = epoll_wait(epfd_, events_.data(), static_cast<int>(events_.size()),
                     -1);
    }
    if (n > 0) {
      handler(n);
    } else {
      if (errno == EINTR) {
        continue;
      }
      LOG_ERROR(EPOLLER_LOG_ID,
                utils::fmt::sprintf("epoll_wait failed, epfd:%d, errno:%d",
                                    epfd_, errno));
      break;
    }
  }
}

void Epoller::control(FDOperator* oper, Event event) {
  int op;
  struct epoll_event evt;
  evt.data.ptr = (void*)oper;
  switch (event) {
    case Event::ADD_R:
      op = EPOLL_CTL_ADD;
      evt.events = EPOLLIN | EPOLLRDHUP | EPOLLERR;
      break;
    case Event::ADD_W:
      op = EPOLL_CTL_ADD;
      evt.events = EPOLLET | EPOLLOUT | EPOLLRDHUP | EPOLLERR;
      break;
    case Event::MOD_R:
      op = EPOLL_CTL_MOD;
      evt.events = EPOLLIN | EPOLLRDHUP | EPOLLERR;
      break;
    case Event::MOD_RW:
      op = EPOLL_CTL_MOD;
      evt.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR;
      break;
    case Event::DEL:
      op = EPOLL_CTL_DEL;
      evt.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR;
      break;
  }
  if (epoll_ctl(epfd_, op, oper->fd_, &evt) < 0) {
    LOG_ERROR(EPOLLER_LOG_ID,
              utils::fmt::sprintf(
                  "epoll_ctl failed, epfd:%d, op:%d, fd:%d, event:%d, errno:%d",
                  epfd_, op, oper->fd_, (int)event, errno));
  }
}

void Epoller::handler(size_t n) {
  bool trigger_read, trigger_write, trigger_hup, trigger_error;
  if (on_hups_ == nullptr) {
    on_hups_ = new (std::nothrow) std::vector<FDOperator::HandlerFunc>;
  }
  for (size_t i = 0; i < n; ++i) {
    FDOperator* op = static_cast<FDOperator*>(events_[i].data.ptr);

    uint32_t evt = events_[i].events;
    trigger_read = evt & EPOLLIN;
    trigger_write = evt & EPOLLOUT;
    trigger_hup = evt & (EPOLLHUP | EPOLLRDHUP);
    trigger_error = evt & EPOLLERR;

    if (trigger_hup) {
      add_hup(op);
      continue;
    }
    if (trigger_read) {
      op->handle_read();
    }
    if (trigger_error) {
      read(op->fd_, nullptr, 0);
      if (errno != EAGAIN) {
        add_hup(op);
      }
      continue;
    }
    if (trigger_write) {
      op->handle_write();
    }
  }
  if (on_hups_ != nullptr && on_hups_->size() > 0) {
    task_coroutine::Coroutine c(handler_hups, on_hups_);
    on_hups_ = nullptr;
  }
}

void* Epoller::handler_hups(void* on_hups) {
  auto list = (std::vector<FDOperator::HandlerFunc>*)on_hups;
  for (auto& handler : *list) {
    handler.f(handler.arg);
  }
  delete list;
  return nullptr;
}

void Epoller::add_hup(FDOperator* op) {
  if (epoll_ctl(epfd_, EPOLL_CTL_DEL, op->fd_, nullptr) < 0) {
    LOG_ERROR(EPOLLER_LOG_ID,
              utils::fmt::sprintf("epoll_ctl delete fd{%d} failed, errno:%d",
                                  op->fd_, errno));
  }
  if (op->hup_.f != nullptr) {
    if (on_hups_ != nullptr) {
      on_hups_->emplace_back(op->hup_);
    } else {
      op->hup_.f(op->hup_.arg);
    }
  }
}

}  // namespace net
