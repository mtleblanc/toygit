#pragma once

#include "toygit/util.hpp"
#include <fcntl.h>
#include <filesystem>
#include <span>
#include <unistd.h>
#include <utility>
namespace toygit {

class File : MoveOnly {
public:
  File(int fd = -1) : fd_{fd} {}
  File(File &&o) : fd_{o.fd_} { o.fd_ = -1; }
  File &operator=(File &&o) {
    if (this != &o) {
      closeIfOpen();
      std::swap(fd_, o.fd_);
    }
    return *this;
  }
  ~File() { closeIfOpen(); }

  static Result<File> open(const std::filesystem::path &path, int flags) {
    auto fd = ::open(path.c_str(), flags);
    if (fd < 0)
      return errnoResult();
    return File(fd);
  }

  Result<void> writeAll(std::string_view sv);
  Result<int> read(std::span<std::byte> dst);
  Result<void> fsync();
  Result<void> close();

private:
  void closeIfOpen() {
    if (fd_ >= 0) {
      ::close(fd_);
      fd_ = -1;
    }
  }
  int fd_;
};
} // namespace toygit
