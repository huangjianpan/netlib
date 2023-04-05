#include "task_control.h"

#include <assert.h>
#include <malloc.h>

#include "task_group.h"

namespace task_coroutine {

TaskControl* g_task_control = nullptr;

thread_local utils::RandomNumber tls_random_number(
    0, TaskControl::task_groups_num() - 1);

TaskControl::Init::Init() {
  g_task_control =
      new (std::nothrow) TaskControl();
  assert(g_task_control != nullptr);
  g_task_control->start_worker_threads();
}

TaskControl::Init TaskControl::init_;

TaskControl::TaskControl()
    : init_success_num_(0) {
  worker_threads_ = new (std::nothrow) std::thread[task_groups_num()];
  task_groups_ = new (std::nothrow) TaskGroup*[task_groups_num()];
  parking_lots_ = new (std::nothrow) ParkingLot[parking_lots_num()];
  assert(worker_threads_ != nullptr && task_groups_ != nullptr &&
         parking_lots_ != nullptr);
}

TaskControl::~TaskControl() {
  for (size_t i = 0; i < task_groups_num(); ++i) {
    worker_threads_[i].join();
  }
  delete[] worker_threads_;
  delete[] task_groups_;
  delete[] parking_lots_;
}

void TaskControl::start_worker_threads() {
  for (size_t i = 0; i < task_groups_num(); ++i) {
    // 开启工作线程，启动函数为run_main_task
    worker_threads_[i] = std::thread(&TaskGroup::run_main_task, this, i);
  }
  wait_init_task_groups_completed();
}

void TaskControl::set_task_group(size_t i, TaskGroup* task_group) {
  task_groups_[i] = task_group;
  init_success_num_.fetch_add(1, std::memory_order_release);
}

void TaskControl::wait_init_task_groups_completed() const {
  while (init_success_num_.load(std::memory_order_acquire) != task_groups_num())
    ;
}

}  // namespace task_coroutine