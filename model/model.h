#pragma once

#include <string>
#include <map>

#include "json_gen.h"

namespace model {

class Location {
 public:
  Attribute("json:latitude") double lat;
  Attribute("json:longitude") double lng;
};

class Req {
 public:
  Attribute("json:id") int id;
  Attribute("json:name") std::string name;
  Attribute("json:src") Location src;
  Attribute("json:dest") Location dest;
  Attribute("json:extra") std::map<std::string, Location> extra;
};

}  // namespace model