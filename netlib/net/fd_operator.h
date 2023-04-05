#pragma once

namespace net {

class Epoller;

class FDOperator {
  friend class Epoller;

 public:
  struct HandlerFunc {
    void (*f)(void* arg);
    void* arg;
  };

  FDOperator() : fd_(-1) {}

  FDOperator(int fd) : fd_(fd) {}

  int fd() const { return fd_; }

  Epoller* poller() const { return poller_; }

  void set_fd(int fd) { fd_ = fd; }

  void set_poller(Epoller* poller) { poller_ = poller; }

  void set_handle_read(void (*f)(void*), void* arg) {
    read_.f = f;
    read_.arg = arg;
  }

  void set_handle_write(void (*f)(void*), void* arg) {
    write_.f = f;
    write_.arg = arg;
  }

  void set_handle_hup(void (*f)(void*), void* arg) {
    hup_.f = f;
    hup_.arg = arg;
  }

  void handle_read() const {
    if (read_.f != nullptr) {
      read_.f(read_.arg);
    }
  }

  void handle_write() const {
    if (write_.f != nullptr) {
      write_.f(write_.arg);
    }
  }

  void handle_hup() const {
    if (hup_.f != nullptr) {
      hup_.f(hup_.arg);
    }
  }

  void reset() {
    fd_ = -1;
    read_.f = nullptr;
    read_.arg = nullptr;
    write_.f = nullptr;
    write_.arg = nullptr;
    hup_.f = nullptr;
    hup_.arg = nullptr;
  }

 private:
  int fd_;
  HandlerFunc read_;
  HandlerFunc write_;
  HandlerFunc hup_;

  Epoller* poller_;
};

}  // namespace net
