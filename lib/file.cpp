#include "toygit/file.hpp"
#include <expected>
#include <span>

namespace toygit {

namespace {
auto err() {
  return std::unexpected{std::make_error_code(static_cast<std::errc>(errno))};
}
} // namespace

Result<void> File::writeAll(std::string_view sv) {
  while (!sv.empty()) {
    auto written = ::write(fd_, sv.data(), sv.size());
    if (written < 0) {
      return err();
    }
    sv = sv.substr(written);
  }
  return {};
}

Result<int> File::read(std::span<std::byte> dst) {
  auto result = ::read(fd_, dst.data(), dst.size());
  if (result < 0) {
    return err();
  }
  return {result};
}

Result<void> File::fsync() {
  auto result = ::fsync(fd_);
  if (result < 0) {
    return err();
  }
  return {};
}

Result<void> File::close() {
  auto result = ::close(fd_);
  // consider closed even on error.  Possible resource leak, but the alternative
  // is that the fd could be reused and further operations occur on an unrelated
  // file
  fd_ = -1;
  if (result < 0) {
    return err();
  }
  return {};
}
} // namespace toygit
