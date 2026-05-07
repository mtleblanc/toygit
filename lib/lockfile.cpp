#include "toygit/file.hpp"
#include <fcntl.h>
#include <filesystem>
#include <unistd.h>
#include <utility>
namespace toygit {

class Lockfile {
public:
  Lockfile(std::filesystem::path file);
  ~Lockfile();

  Result<void> write(std::string_view sv);
  Result<void> commit();
  Result<void> release();

private:
  enum class State { Open, Committed, Released };

  State state_ = State::Open;
  std::filesystem::path file_;
  std::filesystem::path lockFile_;
  File lf_;
};

Lockfile::Lockfile(std::filesystem::path file)
    : file_{file}, lockFile_{std::move(file)} {
  lockFile_ += ".lock";
  auto fd = open(lockFile_.c_str(), O_CREAT | O_EXCL);
  if (fd < 0) {
    throw std::make_error_code(static_cast<std::errc>(errno));
  }
  lf_ = File{fd};
}

Lockfile::~Lockfile() {
  switch (state_) {

  case State::Open:
    (void)release();
    break;
  case State::Committed:
  case State::Released:
    break;
  }
}

Result<void> Lockfile::write(std::string_view sv) { return lf_.writeAll(sv); }

Result<void> Lockfile::commit() {
  (void)lf_.fsync();
  (void)lf_.close();
  std::error_code ec;
  std::filesystem::rename(lockFile_, file_, ec);
  if (ec) {
    return std::unexpected(ec);
  }
  return {};
}

Result<void> Lockfile::release() {
  (void)lf_.close();
  std::error_code ec;
  std::filesystem::remove(lockFile_, ec);
  if (ec) {
    return std::unexpected(ec);
  }
  return {};
}
} // namespace toygit
