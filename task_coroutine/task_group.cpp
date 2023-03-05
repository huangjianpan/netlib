#include "task_group.h"

#include <assert.h>
#include <unistd.h>
#include <chrono>
#include <thread>

#include "task_context.h"
#include "task_control.h"

namespace task_coroutine {

thread_local TaskGroup* tls_task_group = nullptr;

TaskGroup::TaskGroup(TaskControl* task_control)
    : task_control_(task_control),
      main_task_(TaskMeta::main_task()),
      curr_task_(main_task_),
      done_task_(nullptr),
      yield_task_(nullptr) {
  assert(task_control != nullptr);
  assert(main_task_ != nullptr);
}

// BUG:
// co1依赖于co2，tg1运行co1遇到yield，进入wait_task，co1又没有入队，
// 这时恰好co2被tg2执行掉了且所有tg的sq都空，tg1就会陷入wait_task的死循环
void TaskGroup::wait_task(TaskMeta** task) {
again:
  if (sq_.try_pop(*task)) {
    return;
  }
  for (size_t i = 0; i < task_control_->task_groups_num(); ++i) {
    if (task_control_->choose_one_task_group()->sq_.try_pop(*task)) {
      return;
    }
  }
  // TODO: 暂时使用定时换醒
  std::this_thread::sleep_for(std::chrono::microseconds(1));
  goto again;
}

void TaskGroup::reschedule() {
  TaskGroup* g = tls_task_group;
  g->yield_task_ = g->curr_task_;
  g->curr_task_ = g->main_task_;
#ifdef TASK_COROUTINE_DEBUG
  sched_to(g->yield_task_, g->curr_task_, "reschedule");
#else
  sched_to(g->yield_task_, g->curr_task_);
#endif
  g = tls_task_group;  // 重新被调度，task_group可能变化
  g->try_destory_done_task();
}

void TaskGroup::run_main_task(TaskControl* task_control, size_t idx) {
  // 初始化task_group，即tls_task_group
  tls_task_group = new (std::nothrow) TaskGroup(task_control);
  assert(tls_task_group != nullptr);

  // 设置task_group并等待所有task_group初始化完成
  task_control->set_task_group(idx, tls_task_group);
  task_control->wait_init_task_groups_completed();

  TaskGroup* g = tls_task_group;
  TaskMeta* next_task;
  for (;;) {
    g->wait_task(&next_task);
    g->curr_task_ = next_task;

#ifdef TASK_COROUTINE_DEBUG
    sched_to(g->main_task_, next_task, "run_main_task");
    assert(g == tls_task_group);  // main_task所属的task_group才能运行main_task
#else
    sched_to(g->main_task_, next_task);
#endif

    if (g->yield_task_ != nullptr) {  // reschedule后重新入队
      g_task_control->choose_one_task_group()->sq_.push(g->yield_task_);
      g->yield_task_ = nullptr;
    }
  }

  return;
}

void TaskGroup::jump_fn() {
  TaskGroup* g = tls_task_group;
  // 1. 运行当前task
  TaskMeta* curr_task = g->curr_task_;
#ifdef TASK_COROUTINE_DEBUG
  // 标记已经运行过
  if (curr_task->state.fetch_or(TaskMeta::state_start_run) &
      TaskMeta::state_start_run) {
    printf(
        "%s:%d thread_id = %lu, re run {id = %lu}{stack = "
        "%p}\n",
        __FILE__, __LINE__,
        std::hash<std::thread::id>()(std::this_thread::get_id()), curr_task->id,
        curr_task->stack);
    abort();
  } else {
    printf("%s:%d thread_id = %lu, run {id = %lu}{stack = %p}\n", __FILE__,
           __LINE__, std::hash<std::thread::id>()(std::this_thread::get_id()),
           curr_task->id, curr_task->stack);
  }
#endif
  curr_task->run();
  g = tls_task_group;  // 有可能换出后重新调度回来，task_group发生变化
  // 2. 释放上一个运行完成的task的资源
  g->try_destory_done_task();
  // 3. 设置需要释放的task
  g->done_task_ = curr_task;
  // 4. 获取下一个运行的task
  TaskMeta* next_task;
  g->wait_task(&next_task);
  // 5. 设置当前运行的task
  g->curr_task_ = next_task;
  // 6. 保存上下文，切换栈
#ifdef TASK_COROUTINE_DEBUG
  printf("%s:%d thread_id = %lu, done {id = %lu}{stack = %p}\n", __FILE__,
         __LINE__, std::hash<std::thread::id>()(std::this_thread::get_id()),
         curr_task->id, curr_task->stack);
  sched_to(curr_task, next_task, "jump_fn");
#else
  sched_to(curr_task, next_task);
#endif
}

}  // namespace task_coroutine
