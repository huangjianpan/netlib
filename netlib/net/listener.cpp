#include "listener.h"

#include <errno.h>
#include <unistd.h>

#include "logger.h"

namespace net {

Listener::Listener(in_port_t port)
    : fd_(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)),
      idlefd_(open("/dev/null", O_CLOEXEC)),
      addr_(port) {
  // check init access
  assert(fd_ >= 0 && idlefd_ >= 0);
  // reuse addr and port
  int on = 1;
  assert(setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &on,
                    static_cast<socklen_t>(sizeof on)) == 0);
  // bind
  assert(bind(fd_, addr_.sockaddr(),
              static_cast<socklen_t>(sizeof(struct sockaddr_in6))) == 0);
  // listen
  assert(listen(fd_, 128) == 0);
}

Listener::~Listener() { close(fd_); }

int Listener::accept(Address& remote) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in6));
  int conn_fd =
      accept4(fd_, remote.sockaddr(), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (conn_fd >= 0) {
    return conn_fd;
  } else {
    int err = errno;
    if (err == EAGAIN || err == EINTR) {
      // ignore
    } else if (err == EMFILE) {
      close(idlefd_);
      idlefd_ = accept4(fd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
      close(idlefd_);
      idlefd_ = open("dev/null", O_CLOEXEC);
    } else {  // other error
      LOG_ERROR(LISTENER_LOG_ID,
                utils::fmt::sprintf("accept failed, errno:%d", err));
    }
  }
  return -1;
}

}  // namespace net
