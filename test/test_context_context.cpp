#include <cassert>
#include <cstdio>
#include <string>
#include <thread>

#include "context/context.h"

void test01() {
  static bool has_deleted = false;
  static bool has_deleted_2 = false;
  {
    context::Context ctx;
    {
      ctx.set_key_value_non_thread_safe("key1", (void*)(int)1, nullptr);
      int value = (int)(long long)ctx.get_value_non_thread_safe("key1");
      assert(value == 1);
    }
    {
      ctx.set_key_value_non_thread_safe("2", new std::string("aaaaa"),
                                        [](void* value) -> void {
                                          auto p_s = (std::string*)value;
                                          assert(*p_s == "aaaaa");
                                          delete p_s;
                                          has_deleted = true;
                                          return;
                                        });
    }
    {
      const std::string& s = *(std::string*)ctx.get_value_non_thread_safe("2");
      assert(s == "aaaaa");
    }
    {
      ctx.set_key_value_non_thread_safe("2", new int(1), [](void* p) -> void {
        auto p_v = (int*)p;
        assert(*p_v == 1);
        delete p_v;
        has_deleted_2 = true;
      });
    }
    {
      int value = *(int*)ctx.get_value_non_thread_safe("2");
      assert(value == 1);
    }
  }
  if (!has_deleted || !has_deleted_2) {
    abort();
  }
}

void test02() {
  static size_t count = 0;
  constexpr size_t num = 10000;
  {
    context::Context ctx;
    auto f = [](context::Context& ctx, const std::string& s,
                size_t num) -> void {
      for (size_t i = 0; i < num; ++i) {
        ctx.set_key_value("key", new std::string(s), [](void* p) -> void {
          std::string* pv = (std::string*)p;
          delete pv;
          ++count;
        });
        std::this_thread::sleep_for(std::chrono::microseconds(5));
      }
    };
    std::thread t1(f, std::ref(ctx), "aaa", num);
    std::thread t2(f, std::ref(ctx), "bbb", num);
    t1.join();
    t2.join();
    assert(count == 2 * num - 1);
    assert(*(std::string*)ctx.get_value("key") == "aaa" || *(std::string*)ctx.get_value("key") == "bbb");
  }
  assert(count == 2 * num);
}

int main(int argc, char** argv) {
  test01();
  test02();
  printf("access test!\n");
  return 0;
}