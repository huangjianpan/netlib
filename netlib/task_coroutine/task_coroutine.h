#pragma once

#include <assert.h>

#include "task_control.h"
#include "task_group.h"

namespace task_coroutine {

class Coroutine {
 public:
  Coroutine(void* fn(void*), void* arg) {
    TaskGroup* tg = g_task_control->choose_one_task_group();
    task_meta_ = tg->add_task(fn, arg);
    // TODO: 如果创建失败，原地调用，在非main_task中运行有栈溢出风险
    if (task_meta_ == nullptr) {
      fn(arg);
    }
  }

  Coroutine(Coroutine&& c) : task_meta_(c.task_meta_) {
    c.task_meta_ = nullptr;
  }

  ~Coroutine() {
    if (task_meta_ != nullptr) {
      try_destory();
    }
  }

  Coroutine(const Coroutine&) = delete;
  Coroutine& operator=(const Coroutine&) = delete;

  Coroutine& operator=(Coroutine&& rhs) {
    task_meta_ = rhs.task_meta_;
    rhs.task_meta_ = nullptr;
    return *this;
  }

  // join 等待coroutine完成
  void join() {
    if (task_meta_ != nullptr) {
      while ((task_meta_->state.load(std::memory_order_relaxed) &
              TaskMeta::state_fn_done) == 0) {
        yield();
      }
      try_destory();
    }
  }

  // yield 换出当前coroutine
  static void yield() { TaskGroup::reschedule(); }

 private:
  // try_destory 尝试销毁task_meta，修改task_meta的状态，在join和dtor时调用
  void try_destory() {
    // TODO: 添加memory_order
    if (task_meta_->state.fetch_or(TaskMeta::state_coroutine_destructor) &
        TaskMeta::state_task_group_sched_next_one) {
      TaskMeta::destory(task_meta_);
    }
    task_meta_ = nullptr;
  }

  TaskMeta* task_meta_;
};

}  // namespace task_coroutine