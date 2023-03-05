#include <cassert>
#include <mutex>
#include <thread>
#include <utility>

#include "utils/blocked_queue.hpp"
#include "utils/spin_mutex.h"

template <typename Mutex>
void test01() {
  {
    utils::BlockedQueue<int, Mutex> q(10);
    q.push(1);
    assert(q.pop() == 1);
  }
  {
    utils::BlockedQueue<std::pair<int, int>, Mutex> q(10);
    auto p = std::make_pair(1, 1);
    q.push(p);
    auto p2 = q.pop();
    assert(p2.first == 1 && p2.second == 1);
  }
  {
    utils::BlockedQueue<int, Mutex> q(1);
    int ret = 0;
    assert(q.try_pop(ret) == false);
    q.push(100);
    assert(q.try_pop(ret) == true && ret == 100);
    assert(q.size() == 0);
  }
}

template <typename Mutex>
void test02() {
  utils::BlockedQueue<size_t, Mutex> q(100);
  std::thread t1([&q]() -> void {
    for (size_t i = 0; i < 5000; ++i) {
      q.push(i);
      std::this_thread::sleep_for(std::chrono::microseconds(5));
    }
  });
  std::thread t2([&q]() -> void {
    for (size_t i = 0; i < 4900; ++i) {
      assert(q.pop() == i);
    }
  });
  t1.join();
  t2.join();
  assert(q.size() == 100);
  for (size_t i = 4900; i < 5000; ++i) {
    size_t ret = 0;
    assert(q.try_pop(ret) == true && ret == i);
  }
}

template <typename Mutex>
void test03() {
  utils::BlockedQueue<size_t, Mutex> q(400);
  auto producer = [&q]() -> void {
    for (size_t i = 0; i < 5000; ++i) {
      q.push(i);
      std::this_thread::sleep_for(std::chrono::microseconds(5));
    }
  };
  auto consumer = [&q](size_t& sum) -> void {
    sum = 0;
    for (size_t i = 0; i < 5000; ++i) {
      sum += q.pop();
    }
  };
  std::vector<std::thread> ths;
  size_t s1, s2, s3;
  ths.emplace_back(producer);
  ths.emplace_back(producer);
  ths.emplace_back(producer);
  ths.emplace_back(consumer, std::ref(s1));
  ths.emplace_back(consumer, std::ref(s2));
  ths.emplace_back(consumer, std::ref(s3));
  for (auto& t : ths) {
    t.join();
  }
  assert(s1 + s2 + s3 == 5000 * 4999 / 2 * 3);
  assert(q.try_pop(s1) == false);
}

int main(int argc, char** argv) {
  test01<std::mutex>();
  test01<utils::SpinMutex>();
  test02<std::mutex>();
  test02<utils::SpinMutex>();
  test03<std::mutex>();
  test03<utils::SpinMutex>();
  printf("access test!\n");
  return 0;
}