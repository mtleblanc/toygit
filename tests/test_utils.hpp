#pragma once

#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <system_error>
#include <vector>

namespace toygit::test {

namespace fs = std::filesystem;
using namespace std::string_literals;

inline fs::path operator"" _p(const char *p, [[maybe_unused]] size_t len) {
  return fs::path{p};
}

using vs = std::vector<std::string>;

template <typename Map> auto keys(const Map &map) {
  return map | std::views::keys |
         std::ranges::to<std::vector<typename Map::key_type>>();
}

class TempDir {
public:
  TempDir() {
    auto tmpl = (fs::temp_directory_path() / "toygit-test-XXXXXX").string();
    if (::mkdtemp(tmpl.data()) == nullptr) {
      throw std::system_error{errno, std::generic_category(), "mkdtemp"};
    }
    path_ = tmpl;
  }

  ~TempDir() {
    std::error_code ec;
    fs::remove_all(path_, ec);
    // best-effort: never let a destructor throw during stack unwinding
  }

  TempDir(const TempDir &) = delete;
  TempDir &operator=(const TempDir &) = delete;
  TempDir(TempDir &&) = delete;
  TempDir &operator=(TempDir &&) = delete;

  const fs::path &path() const { return path_; }
  operator fs::path() const { return path_; }

  fs::path operator/(const fs::path &rhs) const { return path_ / rhs; }

private:
  fs::path path_;
};

inline fs::path createFile(const fs::path &p, std::string_view text) {
  fs::create_directories(p.parent_path());
  std::ofstream(p) << text;
  return p;
}

} // namespace toygit::test
