#include "toygit/add_command.hpp"
#include "toygit/dircache.hpp"

namespace toygit {
Result<void> AddCommand::run(
    std::span<std::string> args,
    [[maybe_unused]] const std::map<std::string, std::string> &env) {
  auto index = DirCache{repository};
  if (auto res = index.readFromFile(); !res) {
    return res;
  };
  for (auto file : args) {
    auto path = std::filesystem::path{file};
    if (auto res = index.add(path); !res) {
      return res;
    }
  }
  return index.writeToFile();
}
} // namespace toygit
