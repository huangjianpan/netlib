#pragma once

extern "C" {
void task_coroutine_jump_fcontext(void** from, void* to);

void* task_coroutine_make_fcontext(void* stack, void (*fn)());
}
