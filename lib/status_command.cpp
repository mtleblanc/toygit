#include "toygit/status_command.hpp"
#include "toygit/dircache.hpp"
#include <filesystem>
#include <print>

namespace toygit {

namespace {
[[maybe_unused]] Result<void> traverse(const Repository &repo,
                                       const DirCache &index,
                                       const std::filesystem::path &path) {
  for (auto &de : std::filesystem::directory_iterator(path)) {
    if (repo.shouldIgnore(de)) {
      continue;
    }
    auto relative = de.path().lexically_relative(repo.root()).string();
    if (de.is_directory()) {
      auto first = index.entries().lower_bound(relative);
      if (first == index.entries().end() ||
          !(*first).first.starts_with(relative)) {
        std::println("?? {}", relative);
        continue;
      }
      if (auto res = traverse(repo, index, de.path()); !res) {
        return res;
      }
    } else {
      if (!index.entries().contains(relative)) {
        std::println("?? {}", relative);
      }
    }
  }
  return {};
}
} // namespace

Result<void> StatusCommand::run(
    [[maybe_unused]] std::span<std::string> args,
    [[maybe_unused]] const std::map<std::string, std::string> &env) {
  auto index = DirCache{repository};
  if (auto res = index.readFromFile(); !res) {
    return res;
  };

  return traverse(*repository, index, repository->root());
}
} // namespace toygit
