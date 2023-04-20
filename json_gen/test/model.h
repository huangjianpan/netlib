#pragma once

#include "json/json_gen.h"

#include <string>
#include <cstdio>
#include <map>
#include <vector>

class A {
public:
    Attribute("json:id") std::map<std::string, std::vector<size_t>> hjp_test;
    Attribute("json:name") std::string name;
};