#include "file.h"

#include <fcntl.h>

#include <cstdlib>

#include "./fmt.h"

namespace utils {
namespace file {

bool Directory::open(const char* path) {
  close();
  if (path != nullptr) {
    if (::access(path, F_OK) != 0) {
      return false;
    }
    directory_ = ::opendir(path);
  }
  return directory_ != nullptr;
}

bool Directory::close() {
  dirents_.clear();
  int ret = 0;
  if (exist()) {
    ret = ::closedir(directory_);
    directory_ = nullptr;
  }
  return ret == 0;
}

bool Directory::create(const char* path) {
  close();
  if (path == nullptr || ::access(path, F_OK) == 0) {
    return false;
  }
  std::string cmd = utils::fmt::sprintf("mkdir -p %s", path);
  int status = ::system(cmd.c_str());
  return status == 0;
}

const std::vector<Dirent>& Directory::dirents() {
  if (exist()) {
    struct dirent* entry = nullptr;
    while ((entry = ::readdir(directory_)) != nullptr) {
      dirents_.emplace_back(entry);
    }
  }
  return dirents_;
}

bool File::open(const char* path) {
  close();
  if (path != nullptr) {
    fd_ = ::open(path, O_RDWR | O_CLOEXEC, 0664);
  }
  return fd_ >= 0;
}

bool File::close() {
  int ret = 0;
  if (exist()) {
    ret = ::close(fd_);
    fd_ = -1;
  }
  return ret == 0;
}

bool File::create(const char* path) {
  close();
  if (path != nullptr) {
    if (::access(path, F_OK) == 0) {
      return false;
    }
    fd_ = ::open(path, O_CREAT | O_RDWR | O_CLOEXEC, 0664);
  }
  return fd_ >= 0;
}

}  // namespace file
}  // namespace utils