#pragma once

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cassert>

#include "address.h"
#include "socket.h"

namespace net {

class Listener {
 public:
  Listener(in_port_t port);

  ~Listener();

  Listener(const Listener& ln) = delete;

  Listener& operator=(const Listener& ln) = delete;

  int fd() const { return fd_; }

  // return connection fd and remote address, failure return -1.
  int accept(Address& remote);

 private:
  int fd_;
  int idlefd_;
  const Address addr_;
};

}  // namespace net
