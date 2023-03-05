#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "json.h"

std::string read_file() {
  std::fstream fs;
  fs.open("./input.json", std::ios_base::in);
  if (!fs.is_open()) {
    printf("open ./input.json failed!");
    exit(0);
  }
  std::string all;
  while (fs.good()) {
    std::string line;
    std::getline(fs, line);
    all += line;
  }
  fs.close();
  return all;
}

void test01(const std::string& raw) {
  {
    const char* errmsg = nullptr;
    json::Json j = json::Json::unmarshal(raw, errmsg);
    if (errmsg != nullptr) {
      std::cout << errmsg << std::endl;
    }
    std::cout << (double)j["bbb"] << std::endl;
    std::cout << j.marshal() << std::endl;
  }
  {
    json::Json j(json::Json::Object{});
    j.add("aaaa", "bbbb");
    j.add("bbb", 37563.33);
    j.add(std::string("kkk"), true);
    const char* k = "ccc";
    j.add(k, 100000000);
    std::string ret = j["aaaa"];
    j["aaaa"].move_to(ret);
    std::cout << ret << std::endl;
    std::cout << j.marshal() << std::endl;
  }
  {
    json::Json j(json::Json::Array{});
    j.add("aaa");
    j.add(345637657);
    j.add("aaa");

    json::Json j2(json::Json::Object{});
    j2.add("aaa", j);
    j2.add("bbb", 12563473);
    j2.add("ccc", j);
    std::cout << j2.marshal() << std::endl;
  }
}

class A {
 public:
  std::string f1;                         // f1
  double f2;                              // f2
  std::map<std::string, std::string> f3;  // f3
  std::vector<int> f4;                    // f4
};

namespace json {

inline void unmarshal(Json& j, std::string& ret) { j.move_to(ret); }

inline void unmarshal(Json& j, double& ret) { j.move_to(ret); }

inline void unmarshal(Json& j, std::map<std::string, std::string>& ret) {
  j.move_to(ret);
}

inline void unmarshal(Json& j, A& ret) {
  unmarshal(j["f1"], ret.f1);
  unmarshal(j["f2"], ret.f2);
  unmarshal(j["f3"], ret.f3);
}

inline const char* unmarshal(const char* raw, A& ret) {
  const char* errmsg = nullptr;
  json::Json j = json::Json::unmarshal(raw, errmsg);
  if (errmsg != nullptr) {
    return errmsg;
  }
  unmarshal(j, ret);
  std::cout << j.marshal() << std::endl;
  return nullptr;
}

}  // namespace json

int main(int argc, char** argv) {
  A a;
  json::unmarshal(read_file().c_str(), a);
  std::cout << a.f1 << std::endl;
  std::cout << a.f2 << std::endl;
  for (auto& kv : a.f3) {
    std::cout << kv.first << " " << kv.second << std::endl;
  }
  return 0;
}