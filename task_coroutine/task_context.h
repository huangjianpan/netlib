#ifndef __TASK_COROUTINE_TASK_CONTEXT_H__
#define __TASK_COROUTINE_TASK_CONTEXT_H__

extern "C" {
void task_coroutine_jump_fcontext(void** from, void* to);

void* task_coroutine_make_fcontext(void* stack, void (*fn)());
}

#endif  // !__TASK_COROUTINE_TASK_CONTEXT_H__