#include "toygit/lockfile.hpp"
#include <fcntl.h>
#include <tuple>
#include <utility>

namespace toygit {
Lockfile::Lockfile(std::filesystem::path file)
    : file_{file}, lockFile_{std::move(file)} {
  lockFile_ += ".lock";
  auto fd = open(lockFile_.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0644);
  if (fd < 0) {
    throw std::make_error_code(static_cast<std::errc>(errno));
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
        std::error_code ec;
        std::filesystem::rename(lockFile_, file_, ec);
        if (ec) {
          return std::unexpected(ec);
        }
        state_ = State::Committed;
        return {};
      });
}

Result<void> Lockfile::release() {
  return lf_.close().and_then([this] -> Result<void> {
    std::error_code ec;
    std::filesystem::remove(lockFile_, ec);
    if (ec) {
      return std::unexpected(ec);
    }
    state_ = State::Released;
    return {};
  });
}
} // namespace toygit
