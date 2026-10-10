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
  TRY(index.lock());
  TRY(index.readFromFile());
  for (auto file : args) {
    struct stat stat;
    auto ec = std::error_code{};
    auto fullPath = repository->cwd() / file;
    if (lstat(fullPath.c_str(), &stat) < 0) {
      ec = std::error_code(errno, std::system_category());
      if (ec == std::errc::no_such_file_or_directory) {
        std::println(std::cerr, "fatal: pathspec '{}' did not match any files",
                     file);
        return std::unexpected{GitError::FATAL};
      }
      return std::unexpected{ec};
    }
    auto path = std::filesystem::absolute(std::filesystem::path{fullPath}, ec)
                    .lexically_normal();
    if (ec) {
      return std::unexpected{ec};
    }
    TRY(index.add(path));
  }
  return index.writeToFile();
}
} // namespace toygit
