#include "toygit/lockfile.hpp"
#include "toygit/util.hpp"
#include <fcntl.h>
#include <print>
#include <system_error>
#include <tuple>
#include <utility>

namespace toygit {
Lockfile::Lockfile(std::filesystem::path file)
    : file_{file}, lockFile_{std::move(file)} {
  lockFile_ += ".lock";
  auto fd = ::open(lockFile_.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0644);
  if (fd < 0) {
    throw std::system_error(std::error_code{errno, std::system_category()},
                            "Creating lockfile");
  }
  lf_ = File{fd};
}

Lockfile::~Lockfile() {
  switch (state_) {

  case State::Open:
    // TODO: can't do much if this fails in the destructor, but we should at
    // least log
    std::ignore = release();
    break;
  case State::Committed:
  case State::Released:
    break;
  }
}

Result<void> Lockfile::write(std::string_view sv) { return lf_.writeAll(sv); }

Result<void> Lockfile::commit() {
  return lf_.fsync()
      .and_then([this] { return lf_.close(); })
      .and_then([this] -> Result<void> {
        TRY_EC(std::filesystem::rename, lockFile_, file_);
        state_ = State::Committed;
        return {};
      });
}

Result<void> Lockfile::release() {
  return lf_.close().and_then([this] -> Result<void> {
    std::ignore = TRY_EC(std::filesystem::remove, lockFile_);
    state_ = State::Released;
    return {};
  });
}
} // namespace toygit
