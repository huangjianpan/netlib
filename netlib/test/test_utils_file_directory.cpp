#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "utils/file.h"

const char* file_type(const utils::file::Dirent& dirent) {
  if (dirent.is_file()) {
    return "file";
  } else if (dirent.is_dir()) {
    return "directory";
  } else if (dirent.is_link()) {
    return "link";
  }
  return "unknow";
}

void test01(const char* path) {
  utils::file::Directory dir;
  assert(dir.close() && !dir.exist());

  assert(dir.open(path));
  assert(dir.exist());

  for (const auto& dirent : dir.dirents()) {
    printf("%s, %s\n", dirent.name(), file_type(dirent));
  }

  char buf[1024] = {0};
  strcpy(buf, path);
  int len = strlen(buf);
  strcat(buf, "/core/log");

  assert(dir.create(buf));

  buf[len] = '\0';
  strcat(buf, "/core");
  assert(!dir.create(buf));
  assert(!dir.exist());

  strcat(buf, "/log/20230225_1.log");
  utils::file::File file(buf);
  assert(!file.exist());
  file.create(buf);
  assert(file.exist());
  assert(file.fd() >= 0);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("usage: ./main <directory>\n");
    return 0;
  }
  test01(argv[1]);
  printf("access test!\n");
  return 0;
}