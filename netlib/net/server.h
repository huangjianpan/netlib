#pragma once

#include "connection.h"
#include "epoller.h"
#include "listener.h"

namespace net {
class Server {
  static constexpr unsigned int DEFAULT_EPOLLER_NUM = 8;

 public:
  using NewConnectionHandler = void (*)(Connection*, void*);

  Server(in_port_t port, NewConnectionHandler new_connection_handler,
         void* new_connection_handler_arg);

  Server(const Server&) = delete;

  ~Server();

  Server& operator=(const Server&) = delete;

  bool start();

 private:
  Epoller& choose_one_epoller();

  // epoller run in task_coroutine::Coroutine
  static void* event_loop(void* arg);

  // listener default read callback.
  static void listener_default_handler(void* arg);

 private:
  Listener ln_;
  Epoller* epollers_;
  
  size_t n_;             // the number of epollers
  size_t choose_index_;  // for choose_one_epoller

  NewConnectionHandler
      new_connection_handler_;  // new connection callback, use to set
                                // input_handler and output_handler
  void* new_connection_handler_arg_;
};
}  // namespace net
