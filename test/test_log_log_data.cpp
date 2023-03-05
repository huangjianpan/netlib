#include <cassert>
#include <cstring>
#include <string>

#include "log/log_data.h"

void test01() {
  using namespace logger;
  {
    const char* p = "aaaa";
    LogData s(p);
    assert(s.type() == LogData::Type::CString && s.data() == p);
  }
  {
    std::string s("11111111111111111111111111111111111111111111");
    const char* ptr = s.c_str();
    LogData proxy(s);
    assert(proxy.type() == LogData::Type::CXXString && proxy.data() != ptr);
  }
  {
    std::string s("11111111111111111111111111111111111111111111");
    const char* ptr = s.c_str();
    LogData proxy(std::move(s));
    assert(proxy.type() == LogData::Type::CXXString && proxy.data() == ptr);
  }
  {
    LogID log_id = LogID::generate();
    LogData proxy(log_id);
    assert(proxy.type() == LogData::Type::LogID);
    assert(strcmp(log_id.c_str(), proxy.data()) == 0);
  }
}

int main(int argc, char** argv) {
  test01();
  printf("access test!\n");
  return 0;
}