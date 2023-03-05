#ifndef _TASK_COROUTINE_H_
#define _TASK_COROUTINE_H_

#include <assert.h>
#include "task_control.h"
#include "task_group.h"

namespace task_coroutine {

struct InitTaskCoroutine {
  InitTaskCoroutine() {
    g_task_control =
        new (std::nothrow) TaskControl(TASK_COROUTINE_TASK_GROUP_NUM);
    assert(g_task_control != nullptr);
    g_task_control->start_worker_threads();
  }
};

extern InitTaskCoroutine init_task_coroutine__;

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
      // TODO:
      // 目前单个线程不会死锁
      size_t i = 0;
      while ((task_meta_->state.load(std::memory_order_relaxed) &
              TaskMeta::state_fn_done) == 0) {
        if (++i == 1000 && tls_task_group != nullptr) {
          yield();
          i = 0;  // 重新计数
        }
      }
      try_destory();
    }
  }

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

  // yield 换出当前coroutine
  void yield() { TaskGroup::reschedule(); }

  TaskMeta* task_meta_;
};

}  // namespace task_coroutine

#endif  // !_TASK_COROUTINE_H_