#pragma once

#include "define.h"

#include <stdlib.h>
#include <atomic>
#include <cstdio>
#include <new>

#ifdef TASK_COROUTINE_DEBUG
#include <thread>
#endif

#include "task_context.h"

namespace task_coroutine {

#ifdef TASK_COROUTINE_DEBUG
extern std::atomic<size_t> g_task_meta_created_count;
extern std::atomic<size_t> g_task_meta_destroy_count;
#endif

// 关键问题
// 1. 哪些地方会使task_meta运行时的task_group发生变化？
// task_meta运行时被换出，重新被调度到的时候，第一次运行时的task_group和第二次的task_group可能不同
// 如果task_meta的栈上保存了tls_task_group变量，则需要刷新一下
//
// 2. task_meta的回收点？
// （1）运行task结束
// （2）从换出点切回
// （3）reschedule进入到run_main_task

// TODO:
// 1. 优化栈分配，懒加载

// TaskMeta
// 生命周期由与其关联的coroutine和将其执行完成的task_group共同管理，原因：
// 1. coroutine join时需要task_meta存在，在coroutine未析构时，task_meta不能析构
// 2. coroutine析构时，task_meta未必执行，需要task_group来析构
// 析构时机：
// 1. task_meta析构必须是已经完成并且task_group切换到其他task_meta时
// 2. 关联的coroutine已经析构，则task_group执行下一个task_meta时执行析构
// 3. task_group执行完该task_meta，开始执行下一个task_meta后，关联的coroutine析构/join时将其析构
struct TaskMeta {
  void* (*fn)(void*);
  void* arg;
  void* stack;
  void* memory;
  std::atomic<size_t> state;
  // state 3位bit表示
  //   100  完成fn(arg)
  //   010  所属Coroutine调用析构
  //   001  完成该task_meta的task_group调度下一个task
  // 所属Coroutine join的条件为 state & 0x04
  // 销毁的条件为所属Coroutine析构 && 运行其的task_group调度下一个task，即 state
  // & 0x03 == 0x03

#ifdef TASK_COROUTINE_DEBUG
  size_t id;

  static constexpr size_t state_had_destory = 1 << 4;
  static constexpr size_t state_start_run = 1 << 3;
#endif

  static constexpr size_t state_fn_done = 1 << 2;
  static constexpr size_t state_coroutine_destructor = 1 << 1;
  static constexpr size_t state_task_group_sched_next_one = 1;

  TaskMeta(void* (*fn_)(void*), void* arg_, void* stack_, void* memory_)
      : fn(fn_), arg(arg_), stack(stack_), memory(memory_), state(0) {}

  TaskMeta(const TaskMeta&) = delete;
  TaskMeta& operator=(const TaskMeta&) = delete;

  ~TaskMeta() { free(memory); }

  // run 运行函数
  void run() {
    fn(arg);
    // TODO: 添加memory_order
    state.fetch_or(state_fn_done);
  }

  // main_task 创建主函数的task
  static TaskMeta* main_task() {
    return new (std::nothrow) TaskMeta{nullptr, nullptr, nullptr, nullptr};
  }

  // new_task 创建任务
  // 参数：fn是运行函数，arg是函数参数，jump_fn是jump_fcontext时跳转的函数
  // 返回值：使用null方法判空
  static TaskMeta* new_task(void* (*fn)(void*), void* arg, void (*jump_fn)()) {
    constexpr size_t stacksize = 1024 * 8;
    void* m = malloc(stacksize);
    if (m == nullptr) {
      return nullptr;
    }
    TaskMeta* task_meta = new (std::nothrow) TaskMeta{
        fn, arg, task_coroutine_make_fcontext((char*)m + stacksize, jump_fn),
        m};  // 注意stack的bottom在memory +
             // stacksize，因为栈增长的方向是地址下降
    if (task_meta == nullptr) {
      free(m);
      return nullptr;
    }
#ifdef TASK_COROUTINE_DEBUG
    // 初始化task_meta的id
    task_meta->id = g_task_meta_created_count.fetch_add(1, std::memory_order_relaxed) + 1;
#endif
    return task_meta;
  }

  // destory 删除任务
  static void destory(TaskMeta* task_meta) {
#ifdef TASK_COROUTINE_DEBUG
    // 标记已经删除
    if (task_meta->state.fetch_or(TaskMeta::state_had_destory) &
        TaskMeta::state_had_destory) {
      printf("%s:%d thread_id = %lu, re destory {id = %lu}\n", __FILE__,
             __LINE__, std::hash<std::thread::id>()(std::this_thread::get_id()),
             task_meta->id);
      abort();
    }
    g_task_meta_destroy_count.fetch_add(1, std::memory_order_relaxed);
#endif
    delete task_meta;
  }
};

}  // namespace task_coroutine