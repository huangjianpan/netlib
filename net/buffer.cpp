#include "buffer.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace net {

thread_local char Buffer::extra_buffer[Buffer::EXTRA_BUFFER_SIZE];

bool Buffer::read(int fd) {
  struct iovec iov[2];
  iov[0].iov_base = (void*)buf_;
  iov[0].iov_len = cap_;
  iov[1].iov_base = extra_buffer;
  iov[1].iov_len = EXTRA_BUFFER_SIZE;
  ssize_t n = readv(fd, iov, 2);
  if (n < 0) {
    return false;
  }
  if (static_cast<size_t>(n) < cap_) {
    buf_[n] = '\0';
    size_ = static_cast<size_t>(n);
  } else {
    if (static_cast<size_t>(n) < cap_ + EXTRA_BUFFER_SIZE) {
      char* new_buf = new (std::nothrow) char[alignment(n)];
      if (new_buf == nullptr) {
        return false;
      }
      memcpy(new_buf, buf_, cap_);
      memcpy(new_buf + cap_, extra_buffer, n - cap_);
      new_buf[n] = '\0';
      reset_buf(new_buf, n, alignment(n));
    } else {
      std::vector<struct iovec> iovs;
      bool ok = true;
      for (;;) {
        iovs.push_back(
            {new (std::nothrow) char[EXTRA_BUFFER_SIZE], EXTRA_BUFFER_SIZE});
        if (iovs.back().iov_base == nullptr) {
          ok = false;
          break;
        }
        ssize_t len = readv(fd, &iovs.back(), 1);
        if (len < 0) {
          ok = false;
          break;
        }
        if (len == 0) {
          break;
        }
        iovs.back().iov_len = len;
        if (static_cast<size_t>(len) < EXTRA_BUFFER_SIZE) {
          break;
        }
      }
      if (ok) {
        size_t total_cap = n;
        for (const auto& v : iovs) {
          total_cap += v.iov_len;
        }
        total_cap = alignment(total_cap);
        char* new_buf = new (std::nothrow) char[total_cap];
        if (new_buf != nullptr) {
          memcpy(new_buf, buf_, cap_);
          memcpy(new_buf + cap_, extra_buffer, EXTRA_BUFFER_SIZE);
          size_t offset = n;
          for (const auto& v : iovs) {
            memcpy(new_buf + offset, v.iov_base, v.iov_len);
            offset += v.iov_len;
          }
          new_buf[offset] = '\0';
          reset_buf(new_buf, n, total_cap);
        }
      }
      for (auto& v : iovs) {
        delete[] (char*)v.iov_base;
      }
      return ok;
    }
  }
  return true;
}

}  // namespace net