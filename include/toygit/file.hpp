
#include "toygit/util.hpp"
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

  Result<void> writeAll(std::string_view sv);
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
