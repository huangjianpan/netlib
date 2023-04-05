#pragma once

namespace utils {

template <typename F>
struct Defer {
 private:
  F f_;

 public:
  Defer(F&& f) : f_(std::move(f)) {}

  Defer(const F& f) : f_(f) {}

  Defer(const Defer&) = delete;

  Defer& operator=(const Defer&) = delete;

  ~Defer() { f_(); }
};

}  // namespace utils
