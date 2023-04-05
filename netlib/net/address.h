#pragma once

#include <arpa/inet.h>
#include <linux/in.h>
#include <cstring>

namespace net {

class Address {
 public:
  Address() {}

  Address(const char* ip, in_port_t port) {
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr_.sin_addr);
    bzero(&addr_.sin_zero, 8);
  }

  Address(in_port_t port) {
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&addr_.sin_zero, 8);
  }

  Address(const struct sockaddr_in& addr) : addr_(addr) {}

  void set(const struct sockaddr_in& addr) { addr_ = addr; }

  in_port_t port() const { return ntohs(addr_.sin_port); }

  sa_family_t family() const { return addr_.sin_family; }

  const char* ip() const {
    inet_ntop(family(), &addr_.sin_addr, ip_,
              static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    return ip_;
  }

  const struct sockaddr* sockaddr() const {
    return (const struct sockaddr*)&addr_;
  }

  struct sockaddr* sockaddr() { return (struct sockaddr*)&addr_; }

 private:
  struct sockaddr_in addr_;

  static thread_local char ip_[64];  // thread safe
};

}  // namespace net
