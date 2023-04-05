#pragma once

#include <atomic>
#include <random>
#include <thread>

#include "define.h"
#include "task_parking_lot.h"
#include "utils/random_number.h"

namespace task_coroutine {

class TaskGroup;
class TaskControl;

extern TaskControl* g_task_control;
extern thread_local utils::RandomNumber
    tls_random_number;  // 随机数生成器每个线程一个

class TaskControl {
  struct Init {
    Init();
  };

  static Init init_;

 private:
  static constexpr size_t TASK_GROUPS_NUM = 8;
  static constexpr size_t PARKING_LOTS_NUM = 2;

 public:
  TaskControl();

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

  ParkingLot* alloc_parking_lot(size_t idx) {
    return &parking_lots_[idx / ((task_groups_num() + parking_lots_num() - 1) /
                                 parking_lots_num())];
  }

  static constexpr size_t task_groups_num() { return TASK_GROUPS_NUM; }

  static constexpr size_t parking_lots_num() { return PARKING_LOTS_NUM; }

 private:
  TaskGroup** task_groups_;      // task groups
  ParkingLot* parking_lots_;     // parking lots
  std::thread* worker_threads_;  // the thread to which the task group belongs

  std::atomic<size_t> init_success_num_;  // for init task_group
};

}  // namespace task_coroutine