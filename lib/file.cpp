#include "toygit/file.hpp"

namespace toygit {

Result<void> File::writeAll(std::string_view sv) {
  while (!sv.empty()) {
    auto written = ::write(fd_, sv.data(), sv.size());
    if (written < 0) {
      auto ec = std::make_error_code(static_cast<std::errc>(errno));
      return std::unexpected{ec};
    }
    sv = sv.substr(written);
  }
  return {};
}

Result<void> File::fsync() {
  auto result = ::fsync(fd_);
  if (result < 0) {
    return std::unexpected{std::make_error_code(static_cast<std::errc>(errno))};
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
    return std::unexpected{std::make_error_code(static_cast<std::errc>(errno))};
  }
  return {};
}
} // namespace toygit
