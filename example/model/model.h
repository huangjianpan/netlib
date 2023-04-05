#pragma once

#include <string>
#include <vector>

#include "json/json_gen.h"

namespace model {

class Req {
 public:
  Attribute("json:id") int id;
  Attribute("json:name") std::string name;
};

class Rsp {
 public:
  Attribute("json:data") std::vector<int> data;
};

}  // namespace model
