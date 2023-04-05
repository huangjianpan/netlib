#include <cstdio>

#include "net/net.h"

int main(int argc, char** argv) {
  net::Listener ln(8888);
  printf("ln.fd = %d\n", ln.fd());
  for (;;) {
    auto s = net::NetPool::get<net::Socket>();
    if (s == nullptr) {
      printf("get socket failed\n");
      continue;
    }
    net::Address remote;
    int conn_fd = ln.accept(remote);
    if (conn_fd < 0) {
      printf("accept failed\n");
      continue;
    }
    s->set(conn_fd, &remote);
    printf("fd = %d, ip = %s\n", s->fd(), s->address().ip());
    s->close();
    net::NetPool::put<net::Socket>(s);
  }
  return 0;
}