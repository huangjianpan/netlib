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
  json::unmarshal(read_file(), r);
  std::cout << json::marshal(std::move(r)) << std::endl;
  std::cout << json::marshal(r) << std::endl;
  return 0;
}