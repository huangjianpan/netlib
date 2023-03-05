#pragma once

#include <random>

namespace utils {

// RandomNumber: use to generate 64-bit unsigned integer, the result ∈
// [lower_limit, upper_limit]
class RandomNumber {
 public:
  RandomNumber(size_t lower_limit, size_t upper_limit)
      : dis_(lower_limit, upper_limit) {}

  RandomNumber(const RandomNumber&) = delete;

  RandomNumber& operator=(const RandomNumber&) = delete;

  size_t generate() const { return dis_(engine_); }

 private:
  mutable std::default_random_engine engine_;
  mutable std::uniform_int_distribution<size_t> dis_;
};

}  // namespace utils
