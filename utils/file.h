#pragma once

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <vector>

namespace utils {
namespace file {

// Dirent 不保证生命周期，取决于所属于的Directory
class Dirent {
 public:
  Dirent(struct dirent* dir_entry) : dirent_(dir_entry) {}

  Dirent(const Dirent&) = delete;

  Dirent& operator=(const Dirent&) = delete;

  Dirent(Dirent&& rhs) : dirent_(rhs.dirent_) {}

  Dirent& operator=(Dirent&& rhs) {
    dirent_ = rhs.dirent_;
    return *this;
  }

  bool exist() const { return dirent_ != nullptr; }

  bool is_file() const { return dirent_->d_type == 8; }

  bool is_link() const { return dirent_->d_type == 10; }

  bool is_dir() const { return dirent_->d_type == 4; }

  const char* name() const { return dirent_->d_name; }

 private:
  struct dirent* dirent_;
};

class Directory {
 public:
  Directory(const char* path = nullptr) : directory_(nullptr) { open(path); }

  Directory(const Directory&) = delete;

  ~Directory() { close(); }

  Directory& operator=(const Directory&) = delete;

  bool exist() const { return directory_ != nullptr; }

  bool open(const char* path);

  bool close();

  bool create(const char* path);

  const std::vector<Dirent>& dirents();

 private:
  DIR* directory_;
  std::vector<Dirent> dirents_;
};

class File {
 public:
  File(const char* path = nullptr) : fd_(-1) { open(path); }

  File(const File&) = delete;

  File& operator=(const File&) = delete;

  ~File() { close(); }

  int fd() const { return fd_; }

  bool exist() const { return fd_ >= 0; }

  bool open(const char* path);

  bool close();

  bool create(const char* path);

 private:
  int fd_;
};

}  // namespace file
}  // namespace utils
