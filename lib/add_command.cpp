#include "toygit/add_command.hpp"
#include "toygit/dircache.hpp"
#include "toygit/error.hpp"
#include <iostream>
#include <system_error>

namespace toygit {
Result<void> AddCommand::run(
    std::span<std::string> args,
    [[maybe_unused]] const std::map<std::string, std::string> &env) {
  auto index = DirCache{repository};
  if (auto res = index.readFromFile(); !res) {
    return res;
  };
  for (auto file : args) {
    struct stat stat;
    auto ec = std::error_code{};
    if (lstat(file.c_str(), &stat) < 0) {
      ec = std::error_code(errno, std::system_category());
      if (ec == std::errc::no_such_file_or_directory) {
        std::println(std::cerr, "fatal: pathspec '{}' did not match any files",
                     file);
        return std::unexpected{GitError::FATAL};
      }
      return std::unexpected{ec};
    }
    auto path = std::filesystem::absolute(std::filesystem::path{file}, ec)
                    .lexically_normal();
    if (ec) {
      return std::unexpected{ec};
    }
    if (auto res = index.add(path); !res) {
      return res;
    }
  }
  return index.writeToFile();
}
} // namespace toygit
