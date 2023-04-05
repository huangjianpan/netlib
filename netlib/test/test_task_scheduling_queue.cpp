#include <assert.h>
#include <malloc.h>
#include <unistd.h>
#include <cstdio>
#include <thread>

#include "./task_coroutine/task_scheduling_queue.hpp"

task_coroutine::TaskSchedulingQueue<int> q;

void fn1() {
  for (size_t i = 0; i < 1000; ++i) {
    q.push(i);
    usleep(1000);
  }
}

void fn2(int num) {
  for (int i = 0; i < num * 1000; ++i) {
    q.pop();
  }
}

int main(int argc, char** argv) {
  // delete task_coroutine::g_task_control;

  auto t1 = std::thread(fn1);
  auto t2 = std::thread(fn1);

  auto t3 = std::thread(fn1);
  auto t4 = std::thread(fn2, 3);

  t1.join();
  t2.join();
  t3.join();
  t4.join();

  int ret = 0;
  assert(q.try_pop(ret) == false);

  printf("access test\n");

  return 0;
}