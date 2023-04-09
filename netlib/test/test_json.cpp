#include <cstdio>
#include <cstring>
#include <string>

#include "json/json.h"

const char* str =
    "{\"name\":\"hjp\",\"id\":123456,\"hobbies\":[\"music\",\"games\"],\"is_"
    "student\":null}";

int main(int argv, char** argc) {
  const char* errmsg = nullptr;
  json::Json j = json::Json::unmarshal(str, str + strlen(str), errmsg);
  if (errmsg != nullptr) {
    printf("errmsg is %s\n", errmsg);
    return 0;
  }
  printf("name is %s\n", static_cast<std::string>(j["name"]).c_str());
  printf("id is %d\n", static_cast<int>(j["id"]));
  size_t len = j["hobbies"].size();
  printf("the length of hobbies is %d\n", len);
  for (size_t i = 0; i < len; ++i) {
    printf("    hobbies[%d] is %s\n", i,
           static_cast<std::string>(j["hobbies"][i]).c_str());
  }
  printf("j[\"is_student\"] is null: %d\n", j["is_student"].is_null());
  return 0;
}