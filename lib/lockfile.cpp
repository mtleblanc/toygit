#include <fcntl.h>
#include <filesystem>
#include <unistd.h>
#include <utility>
namespace toygit {

class Lockfile {
public:
  Lockfile(std::filesystem::path file)
      : file_{file}, lockFile_{std::move(file)} {
    lockFile_ += ".lock";
    lf_ = open(lockFile_.c_str(), O_CREAT | O_EXCL);
    if (lf_ < 0) {
      throw lf_;
    }
  }
  ~Lockfile() {
    close(lf_);
    std::error_code ec;
    std::filesystem::rename(lockFile_, file_, ec);
  }

private:
  std::filesystem::path file_;
  std::filesystem::path lockFile_;
  int lf_;
};

} // namespace toygit
