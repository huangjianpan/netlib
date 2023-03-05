#ifndef _TASK_COROUTINE_TASK_CONTROL_H_
#define _TASK_COROUTINE_TASK_CONTROL_H_
#include <atomic>
#include <random>
#include <thread>

#include "define.h"
#include "utils/random_number.h"

namespace task_coroutine {

class TaskGroup;
class TaskControl;

extern TaskControl* g_task_control;
extern thread_local utils::RandomNumber
    tls_random_number;  // 随机数生成器每个线程一个

class TaskControl {
 public:
  TaskControl(const size_t task_groups_num);

  TaskControl(const TaskControl&) = delete;
  TaskControl& operator=(const TaskControl&) = delete;

  ~TaskControl();

  // start_worker_threads 开启所有工作线程
  void start_worker_threads();

  void set_task_group(size_t i, TaskGroup* task_group);

  void wait_init_task_groups_completed() const;

  // choose_one_task_group 并发安全，使用tls变量
  TaskGroup* choose_one_task_group() {
    return task_groups_[tls_random_number.generate()];
  }

  size_t task_groups_num() const { return task_groups_num_; }

 private:
  TaskGroup** task_groups_;       // task_group数组
  const size_t task_groups_num_;  // task_group数量
  std::thread* worker_threads_;   // task_group对应的thread

  std::atomic<size_t> init_success_num_;  // 初始化worker线程数量
};

}  // namespace task_coroutine

#endif  // !_TASK_COROUTINE_TASK_CONTROL_H_