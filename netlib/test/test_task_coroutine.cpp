#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "task_coroutine/task_coroutine.h"

// valgrind --tool=memcheck --leak-check=full ./main

void* foo2(void* arg) {
  printf("foo2\n");
  return nullptr;
}

void* foo(void* arg) {
  int* j = static_cast<int*>(arg);
  // printf("%d\n", *j);
  delete j;

  char ch[100];
  for (size_t t = 0; t < 26; ++t) {
    ch[t] = 'a' + t;
  }
  ch[26] = '\0';
  // printf("%s\n", ch);
  {
    task_coroutine::Coroutine c(foo2, nullptr);
    c.join();
  }
  for (size_t t = 0; t < 26; ++t) {
    ch[t] = '*';
  }
  // printf("%s\n", ch);

  return nullptr;
}

int main(int argc, char** argv) {
  std::vector<task_coroutine::Coroutine> cs;
  for (int j = 0; j < 10000; ++j) {
    cs.emplace_back(foo, new int(j));
  }
  for (auto& c : cs) {
    c.join();
  }

#ifdef TASK_COROUTINE_DEBUG
  auto delta =
      task_coroutine::g_task_meta_created_count.load(
          std::memory_order_relaxed) -
      task_coroutine::g_task_meta_destroy_count.load(std::memory_order_relaxed);
  if (delta < 0 || delta > TASK_COROUTINE_TASK_GROUP_NUM) {
    abort();
  }
#endif

  printf("access test\n");
  return 0;
}