#pragma once

#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <system_error>

namespace toygit::test {

// RAII wrapper around a uniquely-named temporary directory. Created via
// mkdtemp (atomic — picks and creates the directory in one syscall, so no
// race between checking a name is free and creating it, the same guarantee
// Lockfile gets from O_CREAT|O_EXCL) and recursively removed on destruction.
class TempDir {
public:
  TempDir() {
    auto tmpl = (std::filesystem::temp_directory_path() / "toygit-test-XXXXXX")
                    .string();
    if (::mkdtemp(tmpl.data()) == nullptr) {
      throw std::system_error{errno, std::generic_category(), "mkdtemp"};
    }
    path_ = tmpl;
  }

  ~TempDir() {
    std::error_code ec;
    std::filesystem::remove_all(path_, ec);
    // best-effort: never let a destructor throw during stack unwinding
  }

  TempDir(const TempDir &) = delete;
  TempDir &operator=(const TempDir &) = delete;
  TempDir(TempDir &&) = delete;
  TempDir &operator=(TempDir &&) = delete;

  const std::filesystem::path &path() const { return path_; }
  operator std::filesystem::path() const { return path_; }

private:
  std::filesystem::path path_;
};

} // namespace toygit::test
