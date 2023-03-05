#ifndef _TASK_COROUTINE_TASK_GROUP_H_
#define _TASK_COROUTINE_TASK_GROUP_H_
#include <assert.h>

#include "task_meta.h"
#include "task_scheduling_queue.hpp"

namespace task_coroutine {

class TaskControl;

class TaskGroup {
 public:
  TaskGroup(TaskControl* task_control);

  // wait_task 等待获取任务
  void wait_task(TaskMeta** task);

  // add_task 添加任务
  TaskMeta* add_task(void* fn(void*), void* arg) {
    TaskMeta* task = TaskMeta::new_task(fn, arg, TaskGroup::jump_fn);
    if (task == nullptr) {
      return nullptr;
    }
    sq_.push(task);
    return task;
  }

  // try_destory_done_task 修改done_task的状态，并尝试释放资源
  void try_destory_done_task() {
    if (done_task_ != nullptr) {
      // TODO: 添加memory_order
      if (done_task_->state.fetch_or(
              TaskMeta::state_task_group_sched_next_one) &
          TaskMeta::state_coroutine_destructor) {
        TaskMeta::destory(done_task_);
      }
      done_task_ = nullptr;
    }
  }

  // reschedule 回到主函数，重新调度
  // 回到主函数的原因：只有主函数接下来执行的代码是明确的，其他task有可能是进入jump_fn或回到换出点
  static void reschedule();

  // sched_to 从from调度/切换到to，切换栈和上下文
  static void sched_to(TaskMeta* from, TaskMeta* to) {
    task_coroutine_jump_fcontext(&from->stack, to->stack);
  }

#ifdef TASK_COROUTINE_DEBUG
  static void sched_to(TaskMeta* from, TaskMeta* to, const char* msg) {
    size_t thread_id = std::hash<std::thread::id>()(std::this_thread::get_id());
    printf(
        "%s:%d thread_id = %lu, %s from{id = %lu}{stack = %p},to{id = %lu}{"
        "stack = %p}\n",
        __FILE__, __LINE__, thread_id, msg, from->id, from->stack, to->id,
        to->stack);
    task_coroutine_jump_fcontext(&from->stack, to->stack);
  }
#endif

  // run_main_task task_group线程（worker thread）运行的主函数
  static void run_main_task(TaskControl* task_control, size_t idx);

  // jump_fn
  // 调度一个新的task时跳转的函数，设置在new_task的上下文中，如果是重新入队后调度的task，会回到上次运行的地方
  static void jump_fn();

 private:
  TaskSchedulingQueue<TaskMeta*> sq_;  // 调度队列
  TaskControl* task_control_;          // 所属的task_control
  TaskMeta* main_task_;                // 线程main函数
  TaskMeta* curr_task_;                // 当前运行的task
  TaskMeta* done_task_;   // 上一个执行完成的task，需要释放内存
  TaskMeta* yield_task_;  // 被换出的task，用于reschedule重新调度
};

extern thread_local TaskGroup* tls_task_group;  // 每个工作线程的task_group

}  // namespace task_coroutine

#endif  // !_TASK_COROUTINE_TASK_GROUP_H_