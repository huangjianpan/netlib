#include <assert.h>
#include <stdio.h>

#include <mutex>
#include <thread>
#include <vector>

#include "./utils/spin_mutex.h"

int cnt = 0;
utils::SpinMutex mtx;

void count() {
  for (int i = 0; i < 1000; ++i) {
    {
      std::lock_guard<util::SpinMutex> lock(mtx);
      ++cnt;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(20));
  }
}

int main(int argc, char** argv) {
  std::vector<std::thread> ths;
  for (int i = 0; i < 10; ++i) {
    ths.emplace_back(count);
  }
  for (auto& t : ths) {
    t.join();
  }
  assert(cnt == 10000);
  printf("access test\n");
  return 0;
}