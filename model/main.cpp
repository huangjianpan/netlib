#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "json.h"
#include "model.h"
#include "model_gen.h"

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

int main() {
  model::Req r;
  json::unmarshal(read_file().c_str(), r);
  std::cout << r.id << std::endl;
  std::cout << r.name << std::endl;
  std::cout << r.src.lat << std::endl;
  std::cout << r.src.lng << std::endl;
  for (auto& kv : r.extra) {
    std::cout << kv.first << " " << kv.second.lng << " " << kv.second.lat
              << std::endl;
  }
  return 0;
}