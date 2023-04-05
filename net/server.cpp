#include "server.h"

#include <thread>
#include <utility>
#include <vector>

#include "connection.h"
#include "fd_operator.h"
#include "net_pool.h"
#include "task_coroutine/task_coroutine.h"

namespace net {

Server::Server(in_port_t port, NewConnectionHandler new_connection_handler,
               void* new_connection_handler_arg)
    : ln_(port),
      new_connection_handler_(new_connection_handler),
      new_connection_handler_arg_(new_connection_handler_arg) {}

Server::~Server() { delete[] epollers_; }

bool Server::start() {
  // must set new_connectino_handler.
  assert(new_connection_handler_ != nullptr);

  size_t n = static_cast<size_t>(std::thread::hardware_concurrency());
  if (n == 0) {
    n = DEFAULT_EPOLLER_NUM;
  }

  epollers_ = new (std::nothrow) Epoller[n];
  if (epollers_ == nullptr) {
    return false;
  }

  // n is the number of epollers
  n_ = n;
  for (size_t i = 1; i < n; ++i) {
    task_coroutine::Coroutine c(event_loop, &epollers_[i]);
  }

  // listener run in the thread which call start()
  FDOperator ln_operator(ln_.fd());
  ln_operator.set_handle_read(listener_default_handler, this);
  choose_index_ = 1 == n ? 0 : 1;
  epollers_[0].control(&ln_operator, Epoller::Event::ADD_R);
  for (;;) {
    epollers_[0].wait(true);
  }
  return true;
}

Epoller& Server::choose_one_epoller() {
  choose_index_ = choose_index_ + 1 == n_ ? 0 : choose_index_ + 1;
  return epollers_[choose_index_];
}

void* Server::event_loop(void* arg) {
  Epoller* epoller = (Epoller*)arg;
  int timeout = 0;
  for (;;) {
    if (epoller->wait(timeout)) {
      timeout = 0;
    } else {
      task_coroutine::Coroutine::yield();
      timeout = 5;
    }
  }
  return nullptr;
}

void Server::listener_default_handler(void* arg) {
  Server* svr = (Server*)arg;
  net::Address remote;
  int conn_fd = svr->ln_.accept(remote);
  if (conn_fd >= 0) {
    Epoller& epoller = svr->choose_one_epoller();
    // init connection
    Connection* conn = NetPool::get<Connection>();
    conn->set_address(remote);
    conn->fd_operator().set_fd(conn_fd);
    conn->fd_operator().set_poller(&epoller);
    // invoke new connection callback
    svr->new_connection_handler_(conn, svr->new_connection_handler_arg_);
    // add fd to epoller
    epoller.control(&conn->fd_operator(), Epoller::Event::ADD_R);
  }
}

}  // namespace net
