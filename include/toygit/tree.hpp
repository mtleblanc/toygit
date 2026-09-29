#pragma once

#include "toygit/object.hpp"
#include <filesystem>
#include <map>

namespace toygit {
class Tree : public Object {
public:
  enum class Mode { DIRECTORY, REGUALAR_FILE, EXECUTABLE_FILE, SYMLINK };
  std::string_view content() override;
  static std::shared_ptr<Tree> buildFrom(std::filesystem::path);

private:
  std::map<std::string, std::tuple<Id, Mode>> children_;
  std::string content_;
};
} // namespace toygit
