#pragma once

#include <sys/uio.h>

#include <vector>

namespace net {

class Buffer {
  static constexpr size_t BUFFER_INIT_SIZE = 4096;
  static constexpr size_t EXTRA_BUFFER_SIZE = 65536;

  static thread_local char extra_buffer[EXTRA_BUFFER_SIZE];

 public:
  Buffer() {
    buf_ = new (std::nothrow) char[BUFFER_INIT_SIZE];
    size_ = 0;
    cap_ = buf_ == nullptr ? 0 : BUFFER_INIT_SIZE;
  }

  Buffer(const Buffer& rhs) = delete;

  Buffer& operator=(const Buffer& rhs) = delete;

  ~Buffer() { delete[] buf_; }

  bool empty() const { return size_ == 0; }

  bool valid() const { return buf_ != nullptr; }

  void clear() {
    size_ = 0;
    buf_[0] = '\0';
  }

  const char* begin() const { return &buf_[0]; }

  const char* end() const { return &buf_[0] + size_; }

  size_t size() const { return size_; }

  bool read(int fd);

 private:
  void reset_buf(char* buf, size_t size, size_t cap) {
    delete[] buf_;
    buf_ = buf;
    size_ = size;
    cap_ = cap;
  }

  static size_t alignment(size_t n) {
    return ((n + BUFFER_INIT_SIZE) / BUFFER_INIT_SIZE) * BUFFER_INIT_SIZE;
  }

  char* buf_;
  size_t size_;
  size_t cap_;
};

}  // namespace net
