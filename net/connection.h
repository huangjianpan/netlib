#pragma once

#include "address.h"
#include "buffer.h"
#include "epoller.h"
#include "fd_operator.h"
#include "task_coroutine/task_coroutine.h"

namespace net {

// state transition
// idle --(in on_read)--> inprocess --(call close)--> close
//  ^                         |
//  |                         |
//  +-----(out on_read)-------+
//                         

class Connection {
 public:
  using HandlerFunc = void (*)(Connection*);

  Connection() : state_(0) {
    fd_operator_.set_handle_read(on_read, this);
    fd_operator_.set_handle_write(on_write, this);
    fd_operator_.set_handle_hup(on_hup, this);
  }

  Connection(const Connection&) = delete;

  Connection& operator=(const Connection&) = delete;

  FDOperator& fd_operator() { return fd_operator_; }

  const Buffer& input_buffer() const { return input_buffer_; }

  Buffer& input_buffer() { return input_buffer_; }

  const Address& address() const { return address_; }

  void set_address(const Address& addr) { address_ = addr; }

  void set_address(Address&& addr) { address_ = std::move(addr); }

  void set_data(void* data) { data_ = data; }

  void* data() { return data_; }

  void set_input_handler(HandlerFunc handler) { input_handler_ = handler; }

  void set_output_handler(HandlerFunc handler) { output_handler_ = handler; }

  // use to trigger output_handler
  void trigger_write() {
    fd_operator_.poller()->control(&fd_operator_, net::Epoller::Event::MOD_RW);
  }

  void close();

 private:
  static void on_read(void* arg) {
    Connection* conn = (Connection*)arg;
    conn->input_buffer_.read(conn->fd_operator_.fd());
    if (conn->input_handler_ != nullptr) {
      int expected = 0;
      if (conn->state_.compare_exchange_strong(expected, 1)) {
        task_coroutine::Coroutine c(on_handler, conn);
      }
    }
  }

  static void* on_handler(void* arg) {
    Connection* conn = (Connection*)arg;
    conn->input_handler_(conn);
    conn->state_.store(0);
    return nullptr;
  }

  static void on_write(void* arg) {
    Connection* conn = (Connection*)arg;
    if (conn->output_handler_ != nullptr) {
      conn->output_handler_(conn);
    }
    conn->fd_operator_.poller()->control(&conn->fd_operator_,
                                         net::Epoller::Event::MOD_R);
  }

  static void on_hup(void* arg);

 private:
  FDOperator fd_operator_;
  Address address_;      // remote address.
  Buffer input_buffer_;  // store remote input data.

  HandlerFunc input_handler_;
  HandlerFunc output_handler_;

  void* data_;  // user data.

  std::atomic<int> state_;  // 0 idle, 1 inprocess
};
}  // namespace net
