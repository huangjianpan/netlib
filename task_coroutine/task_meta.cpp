#include "task_meta.h"

#ifdef TASK_COROUTINE_DEBUG
namespace task_coroutine {
std::atomic<size_t> g_task_meta_created_count(0); // 总共创建的task_meta数量
std::atomic<size_t> g_task_meta_destroy_count(0); // 总共销毁的task_meta数量
}  // namespace task_coroutine
#endif