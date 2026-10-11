#pragma once

#include "toygit/object.hpp"
#include "toygit/repository.hpp"
#include "toygit/util.hpp"
#include <filesystem>
#include <map>

namespace toygit {
class Tree : public Object {
public:
  enum class Mode { DIRECTORY, REGUALAR_FILE, EXECUTABLE_FILE, SYMLINK };
  std::string_view content() override;
  static Result<std::shared_ptr<Tree>> buildFrom(std::filesystem::path);
  static Result<std::shared_ptr<Tree>>
  buildFromIndex(std::shared_ptr<Repository> repository);

private:
  std::map<std::string, std::tuple<Id, Mode>> children_;
  std::string content_;
};
} // namespace toygit
