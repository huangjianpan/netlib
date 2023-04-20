#include <iostream>

#include "model.h"
#include "model_gen.h"

// g++ -std=c++17 -Wall main.cpp ../netlib/json/json.cpp -I ../netlib/json -I ../netlib/

int main() {
  A a;
  a.id = 12345;
  a.name = "hjp";
  std::cout << json::marshal(a) << std::endl;

  std::string s = json::marshal(a);
  A b;
  json::unmarshal(s, b);
  std::cout << b.id << " " << b.name << std::endl;
}