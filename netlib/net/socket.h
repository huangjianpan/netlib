#pragma once

#include <unistd.h>

#include "address.h"

namespace net {

class Socket {
 public:
  Socket() : fd_(-1) {}

  Socket(const Socket& rhs) = delete;

  Socket& operator=(const Socket& rhs) = delete;

  ~Socket() { close(); }

  void set(int fd, Address* addr) {
    fd_ = fd;
    if (addr != nullptr) {
      address_ = *addr;
    }
  }

  void close() {
    if (fd_ >= 0) {
      ::close(fd_);
      fd_ = -1;
    }
  }

  int fd() const { return fd_; }

  const Address& address() const { return address_; }

 private:
  int fd_;
  Address address_;
};

}  // namespace net